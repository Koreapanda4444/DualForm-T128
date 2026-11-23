# DualForm-T128

A C11 research implementation of the specified 24-round tweakable block cipher: 128-bit blocks, 256-bit keys and 128-bit public tweaks. Bit sizes are not a security rating. Authentication and encryption modes are outside this project.

[한국어](README.ko.md) · [Specification](docs/spec_ko.md) · [API and installation](docs/api_ko.md) · [Security findings](docs/security_ko.md) · [Measured performance](docs/benchmark_ko.md) · [Validation](docs/validation_ko.md) · [Commit progress](docs/progress_ko.md)

The baseline includes an opaque base-key context, independent prepared-tweak contexts, fixed-round encryption/decryption, exact in-place support, checked lengths/capacity/overlap and failure-atomic outputs. Context-owned secrets are cleansed before free. Research entrypoints and failure hooks are excluded from the installed library.

## Build and validate

Requires CMake 3.20+, a C11 compiler and OpenSSL 3.x libcrypto. Testing, research and benchmarks additionally require Python 3.11+ standard libraries.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCRYPTO_BUILD_RESEARCH=ON -DCRYPTO_BUILD_BENCHMARKS=ON
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target check-differential research-smoke
cmake --build build --config Release --target research-full
cmake --build build --config Release --target bench
cmake --build build --config Release --target check-configurations
```

`check-differential` compares two independent implementations: 1,024 combinations, 24 subkeys and 25 prefixes each. It is not a differential attack. `research-full` has a 30-minute default budget and records `completed`, `incomplete` or `error` without silently reducing samples. Results go under the build directory. Benchmarks preserve at least five samples of 200ms per mode and separate key/tweak setup from prepared blocks.

For library-only builds, use `-DBUILD_TESTING=OFF`; research and benchmark options default OFF. Python is then unnecessary. Install with `cmake --install build --prefix /your/prefix --config Release`. Consumers use `find_package(DualForm CONFIG REQUIRED)` and `DualForm::dualform`; see the [external consumer example](examples/consumer/main.c).

Linux GCC/Clang, Windows MSVC x64, macOS AppleClang and Linux ASan+UBSan are covered by [CI](https://github.com/Koreapanda4444/DualForm-T128/actions). Local LeakSanitizer environment limitations and the scope of machine-code review are recorded in the validation document.

## Research status

The complete bounded experiment ran 1,572,864 fixed-difference pairs and 655,360 related-tweak pairs. Two-round SK0/SK1 recovery and a four-round statistical distinguisher were reproduced. These are not master-key recovery or 24-round breaks. Full-round security, broad attack coverage and a universal constant-time proof remain unresolved. This baseline is not approved for protecting real secrets.

The SHAKE-context reuse experiment preserved all outputs but did not show a consistent overall performance benefit, so it was not adopted. Its measurements and source hashes remain in the history and result artifacts.

## Dependencies and license

OpenSSL supplies SHAKE256 and secret cleansing; the project does not implement a new hash. OpenSSL 3.x is an external dependency under Apache-2.0; Python's hashlib may use the same backend, so this is not independent double validation of SHAKE itself. No source or history was imported from an earlier cipher implementation. The repository owner has not selected a distribution license for this project.
