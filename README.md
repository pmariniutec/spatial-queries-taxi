# CMake C++ Project with Catch2 testing

## Directory Layout
    .
    ├── lib                     # Dependencies
    ├── src                     # Source files
    ├── tests                   # Automated tests
    ├── cmake                   # cmake scripts
    ├── CMakeLists.txt          # main build config
    ├── LICENSE
    └── README.md


## Requirements:
- cmake
- gcc / clang / msvc


## Usage
```bash
mkdir build
cd build

# Generate build files.
cmake ..

# Compile
make

# Run program (EXEC_NAME is set on CMakeLists.txt)
./[EXEC_NAME] 

# Run tests
make tests
```


## Remarks
- This project uses the [v3](https://github.com/catchorg/Catch2/blob/devel/docs/migrate-v2-to-v3.md#top) branch of Catch2.
- Catch2 is built as a static library and then linked to our tests executable.
- When generating the build files, all submodules will be fetched using the script `SubmodulesCheck.cmake`. This can be turned off by changing the variable `GIT_SUBMODULE`.
