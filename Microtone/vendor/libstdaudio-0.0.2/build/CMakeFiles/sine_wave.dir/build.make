# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.23

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/build

# Include any dependencies generated for this target.
include CMakeFiles/sine_wave.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/sine_wave.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/sine_wave.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sine_wave.dir/flags.make

CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o: CMakeFiles/sine_wave.dir/flags.make
CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o: ../examples/sine_wave.cpp
CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o: CMakeFiles/sine_wave.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o -MF CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o.d -o CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o -c /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/examples/sine_wave.cpp

CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/examples/sine_wave.cpp > CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.i

CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/examples/sine_wave.cpp -o CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.s

# Object files for target sine_wave
sine_wave_OBJECTS = \
"CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o"

# External object files for target sine_wave
sine_wave_EXTERNAL_OBJECTS =

sine_wave: CMakeFiles/sine_wave.dir/examples/sine_wave.cpp.o
sine_wave: CMakeFiles/sine_wave.dir/build.make
sine_wave: CMakeFiles/sine_wave.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable sine_wave"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sine_wave.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sine_wave.dir/build: sine_wave
.PHONY : CMakeFiles/sine_wave.dir/build

CMakeFiles/sine_wave.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sine_wave.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sine_wave.dir/clean

CMakeFiles/sine_wave.dir/depend:
	cd /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2 /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2 /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/build /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/build /Users/danieltoby/Developer/microtone/Microtone/vendor/libstdaudio-0.0.2/build/CMakeFiles/sine_wave.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/sine_wave.dir/depend
