#include <microtone/exception.hpp>
#include <microtone/log.hpp>
#include <microtone/midi_input.hpp>
#include <microtone/synthesizer/envelope.hpp>
#include <microtone/synthesizer/filter.hpp>
#include <microtone/synthesizer/low_frequency_oscillator.hpp>
#include <microtone/synthesizer/oscillator.hpp>
#include <microtone/synthesizer/synthesizer.hpp>
#include <microtone/synthesizer/synthesizer_voice.hpp>

#include <portaudio/portaudio.h>
#include <rtmidi/RtMidi.h>

#include <cmath>
#include <mutex>
#include <unordered_set>
#include <vector>

namespace microtone {

class Synthesizer::impl {
public:
    impl(const std::vector<WeightedWaveTable>& weightedWaveTables, OnOutputFn fn) :
        _onOutputFn{fn},
        _weightedWaveTables{weightedWaveTables},
        _activeVoices{},
        _sustainedVoices{},
        _voices{},
        _sustainPedalOn{false},
        _sampleRate{0} {
        // Initialize portaudio
        auto portAudioInitResult = Pa_Initialize();
        if (portAudioInitResult != paNoError) {
            throw MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 portAudioInitResult,
                                                 Pa_GetErrorText(portAudioInitResult)));
        }

        auto deviceId = Pa_GetDefaultOutputDevice();
        auto outputParameters = PaStreamParameters{};
        outputParameters.device = deviceId;
        if (outputParameters.device == paNoDevice) {
            throw MicrotoneException(fmt::format("Unable to open output device {}.", deviceId));
        }

        const auto deviceInfo = Pa_GetDeviceInfo(deviceId);
        if (deviceInfo) {
            M_INFO("Opened output device '{}'.", deviceInfo->name);
        } else {
            throw MicrotoneException(fmt::format("Unable to collect info on output device {}.", deviceId));
        }

        _sampleRate = deviceInfo->defaultSampleRate;

        outputParameters.channelCount = 2;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        PaError openStreamResult = Pa_OpenStream(
            &_portAudioStream,
            nullptr,
            &outputParameters,
            _sampleRate,
            FRAMES_PER_BUFFER,
            paClipOff,
            &impl::paCallback,
            this);

        if (openStreamResult != paNoError) {
            throw MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 openStreamResult,
                                                 Pa_GetErrorText(openStreamResult)));
        }

        for (auto i = 0; i < 127; ++i) {
            _voices.emplace_back(noteToFrequencyHertz(i),
                                 Envelope{0.01, 0.1, .8, 0.01, _sampleRate},
                                 Oscillator{noteToFrequencyHertz(i), _sampleRate},
                                 LowFrequencyOscillator{0.25, _sampleRate},
                                 Filter{});
        }
    }

    ~impl() {
        if (_portAudioStream && _portAudioStream) {
            Pa_StopStream(_portAudioStream);
        }
        Pa_Terminate();
    }

    void start() {
        auto startStreamResult = Pa_StartStream(_portAudioStream);
        if (startStreamResult != paNoError) {
            throw MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 startStreamResult,
                                                 Pa_GetErrorText(startStreamResult)));
        }
        M_INFO("Started synthesizer.");
    }

    void stop() {
        auto stopStreamResult = Pa_StopStream(_portAudioStream);
        if (stopStreamResult != paNoError) {
            throw MicrotoneException(fmt::format("PortAudio error: {}, '{}'.",
                                                 stopStreamResult,
                                                 Pa_GetErrorText(stopStreamResult)));
        }
        M_INFO("Stopped synthesizer.");
    }

    /* This routine will be called by the PortAudio engine when audio is needed.
       It may called at interrupt level on some machines so don't do anything
       that could mess up the system like calling malloc() or free().
    */
    static int paCallback([[maybe_unused]] const void* inputBuffer,
                          void* outputBuffer,
                          unsigned long framesPerBuffer,
                          [[maybe_unused]] const PaStreamCallbackTimeInfo* timeInfo,
                          [[maybe_unused]] PaStreamCallbackFlags statusFlags,
                          void* userData) {
        auto data = static_cast<impl*>(userData);
        auto out = static_cast<float*>(outputBuffer);

        for (auto frame = 0; frame < static_cast<int>(framesPerBuffer); ++frame) {
            auto sample = data->nextSample();
            data->_lastOutputBuffer[frame] = sample;

            for (std::size_t channel = 0; channel < 2; ++channel) {
                *out++ = sample;
            }
        }

        data->_onOutputFn(data->_lastOutputBuffer);

        return paContinue;
    }

    float nextSample() {
        auto nextSample = 0.0;
        if (_mutex.try_lock()) {
            if (_voices.empty()) {
                return 0;
            }
            for (auto& id : _activeVoices) {
                nextSample += _voices[id].nextSample(_weightedWaveTables);
            }

            _mutex.unlock();
        }
        return nextSample;
    }

    std::vector<WeightedWaveTable> weightedWaveTables() const {
        return _weightedWaveTables;
    }

    void setWaveTables(const std::vector<WeightedWaveTable>& weightedWaveTables) {
        auto lockGaurd = std::unique_lock<std::mutex>{_mutex};
        _weightedWaveTables = weightedWaveTables;
    }

    void setEnvelope(const Envelope& envelope) {
        auto lockGaurd = std::unique_lock<std::mutex>{_mutex};
        for (auto& voice : _voices) {
            voice.setEnvelope(envelope);
        }
    }

    void setFilter(const Filter& filter) {
        auto lockGaurd = std::unique_lock<std::mutex>{_mutex};
        for (auto& voice : _voices) {
            voice.setFilter(filter);
        }
    }

    double noteToFrequencyHertz(int note) {
        constexpr auto pitch = 440.0f;
        return pitch * std::pow(2.0f, static_cast<float>(note - 69) / 12.0);
    }

    void addMidiData(int status, int note, int velocity) {
	static FILE *log = 0;
	if (!log) {
		log = fopen("out.log", "wt");
	}
        fprintf(log, "%s: note=%d\n", __func__, note);fflush(log);
        auto lockGaurd = std::unique_lock<std::mutex>{_mutex};
        auto midiStatus = MidiStatusMessage(status);

        if (midiStatus == MidiStatusMessage::NoteOn) {
            fprintf(log, "%s:  ON\n", __func__);fflush(log);
            _voices[note].setVelocity(velocity);
            _voices[note].triggerOn();
            _activeVoices.insert(note);
        } else if (midiStatus == MidiStatusMessage::NoteOff) {
            fprintf(log, "%s:  OFF\n", __func__);fflush(log);
            if (_sustainPedalOn) {
                fprintf(log, "%s:  SUST\n", __func__);fflush(log);
                _sustainedVoices.insert(note);
            } else {
                fprintf(log, "%s:  NONSUST\n", __func__);fflush(log);
                _voices[note].triggerOff();
            }
        } else if (midiStatus == MidiStatusMessage::ControlChange) {
            fprintf(log, "%s:  CC\n", __func__);fflush(log);
            if (note == 64) {
                _sustainPedalOn = velocity > 64;
                if (!_sustainPedalOn) {
                    for (const auto& id : _sustainedVoices) {
                        _voices[id].triggerOff();
                    }
                    _sustainedVoices.clear();
                }
            }
        }

        fprintf(log, "%s: voices:%d active:%d\n", __func__, (int)_voices.size(), (int)_activeVoices.size());fflush(log);
        std::unordered_set<int> toerase;
        for (const auto& id : _activeVoices) {
	    fprintf(log, "%s:  id=%d\n", __func__, (int)id);fflush(log);
            if (!_voices[id].isActive()) {
		fprintf(log, "%s:   ERASE\n", __func__, (int)id);fflush(log);
		toerase.insert(id);
//                _activeVoices.erase(id);
            } else {
		fprintf(log, "%s:   NONERASE\n", __func__, (int)id);fflush(log);
            }
        }
        for (const auto& id : toerase) {
	    fprintf(log, "%s:  ERASING %d\n", __func__, (int)id);fflush(log);
    	    _activeVoices.erase(id);
        }
    }

    double sampleRate() {
        return _sampleRate;
    }

    OnOutputFn _onOutputFn;
    PaStream* _portAudioStream;    // Owned by port audio, cleaned up by Pa_Terminate().
    std::mutex _mutex;
    std::vector<WeightedWaveTable> _weightedWaveTables;
    AudioBuffer _lastOutputBuffer;  // Forwarded to onOutputFn()
    std::unordered_set<int> _activeVoices;
    std::unordered_set<int> _sustainedVoices;
    std::vector<SynthesizerVoice> _voices;
    bool _sustainPedalOn;
    double _sampleRate;
};

Synthesizer::Synthesizer(const std::vector<WeightedWaveTable>& weightedWaveTables, OnOutputFn fn) :
    _impl{new impl{weightedWaveTables, fn}} {
}

Synthesizer::Synthesizer(Synthesizer&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Synthesizer& Synthesizer::operator=(Synthesizer&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

Synthesizer::~Synthesizer() = default;

void Synthesizer::start() {
    _impl->start();
}

void Synthesizer::stop() {
    _impl->stop();
}

std::vector<WeightedWaveTable> Synthesizer::weightedWaveTables() const {
    return _impl->weightedWaveTables();
}

void Synthesizer::setWaveTables(const std::vector<WeightedWaveTable>& weightedWaveTables) {
    _impl->setWaveTables(weightedWaveTables);
}

void Synthesizer::setEnvelope(const Envelope& envelope) {
    _impl->setEnvelope(envelope);
}

void Synthesizer::setFilter(const Filter& filter) {
    _impl->setFilter(filter);
}

void Synthesizer::addMidiData(int status, int note, int velocity) {
    _impl->addMidiData(status, note, velocity);
}

double Synthesizer::sampleRate() {
    return _impl->sampleRate();
}

}
