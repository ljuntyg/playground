# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

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
CMAKE_COMMAND = "C:/Program Files/CMake/bin/cmake.exe"

# The command to remove a file.
RM = "C:/Program Files/CMake/bin/cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:/Users/gbhko/Desktop/Prog/VSCode/playground

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:/Users/gbhko/Desktop/Prog/VSCode/playground/build

# Include any dependencies generated for this target.
include CMakeFiles/cpp-is-not-fun.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/cpp-is-not-fun.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/cpp-is-not-fun.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cpp-is-not-fun.dir/flags.make

CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj: CMakeFiles/cpp-is-not-fun.dir/flags.make
CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj: C:/Users/gbhko/Desktop/Prog/VSCode/playground/main.cpp
CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj: CMakeFiles/cpp-is-not-fun.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj -MF CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj.d -o CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj -c C:/Users/gbhko/Desktop/Prog/VSCode/playground/main.cpp

CMakeFiles/cpp-is-not-fun.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cpp-is-not-fun.dir/main.cpp.i"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/gbhko/Desktop/Prog/VSCode/playground/main.cpp > CMakeFiles/cpp-is-not-fun.dir/main.cpp.i

CMakeFiles/cpp-is-not-fun.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cpp-is-not-fun.dir/main.cpp.s"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/gbhko/Desktop/Prog/VSCode/playground/main.cpp -o CMakeFiles/cpp-is-not-fun.dir/main.cpp.s

# Object files for target cpp-is-not-fun
cpp__is__not__fun_OBJECTS = \
"CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj"

# External object files for target cpp-is-not-fun
cpp__is__not__fun_EXTERNAL_OBJECTS =

cpp-is-not-fun.exe: CMakeFiles/cpp-is-not-fun.dir/main.cpp.obj
cpp-is-not-fun.exe: CMakeFiles/cpp-is-not-fun.dir/build.make
cpp-is-not-fun.exe: CMakeFiles/cpp-is-not-fun.dir/linkLibs.rsp
cpp-is-not-fun.exe: CMakeFiles/cpp-is-not-fun.dir/objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable cpp-is-not-fun.exe"
	"C:/Program Files/CMake/bin/cmake.exe" -E rm -f CMakeFiles/cpp-is-not-fun.dir/objects.a
	C:/msys64/mingw64/bin/ar.exe qc CMakeFiles/cpp-is-not-fun.dir/objects.a @CMakeFiles/cpp-is-not-fun.dir/objects1.rsp
	C:/msys64/mingw64/bin/g++.exe -O3 -DNDEBUG -Wl,--whole-archive CMakeFiles/cpp-is-not-fun.dir/objects.a -Wl,--no-whole-archive -o cpp-is-not-fun.exe -Wl,--out-implib,libcpp-is-not-fun.dll.a -Wl,--major-image-version,0,--minor-image-version,0 @CMakeFiles/cpp-is-not-fun.dir/linkLibs.rsp
	C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe -noprofile -executionpolicy Bypass -file C:/dev/vcpkg/scripts/buildsystems/msbuild/applocal.ps1 -targetBinary C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/cpp-is-not-fun.exe -installedDir C:/dev/vcpkg/installed/x64-windows/bin -OutVariable out

# Rule to build all files generated by this target.
CMakeFiles/cpp-is-not-fun.dir/build: cpp-is-not-fun.exe
.PHONY : CMakeFiles/cpp-is-not-fun.dir/build

CMakeFiles/cpp-is-not-fun.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cpp-is-not-fun.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cpp-is-not-fun.dir/clean

CMakeFiles/cpp-is-not-fun.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" C:/Users/gbhko/Desktop/Prog/VSCode/playground C:/Users/gbhko/Desktop/Prog/VSCode/playground C:/Users/gbhko/Desktop/Prog/VSCode/playground/build C:/Users/gbhko/Desktop/Prog/VSCode/playground/build C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles/cpp-is-not-fun.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cpp-is-not-fun.dir/depend

