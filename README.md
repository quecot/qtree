# qtree - A Fast and Customizable Directory Tree

`qtree` is a command-line tool for traversing and displaying directory structures. It provides output in multiple formats including plain text, JSON, and XML. Designed for flexibility, it can be easily integrated into various workflows to visualize directory hierarchies.

## Features

- **Directory Traversal**: Recursively traverses directories and lists their contents.
- **Output Formats**: Supports output in plain text, JSON, and XML formats.
- **Customizable Output**: Allows specification of output files and format via command-line arguments.
- **Error Handling**: Provides informative error messages for common issues like file access errors.

## Installation

### Prerequisites

Ensure you have the following installed:

- CMake 3.10 or later
- A C compiler like `clang`

### Build Instructions

1. **Clone the Repository**:

   ```sh
   git clone https://github.com/quecot/qtree.git
   cd qtree
   ```

2. **Build the Project**:

   ```sh
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Install (Optional)**:
   ```sh
   sudo make install
   ```

## Usage

The `qtree` program is executed from the command line and provides various options for its operation. Below is a summary of the available options:

```sh
Usage: qtree [options] <directory>
Options:
-j, --json Output as JSON
-x, --xml Output as XML
-h, --help Show this help message
-v, --version Show version information
-o, --output Specify output file
```

Where:

- **`<directory>`**: The path to the directory to be traversed.
- **`-j, --json`**: Output the directory structure in JSON format.
- **`-x, --xml`**: Output the directory structure in XML format.
- **`-h, --help`**: Display the help message and exit.
- **`-v, --version`**: Display version information and exit.
- **`-o, --output <file>`**: Write the output to the specified file instead of the default `stdout`.

### Example

```sh
qtree -j -o output.json /path/to/directory
```

This command will traverse /path/to/directory and output the directory structure in JSON format to output.json.

## Development

### Building from Source

Follow the [Build Instructions](#build-instructions) to compile the project. The source code is organized with the main logic in `src/main.c`.

### CMake Configuration

The project uses CMake for build configuration. The `CMakeLists.txt` file is configured to use `clang` as the compiler and `clang-tidy` for code quality checks. The build type defaults to `Release`.

```cmake
cmake_minimum_required(VERSION 3.10)
project(qtree LANGUAGES C VERSION 0.1.0)

message(STATUS "CMake ${CMAKE_VERSION}")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_CLANG_TIDY "clang-tidy")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(qtree src/main.c)

target_compile_options(qtree PRIVATE
    -Werror
    -Wall
    -Wextra
    -Wpedantic
)
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE.md) file for more details.
