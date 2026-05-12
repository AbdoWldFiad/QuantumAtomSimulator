# **Atoms Quantum Orbital Visualizer**

the purpose of the project is to visualize the atomic behaver at the quantum level.


## **Building Requirements:**

1. C++ Compiler supporting C++ 17 or newer

2. [Cmake](https://cmake.org/)

3. [Vcpkg](https://vcpkg.io/en/)

4. [Git](https://git-scm.com/)

3. Install dependencies with Vcpkg
	- `vcpkg install glew glfw3 glm`
4. Get the vcpkg cmake toolchain file path
	- `vcpkg integrate install`
	- This will output something like : `CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=/Your/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"`

## **Build Instructions:**

1. Clone the repository:
	-  `git clone https://github.com/AbdoWldFiad/QuantumAtomSimulator.git`
2. CD into the newly cloned directory
	- `cd ./QuantumAtomSimulator` 
5. Create a build directory
	- `mkdir build`
6. Configure project with CMake
	-  `cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake`
	- Use the vcpkg cmake toolchain path from above
7. Build the project
	- `cmake --build build`
8. Run the program
	- The executables will be located in the build/Release 
	./build/Debug/Atoms.exe 

here is a bash script i use to automate the process
```bash
#!/usr/bin/env bash

set -e  # exit on error

rm -rf build

echo "==== Creating build directory...===="
mkdir -p build

echo "==== Running CMake configuration...===="
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE="path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"

echo "==== Building project...===="
cmake --build build

echo "==== Running executable...===="
./build/Debug/Atoms.exe 
```
run it after step 2


future goal:

1. [ ] Visualize different orbital path with different color

2. [ ] save the periodic table as the project database

3. [ ] simulate protons and neutrons or at least visualize the shape of it for simplicity

4. [ ] simulate the electrons behavior (or just make it as chaotic as possible)

5. [ ] update the project to handle multi atoms simulated together

6. [ ]  simulate the electrons behavior when bonding , either same or different atoms  