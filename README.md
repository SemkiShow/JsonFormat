# JsonFormat

A json library with individual json object formatting support

## How to use

1. Add the library as a submodule
```bash
mkdir -p thirdparty
cd thirdparty
git submodule add --depth 1 https://github.com/SemkiShow/JsonFormat
cd ..
```
2. Add JsonFormat to your CMakeLists.txt
```cmake
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/JsonFormat ${CMAKE_BINARY_DIR}/_deps/JsonFormat-build SYSTEM)
```

## CMake options

| Option name | Purpose |
| --- | --- |
| JSON_FORMAT_STRICT_ERRORS | Get*() functions throw an error instead of returning 0 on a null object  |

## Examples

1. Run 
```bash
git clone https://github.com/SemkiShow/JsonFormat
cd JsonFormat/examples
chmod +x run.sh
./run.sh --help # See what examples are available
./run.sh <example-name>
```

## Documentation

Documentation is available [here](https://SemkiShow.github.io/JsonFormat/)
