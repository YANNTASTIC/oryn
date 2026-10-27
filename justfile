default:
    @just --list

configure:
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    ln -sf build/compile_commands.json compile_commands.json

build: configure
    cmake --build build

test: build
    ctest --test-dir build --output-on-failure

watch:
    watchexec -e cpp,hpp,h,cc,txt -- just test

fmt:
    clang-format -i $(git ls-files '*.cpp' '*.hpp' '*.h' '*.cc')

tidy:
    clang-tidy -p build $(git ls-files '*.cpp')

clean:
    rm -rf build

