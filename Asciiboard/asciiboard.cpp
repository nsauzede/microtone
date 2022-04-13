#include <asciiboard.hpp>

#include "ftxui/component/component.hpp"         // for Checkbox, Renderer, Horizontal, Vertical, Input, Menu, Radiobox, ResizableSplitLeft, Tab
#include "ftxui/component/component_base.hpp"    // for ComponentBase
#include "ftxui/component/component_options.hpp" // for MenuOption, InputOption
#include "ftxui/component/event.hpp"             // for Event, Event::Custom
#include "ftxui/component/screen_interactive.hpp"// for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"                // for text, color, operator|, bgcolor, filler, Element, vbox, size, hbox, separator, flex, window, graph, EQUAL, paragraph, WIDTH, hcenter, Elements, bold, vscroll_indicator, HEIGHT, flexbox, hflow, border, frame, flex_grow, gauge, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight, dim, spinner, LESS_THAN, center, yframe, GREATER_THAN
#include "ftxui/dom/flexbox_config.hpp"          // for FlexboxConfig
#include "ftxui/screen/color.hpp"                // for Color, Color::BlueLight, Color::RedLight, Color::Black, Color::Blue, Color::Cyan, Color::CyanLight, Color::GrayDark, Color::GrayLight, Color::Green, Color::GreenLight, Color::Magenta, Color::MagentaLight, Color::Red, Color::White, Color::Yellow, Color::YellowLight, Color::Default, Color::Palette256, ftxui
#include "ftxui/screen/color_info.hpp"           // for ColorInfo
#include "ftxui/screen/terminal.hpp"             // for Size, Dimensions

#include <fmt/format.h>

#include <microtone/midi_input.hpp>

#include <unordered_set>
#include <string>

namespace asciiboard {

using namespace ftxui;

class Asciiboard::impl {
public:
    impl() :
        _screen{ScreenInteractive::Fullscreen()},
        _lastOutputBuffer{},
        _activeMidiNotes{},
        _sustainedMidiNotes{},
        _sustainPedalOn{false} {}

    // No allocation can take place here (called every frame)
    void addOutputData(const microtone::AudioBuffer& data) {
        std::copy(std::begin(data), std::end(data), std::begin(_lastOutputBuffer));
        _screen.PostEvent(Event::Custom);
    }

    void addMidiData(int status, int note, [[maybe_unused]] int velocity) {
        // auto midiStatus = static_cast<microtone::MidiStatusMessage>(status);

        if (status == 0b10010000) {
            _activeMidiNotes.insert(note);
        } else if (status == 0b10000000) {
            if (_sustainPedalOn) {
                _sustainedMidiNotes.insert(note);
            } else {
                _activeMidiNotes.erase(note);
            }
        } else if (status == 0b10110000) {
            // Control Change
            if (note == 64) {
                _sustainPedalOn = velocity > 64;
                if (!_sustainPedalOn) {
                    for (const auto& id : _sustainedMidiNotes) {
                        _activeMidiNotes.erase(id);
                    }
                    _sustainedMidiNotes.clear();
                }
            }
        }
        _screen.PostEvent(Event::Custom);
    }


    void loop(const SynthControls& initialControls, const OnControlsChangedFn& onControlsChangedFn) {
        auto controls = SynthControls{initialControls};

        // Instructions
        auto showInstructions = true;
        auto closeInstructions = Button("Ok", [&showInstructions] { showInstructions = false; });
        auto instructions = Renderer(closeInstructions, [&closeInstructions] {
            return vbox({
                       text("Press <enter> to submit changes to the controls. Press 'q' to quit."),
                         hbox({
                           filler(),
                           closeInstructions->Render(),
                           filler()
                         })
                    }) | hcenter | border;
        });

        // Oscillator Controls
        auto sine = static_cast<int>(controls.sineWeight * 100);
        auto square = static_cast<int>(controls.squareWeight * 100);
        auto triangle = static_cast<int>(controls.triangleWeight * 100);
        auto sineSlider = Slider("Sine:", &sine, 0, 100, 1);
        auto squareSlider = Slider("Square:", &square, 0, 100, 1);
        auto triangleSlider = Slider("Triangle:", &triangle, 0, 100, 1);

        auto oscillatorControlsContainer = Container::Horizontal({
                                              sineSlider,
                                              squareSlider,
                                              triangleSlider,
                                              });

        auto oscillatorControls = Renderer(oscillatorControlsContainer,
                                           [&sineSlider, &squareSlider, &triangleSlider] () {
            return hbox({
                sineSlider->Render(),
                filler(),
                squareSlider->Render(),
                filler(),
                triangleSlider->Render()
            });
        });

        // Envelope Controls
        auto attack = static_cast<int>(controls.attack * 100);
        auto decay = static_cast<int>(controls.decay * 100);
        auto sustain = static_cast<int>(controls.sustain * 100);
        auto release = static_cast<int>(controls.release * 100);
        auto attackSlider = Slider("Attack:", &attack, 0, 100, 1);
        auto decaySlider = Slider("Decay:", &decay, 0, 100, 1);
        auto sustainSlider = Slider("Sustain:", &sustain, 0, 100, 1);
        auto releaseSlider = Slider("Release:", &release, 0, 100, 1);

        auto envelopeControlsContainer = Container::Horizontal({
            attackSlider,
            decaySlider,
            sustainSlider,
            releaseSlider
            });

        auto envelopeControls = Renderer(envelopeControlsContainer,
                                           [&attackSlider, &decaySlider, &sustainSlider, &releaseSlider] () {
                                               return hbox({
                                                   attackSlider->Render(),
                                                   filler(),
                                                   decaySlider->Render(),
                                                   filler(),
                                                   sustainSlider->Render(),
                                                 filler(),
                                                     releaseSlider->Render()
                                               });
                                           });

        // Oscilloscope
        auto scaleFactor = 0.5;
        auto oscilloscopeHeight = 60;
        auto oscilloscope = Renderer([&] {
            auto width = 200;
            auto c = Canvas(width, oscilloscopeHeight);
            for (auto i = 0; i < width - 1; ++i) {
                if (i < static_cast<int>(_lastOutputBuffer.size())) {
                    c.DrawPointLine(i,
                                    (_lastOutputBuffer[i] * oscilloscopeHeight * scaleFactor) + (oscilloscopeHeight / 2),
                                    i + 1,
                                    (_lastOutputBuffer[i + 1] * oscilloscopeHeight * scaleFactor) + (oscilloscopeHeight / 2));
                } else {
                    c.DrawPointLine(i, oscilloscopeHeight / 2, i + 1, oscilloscopeHeight / 2);
                }
            }

            return vbox({
                text(fmt::format("Frequency [Mhz] x ({})", scaleFactor)) | hcenter,
                canvas(std::move(c))
            });
        });

        // Piano roll
        auto keyboard = [](int width, int height) {
            std::vector<int> output(width, 0);
            auto blackKeys = std::unordered_set<int>{1, 3, 6, 8, 10};
            for (auto i = 0; i < width; ++i) {
                if (blackKeys.find((i) % 12) == blackKeys.end()) {
                    // White key
                    output[i] = height - 1;
                }
            }
            return output;
        };

        auto activeNotes = [this](int width, [[maybe_unused]] int height) {
            std::vector<int> output(width, -1);
            for (auto i = 0; i < width; ++i) {
                if (_activeMidiNotes.find(i) != _activeMidiNotes.end() ||
                        _sustainedMidiNotes.find(i) != _sustainedMidiNotes.end()) {
                    output[i] = 1;
                }
            }
            return output;
        };

        auto pianoRoll = Renderer([&activeNotes, &keyboard](){
            return vbox({
                hbox({
                    filler(),
                    graph(std::ref(activeNotes))
                        | size(WIDTH, EQUAL, 66)
                        | size(HEIGHT, EQUAL, 2)
                        | color(Color::GreenLight),
                    filler()
                }),
                hbox({
                    filler(),
                    graph(std::ref(keyboard))
                        | size(WIDTH, EQUAL, 66)
                        | size(HEIGHT, EQUAL, 2),
                    filler()
                })
            });
        });

        auto synthesizerContainer = Container::Vertical({
                                                  oscillatorControls,
                                                  envelopeControls,
                                                  oscilloscope,
                                                  pianoRoll});

        auto synthesizer = Renderer(synthesizerContainer, [&] {
            return vbox({
                oscillatorControls->Render() | borderRounded,
                envelopeControls->Render() | borderRounded,
                vbox({
                    oscilloscope->Render()
                        | size(HEIGHT, LESS_THAN, oscilloscopeHeight)
                        | hcenter
                        | borderRounded,
                    pianoRoll->Render() | borderRounded
                 })
            });
        });

        auto mainContents = Container::Vertical({
            instructions,
            synthesizer
        });

        auto mainRenderer = Renderer(mainContents, [&showInstructions, &synthesizer, &instructions] {
            Element document = synthesizer->Render();

            if (showInstructions) {
            document = dbox({
                             document,
                    instructions->Render() | clear_under | center,
                             });
            }
            return document;
        });

        auto eventListener = CatchEvent(mainRenderer, [&] (Event event) {
            if (event == Event::Character('q')) {
                _screen.ExitLoopClosure()();
                return true;
            } else if (event == Event::Return) {
                controls.sineWeight = static_cast<double>(sine) / 100;
                controls.squareWeight = static_cast<double>(square) / 100;
                controls.triangleWeight = static_cast<double>(triangle) / 100;

                controls.attack = static_cast<double>(attack) / 100;
                controls.decay = static_cast<double>(decay) / 100;
                controls.sustain = static_cast<double>(sustain) / 100;
                controls.release = static_cast<double>(release) / 100;

                onControlsChangedFn(controls);
            }
            return false;
        });

        _screen.Loop(eventListener);
    }

    ftxui::ScreenInteractive _screen;
    microtone::AudioBuffer _lastOutputBuffer;
    std::unordered_set<int> _activeMidiNotes;
    std::unordered_set<int> _sustainedMidiNotes;
    bool _sustainPedalOn;
};

Asciiboard::Asciiboard() :
    _impl{new impl{}} {
}

Asciiboard::Asciiboard(Asciiboard&& other) noexcept :
    _impl{std::move(other._impl)} {
}

Asciiboard& Asciiboard::operator=(Asciiboard&& other) noexcept {
    if (this != &other) {
        _impl = std::move(other._impl);
    }
    return *this;
}

void Asciiboard::addOutputData(const microtone::AudioBuffer& data) {
    _impl->addOutputData(data);
}

void Asciiboard::addMidiData(int status, int note, int velocity) {
    _impl->addMidiData(status, note, velocity);
}

void Asciiboard::loop(const SynthControls& initialControls, const OnControlsChangedFn& onControlsChangedFn) {
    _impl->loop(initialControls, onControlsChangedFn);
}

Asciiboard::~Asciiboard() = default;

}
