# DualForm-T128

명세에 정의된 128비트 블록·256비트 키·128비트 공개 tweak의 24라운드 암호를 구현한 C11 연구 프로젝트입니다. 비트 크기는 검증된 보안 강도가 아니며 인증·운용모드를 제공하지 않습니다.

[English](README.md) · [규범 명세](docs/spec_ko.md) · [API·설치](docs/api_ko.md) · [보안 연구](docs/security_ko.md) · [성능 실측](docs/benchmark_ko.md) · [검증 기록](docs/validation_ko.md) · [커밋 진행표](docs/progress_ko.md)

기본 키 컨텍스트와 부모에 독립적인 준비 tweak 컨텍스트를 분리합니다. 공개 API는 항상 24라운드이며 길이·출력 용량·부분 중첩을 검사하고 exact in-place를 지원합니다. 오류 시 출력은 바꾸지 않으며 소유 비밀값은 해제 전에 소거합니다. 설치 라이브러리에 연구용 prefix·역함수·실패 주입 entrypoint를 포함하지 않습니다.

## 빌드와 재현

CMake 3.20+, C11 컴파일러, OpenSSL 3.x libcrypto가 필요합니다. 테스트·연구·벤치에는 Python 3.11+ 표준 라이브러리가 추가로 필요합니다.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCRYPTO_BUILD_RESEARCH=ON -DCRYPTO_BUILD_BENCHMARKS=ON
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target check-differential research-smoke
cmake --build build --config Release --target research-full
cmake --build build --config Release --target bench
cmake --build build --config Release --target check-configurations
```

`check-differential`은 차분 공격이 아닌 C/Python 교차 대조입니다. 1,024조합의 24개 서브키·25개 prefix를 대조합니다. `research-full`의 기본 예산은 30분이며 표본을 몰래 줄이지 않고 completed/incomplete/error를 기록합니다. 벤치는 모드별 200ms 이상 표본 5개를 보존하며 준비 비용과 블록 연산을 분리합니다. 실행 결과는 빌드 디렉터리에 생성됩니다.

라이브러리 전용은 `-DBUILD_TESTING=OFF`로 빌드합니다. 연구·벤치 옵션 기본값은 OFF이므로 Python이 필요하지 않습니다. `cmake --install build --prefix /your/prefix --config Release`로 설치한 뒤 외부 프로젝트에서 `find_package(DualForm CONFIG REQUIRED)`와 `DualForm::dualform`을 사용합니다. [소비자 예제](examples/consumer/main.c)는 설치 후 별도 빌드·KAT로 검증합니다.

[CI](https://github.com/Koreapanda4444/DualForm-T128/actions)는 Linux GCC/Clang, Windows MSVC x64, macOS AppleClang 및 Linux ASan+UBSan을 검사합니다. 실제 run별 상태와 로컬 LeakSanitizer 환경 제한·기계어 검토 범위는 검증/진행 문서에 구분합니다.

## 연구 완료와 한계

고정 차분 1,572,864쌍과 관련 tweak 655,360쌍을 정해진 범위까지 모두 실행했습니다. **2라운드 SK0·SK1 복구**와 **4라운드 통계 구별 편향**을 재현했습니다. 마스터키 또는 24라운드 공격으로 해석하지 않습니다. 전체 라운드 안전성 증명·광범위 공격 분석·보편적인 상수시간 증명은 미해결이며 실제 기밀 보호용 사용을 승인하지 않습니다.

SHAKE 컨텍스트 재사용은 출력·공격 counts를 보존했으나 일관된 전체 성능 이득을 확인하지 못해 채택하지 않았습니다. 실험 기록과 소스 해시는 이력 및 결과 JSON에 남깁니다.

## 의존성과 라이선스

SHAKE256·비밀 소거는 외부 OpenSSL을 사용합니다. OpenSSL 3.x는 Apache-2.0 의존성이며 Python hashlib가 같은 백엔드를 쓸 수 있으므로 SHAKE 자체의 독립 이중 검증이라고 부르지 않습니다. 이전 암호 구현의 소스·이력은 가져오지 않았습니다. 소유자의 프로젝트 배포 라이선스 선택은 미정입니다.
