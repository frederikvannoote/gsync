Conan integration for libsodium

This project uses Conan (v2) to obtain native libsodium for encryption support.

Prerequisites
- Python + pip
- Conan 2.x (install with `pip install --upgrade conan`)

Quick start (Windows / MSVC example)

1. Create a build directory:
   mkdir build && cd build

2. Install Conan dependencies (writes toolchain and CMakeDeps files into build/):
   conan install .. --output-folder=. --profile:host=default -s build_type=Debug --build=missing

3. Configure CMake using the generated Conan toolchain:
   cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug

4. Build:
   cmake --build . --config Debug

Notes
- To change libsodium version, update `conanfile.txt` at repository root and re-run `conan install`.
- You can use Conan lockfiles for reproducible builds (`conan lock create ...`).
- Recommended environment variables: `CONAN_REVISIONS_ENABLED=1` for revision-aware installs.

CI
See `.github/workflows/ci.yml` for an example GitHub Actions workflow that installs Conan and builds on Windows.
