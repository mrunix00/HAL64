# HAL64

HAL64 is a simple stack-based virtual machine with a custom bytecode format,
written in C89. It is designed to be easy to understand and modify, and to be
embeddable in other programs and to make it easy to implement programming
languages on top of it.

## Building

1. Clone the repository:
```console
git clone --recursive https://github.com/yourusername/HAL64.git
cd HAL64
```

2. Create a build directory and navigate into it:
```console
mkdir build
cd build
```

3. Build the project:
```console
cmake ..
cmake --build .
```
