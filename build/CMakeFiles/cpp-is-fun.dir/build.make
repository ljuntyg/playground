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
include CMakeFiles/cpp-is-fun.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/cpp-is-fun.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/cpp-is-fun.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/cpp-is-fun.dir/flags.make

CMakeFiles/cpp-is-fun.dir/main.cpp.obj: CMakeFiles/cpp-is-fun.dir/flags.make
CMakeFiles/cpp-is-fun.dir/main.cpp.obj: CMakeFiles/cpp-is-fun.dir/includes_CXX.rsp
CMakeFiles/cpp-is-fun.dir/main.cpp.obj: C:/Users/gbhko/Desktop/Prog/VSCode/playground/main.cpp
CMakeFiles/cpp-is-fun.dir/main.cpp.obj: CMakeFiles/cpp-is-fun.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/cpp-is-fun.dir/main.cpp.obj"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cpp-is-fun.dir/main.cpp.obj -MF CMakeFiles/cpp-is-fun.dir/main.cpp.obj.d -o CMakeFiles/cpp-is-fun.dir/main.cpp.obj -c C:/Users/gbhko/Desktop/Prog/VSCode/playground/main.cpp

CMakeFiles/cpp-is-fun.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cpp-is-fun.dir/main.cpp.i"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/gbhko/Desktop/Prog/VSCode/playground/main.cpp > CMakeFiles/cpp-is-fun.dir/main.cpp.i

CMakeFiles/cpp-is-fun.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cpp-is-fun.dir/main.cpp.s"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/gbhko/Desktop/Prog/VSCode/playground/main.cpp -o CMakeFiles/cpp-is-fun.dir/main.cpp.s

CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj: CMakeFiles/cpp-is-fun.dir/flags.make
CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj: CMakeFiles/cpp-is-fun.dir/includes_CXX.rsp
CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj: C:/Users/gbhko/Desktop/Prog/VSCode/playground/matrix.cpp
CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj: CMakeFiles/cpp-is-fun.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj -MF CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj.d -o CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj -c C:/Users/gbhko/Desktop/Prog/VSCode/playground/matrix.cpp

CMakeFiles/cpp-is-fun.dir/matrix.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cpp-is-fun.dir/matrix.cpp.i"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/gbhko/Desktop/Prog/VSCode/playground/matrix.cpp > CMakeFiles/cpp-is-fun.dir/matrix.cpp.i

CMakeFiles/cpp-is-fun.dir/matrix.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cpp-is-fun.dir/matrix.cpp.s"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/gbhko/Desktop/Prog/VSCode/playground/matrix.cpp -o CMakeFiles/cpp-is-fun.dir/matrix.cpp.s

CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj: CMakeFiles/cpp-is-fun.dir/flags.make
CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj: CMakeFiles/cpp-is-fun.dir/includes_CXX.rsp
CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj: C:/Users/gbhko/Desktop/Prog/VSCode/playground/renderer.cpp
CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj: CMakeFiles/cpp-is-fun.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj -MF CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj.d -o CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj -c C:/Users/gbhko/Desktop/Prog/VSCode/playground/renderer.cpp

CMakeFiles/cpp-is-fun.dir/renderer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cpp-is-fun.dir/renderer.cpp.i"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/gbhko/Desktop/Prog/VSCode/playground/renderer.cpp > CMakeFiles/cpp-is-fun.dir/renderer.cpp.i

CMakeFiles/cpp-is-fun.dir/renderer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cpp-is-fun.dir/renderer.cpp.s"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/gbhko/Desktop/Prog/VSCode/playground/renderer.cpp -o CMakeFiles/cpp-is-fun.dir/renderer.cpp.s

CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj: CMakeFiles/cpp-is-fun.dir/flags.make
CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj: CMakeFiles/cpp-is-fun.dir/includes_CXX.rsp
CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj: C:/Users/gbhko/Desktop/Prog/VSCode/playground/vertex.cpp
CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj: CMakeFiles/cpp-is-fun.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj -MF CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj.d -o CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj -c C:/Users/gbhko/Desktop/Prog/VSCode/playground/vertex.cpp

CMakeFiles/cpp-is-fun.dir/vertex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cpp-is-fun.dir/vertex.cpp.i"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/gbhko/Desktop/Prog/VSCode/playground/vertex.cpp > CMakeFiles/cpp-is-fun.dir/vertex.cpp.i

CMakeFiles/cpp-is-fun.dir/vertex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cpp-is-fun.dir/vertex.cpp.s"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/gbhko/Desktop/Prog/VSCode/playground/vertex.cpp -o CMakeFiles/cpp-is-fun.dir/vertex.cpp.s

CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj: CMakeFiles/cpp-is-fun.dir/flags.make
CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj: CMakeFiles/cpp-is-fun.dir/includes_CXX.rsp
CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj: C:/Users/gbhko/Desktop/Prog/VSCode/playground/triangle.cpp
CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj: CMakeFiles/cpp-is-fun.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj -MF CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj.d -o CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj -c C:/Users/gbhko/Desktop/Prog/VSCode/playground/triangle.cpp

CMakeFiles/cpp-is-fun.dir/triangle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/cpp-is-fun.dir/triangle.cpp.i"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:/Users/gbhko/Desktop/Prog/VSCode/playground/triangle.cpp > CMakeFiles/cpp-is-fun.dir/triangle.cpp.i

CMakeFiles/cpp-is-fun.dir/triangle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/cpp-is-fun.dir/triangle.cpp.s"
	C:/msys64/mingw64/bin/g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:/Users/gbhko/Desktop/Prog/VSCode/playground/triangle.cpp -o CMakeFiles/cpp-is-fun.dir/triangle.cpp.s

# Object files for target cpp-is-fun
cpp__is__fun_OBJECTS = \
"CMakeFiles/cpp-is-fun.dir/main.cpp.obj" \
"CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj" \
"CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj" \
"CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj" \
"CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj"

# External object files for target cpp-is-fun
cpp__is__fun_EXTERNAL_OBJECTS =

cpp-is-fun.exe: CMakeFiles/cpp-is-fun.dir/main.cpp.obj
cpp-is-fun.exe: CMakeFiles/cpp-is-fun.dir/matrix.cpp.obj
cpp-is-fun.exe: CMakeFiles/cpp-is-fun.dir/renderer.cpp.obj
cpp-is-fun.exe: CMakeFiles/cpp-is-fun.dir/vertex.cpp.obj
cpp-is-fun.exe: CMakeFiles/cpp-is-fun.dir/triangle.cpp.obj
cpp-is-fun.exe: CMakeFiles/cpp-is-fun.dir/build.make
cpp-is-fun.exe: C:/dev/vcpkg/installed/x64-windows/lib/manual-link/SDL2main.lib
cpp-is-fun.exe: C:/dev/vcpkg/installed/x64-windows/lib/SDL2.lib
cpp-is-fun.exe: CMakeFiles/cpp-is-fun.dir/linkLibs.rsp
cpp-is-fun.exe: CMakeFiles/cpp-is-fun.dir/objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable cpp-is-fun.exe"
	"C:/Program Files/CMake/bin/cmake.exe" -E rm -f CMakeFiles/cpp-is-fun.dir/objects.a
	C:/msys64/mingw64/bin/ar.exe qc CMakeFiles/cpp-is-fun.dir/objects.a @CMakeFiles/cpp-is-fun.dir/objects1.rsp
	C:/msys64/mingw64/bin/g++.exe -O3 -DNDEBUG -Wl,--whole-archive CMakeFiles/cpp-is-fun.dir/objects.a -Wl,--no-whole-archive -o cpp-is-fun.exe -Wl,--out-implib,libcpp-is-fun.dll.a -Wl,--major-image-version,0,--minor-image-version,0 @CMakeFiles/cpp-is-fun.dir/linkLibs.rsp
	C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe -noprofile -executionpolicy Bypass -file C:/dev/vcpkg/scripts/buildsystems/msbuild/applocal.ps1 -targetBinary C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/cpp-is-fun.exe -installedDir C:/dev/vcpkg/installed/x64-windows/bin -OutVariable out

# Rule to build all files generated by this target.
CMakeFiles/cpp-is-fun.dir/build: cpp-is-fun.exe
.PHONY : CMakeFiles/cpp-is-fun.dir/build

CMakeFiles/cpp-is-fun.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/cpp-is-fun.dir/cmake_clean.cmake
.PHONY : CMakeFiles/cpp-is-fun.dir/clean

CMakeFiles/cpp-is-fun.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" C:/Users/gbhko/Desktop/Prog/VSCode/playground C:/Users/gbhko/Desktop/Prog/VSCode/playground C:/Users/gbhko/Desktop/Prog/VSCode/playground/build C:/Users/gbhko/Desktop/Prog/VSCode/playground/build C:/Users/gbhko/Desktop/Prog/VSCode/playground/build/CMakeFiles/cpp-is-fun.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/cpp-is-fun.dir/depend

