# DualForm-T128 신규 개발 명세서와 커밋 계획

문서 개정: **2.0 / 2026-11-09**. 문서의 개정 번호이며 암호 알고리즘을 바꾼 버전 번호가 아니다.

- 신규 GitHub 대상: `Koreapanda4444/DualForm-T128`
- 기본 브랜치: `main`
- 개발 범위: C11 라이브러리, 독립 검증, 한정된 암호분석, 성능 측정, 문서, CI
- 계획: **34개 커밋**. 실제 커밋 제목은 10절의 제목을 사용한다.
- 필수 첨부물: **이 파일 하나**. 이전 대화·다른 명세·원본 ZIP·기존 코드 없이 시작한다.

## 1. 새 프로젝트 실행 원칙

새 Work와 새 작업 디렉터리에서 이 문서의 정의를 구현한다. 이전 저장소를 clone하거나 소스·커밋을 복사·cherry-pick하지 않는다. 이전 원격의 수정·삭제·강제 push도 하지 않는다. 새 원격 이름을 위와 같이 정한 것이지 저장소의 존재와 접근 권한을 확인했다는 뜻은 아니다.

신규 원격에 README·LICENSE 등의 초기 파일만 있으면 이를 확인하고 보존한다. 이미 암호 구현이 있거나 원격 대상이 불명확하면 덮어쓰지 않고 사용자에게 확인한다. 원격이 아직 없거나 인증이 필요한 경우 가능한 로컬 구현·테스트·커밋은 계속하되 push는 미완료로 보고한다. 접근 제한을 우회하지 않는다.

사용자가 `N번까지`를 지정하면 10절의 해당 범위까지 수행한다. 별도 범위가 없으면 1번부터 순서대로 구현·검증·커밋하며, 중단 시 실제 완료 지점과 다음 번호를 남긴다. 이 파일 밖의 추가 지시문으로 기존 지시를 무효화할 필요가 없도록 작성된 명세다.

현재 대상은 **128비트 블록·256비트 키·128비트 공개 tweak의 24라운드 연구용 블록 암호**다. 128/256이라는 크기는 검증된 보안 강도를 뜻하지 않는다. 실행 가능한 구현의 완성, 측정된 성능, 암호학적 안전성은 별도로 판정한다. 기능과 연구 산출물을 완료해도 실제 비밀 보호에 안전하다는 결론이 자동으로 생기지 않는다.

이번 개발에서는 수식·키 도메인·라운드 수·바이트 순서를 바꾸지 않는다. 구조적 문제가 발견되면 최소 재현과 영향 범위를 기록하고 개선안을 제시한다. 사용자 승인 없이 새 설계로 교체하거나 테스트 벡터를 코드에 맞춰 고치지 않는다. GUI·서버·DB·파일 암호화 앱·AEAD·추가 운용모드·다중언어 바인딩·SIMD·어셈블리·GPU 구현은 범위 밖이다.

## 2. 기술 구성과 저장소 구조

세 암호 프로젝트의 공통 개발 구성은 다음으로 고정한다.

- C11, CMake 3.20 이상, CTest.
- OpenSSL 3.x `libcrypto`: 규정된 SHAKE256, 비밀 버퍼 소거 및 비교 벤치에 사용한다. SHAKE나 다른 해시를 새로 설계하지 않는다.
- Python 3.11 이상 표준 라이브러리: `hashlib`, `json`, `subprocess`, `math`, `statistics` 등. 필수 SciPy·NumPy·SAT/SMT 설치를 요구하지 않는다.
- Linux GCC/Clang, Windows MSVC x64, macOS AppleClang을 CI에서 검증한다. 플랫폼별 컴파일 옵션을 분리한다. 실행하지 못한 환경은 미검증으로 남긴다.
- 공식 암호 코어는 C 구현 하나다. `tests/reference.py`는 C 코드를 호출하지 않고 수식을 별도로 작성한 검증 도구다. `hashlib`와 C OpenSSL이 같은 SHAKE 백엔드를 사용할 수 있으므로 이를 SHAKE 자체의 독립 이중 구현 검증이라고 부르지 않는다.

| 경로 | 역할 |
| --- | --- |
| `include/dualform.h` | 설치하는 유일한 공개 헤더, 전체 24라운드 API |
| `src/dualform.c` | 공개 API, 컨텍스트, 블록 암·복호화 |
| `src/dualform_internal.h` | 내부 자료형·함수 선언, 설치하지 않음 |
| `src/dualform_core.c` | 치환·라운드·키 및 tweak 처리 |
| `src/shake256.c`, `src/shake256.h` | 오류를 검사하는 OpenSSL SHAKE256 어댑터, 자체 SHAKE 구현 아님 |
| `tests/` | CTest 실행 시험, `vectors.json`, `reference.py`, `vector_runner.c` |
| `research/` | 축소 라운드 공격·통계 실험·실행기 |
| `research/results/` | 명세·소스 해시와 조건을 포함한 작은 재현 결과 JSON |
| `bench/` | C 내부 반복으로 측정하는 벤치 및 결과 요약 실행기 |
| `docs/spec_ko.md` | 유일한 규범 사양, 이 문서의 수학 정의와 결정 사항 |
| `docs/api_ko.md` | 길이·오류·소유권·버퍼 계약 |
| `docs/security_ko.md` | 공격 모델, 재현 결과, 한계, 미해결 연구 |
| `docs/benchmark_ko.md` | 측정 환경·조건·실측 결과 |
| `docs/progress_ko.md` | 계획 번호, 완료 SHA, 검증, push·CI 상태 |
| `.github/workflows/ci.yml` | 실제 실행되는 플랫폼·sanitizer 검사 |
| `CMakeLists.txt`, `README.md`, `.gitignore` | 공통 빌드·안내·생성물 제외 |

파일 수는 필요에 따라 이 디렉터리 안에서 늘릴 수 있지만, 사용하지 않는 빈 폴더·중복 코어·`legacy/`·과거 버전 백업을 만들지 않는다. 빌드 결과·캐시·대량 원시 로그는 `build/`, `build-sanitize/` 아래 두고 커밋하지 않는다. 분석 결과에는 실제 테스트에 사용한 공개 시험 키만 기록하며 사용자의 비밀키를 기록하지 않는다.

축소 라운드 수, 내부 상태, 공격용 역함수는 테스트·연구 전용 빌드에서만 사용한다. 공개 헤더에 `rounds` 인자를 넣지 않으며 설치 라이브러리가 연구 전용 entrypoint를 노출하지 않도록 한다. 연구용 경로는 같은 내부 라운드 연산을 호출하고 별도 공식 코어를 만들지 않는다.

## 3. 규범 연산과 블록 함수

### 3.1 입력과 표기

- 블록과 tweak는 각각 16바이트, 키는 32바이트다.
- 블록은 첫 8바이트가 `L`, 다음 8바이트가 `R`인 little-endian `uint64_t` 두 워드다.
- `LE32`, `LE64`는 명시적인 little-endian 읽기·직렬화다. 호스트 엔디언이나 정렬에 의존하는 포인터 cast로 대체하지 않는다.
- `ROL32`, `ROL64`는 해당 폭의 순환좌회전이다. 회전수는 폭으로 나눈 나머지를 사용하며 0 회전도 정의되어야 한다. C에서 32/64비트 전체 폭만큼 shift하지 않는다.
- XOR는 `^`, AND는 `&`다. 덧셈·뺄셈은 지정된 32/64비트 폭으로 줄인다. 부호 있는 overflow는 허용하지 않는다.
- `S`는 다음 PRESENT S-box를 64비트 워드의 각 4비트 니블에 독립 적용한다.

```text
SBOX = [12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2]
```

Python oracle은 표를 사용할 수 있다. C 공식 코어는 비밀 니블을 주소로 사용하는 조회를 하지 않고 Boolean 식 등 고정된 연산으로 동일 치환을 구현한다. 속도를 위해 비밀 의존 테이블로 바꾸지 않는다.

### 3.2 짝수 라운드 함수

```text
y = S(x)
F_even(x) = y ^ ROL64(y, 11) ^ ROL64(y, 37)
```

라운드 인덱스는 0부터 시작하므로 첫 라운드가 이 함수를 사용한다.

### 3.3 홀수 라운드 함수

아래는 반드시 순차 갱신한다.

```text
a = low32(x)
b = high32(x)
a = (a + ROL32(b, 7)) mod 2^32
b = b ^ ROL32(a, 11)
a = a ^ ROL32(b, 17)
b = (b + ROL32(a, 5)) mod 2^32
F_odd(x) = uint64(a) | (uint64(b) << 32)
```

### 3.4 암호화와 복호화

`SK[0..23]`은 4절에서 생성한다. 짝수 `r`은 `F_even`, 홀수 `r`은 `F_odd`를 쓴다.

```text
encrypt:
    for r = 0 .. 23:
        (L, R) = (R, L ^ F_r(R ^ SK[r]))
    output = LE64(L) || LE64(R)

decrypt:
    for r = 23 .. 0:
        (L, R) = (R ^ F_r(L ^ SK[r]), L)
    output = LE64(L) || LE64(R)
```

괄호 안 두 값은 갱신 전 상태로 계산해 동시에 대입한다. 마지막에 별도 swap을 하지 않는다. 테스트 전용 `q`라운드는 전체 24개 키 스케줄의 앞 `q`개를 사용하고 `r=0`부터 시작한다. `q=0`은 항등함수이며 암호화 라운드로 세지 않는다.

## 4. 키와 tweak 준비

### 4.1 기본 키 확장

```text
base = SHAKE256(ASCII("DualForm-T128/base") || 0x00 || key[32], 192 bytes)
B[i] = LE64(base[8*i : 8*i+8]), i = 0 .. 23
```

기본 키 컨텍스트는 원본 키 32바이트와 `B[24]`를 소유한다. 원본 키는 이후 tweak 준비에도 필요하다. 호출자의 입력 키 버퍼를 참조로 보관하지 않는다.

### 4.2 키 의존 tweak 인코딩

tweak를 LE64 `(L,R)`로 읽고 다음 6라운드를 수행한다.

```text
for i = 0 .. 5:
    h = LE64(SHAKE256(
        ASCII("DualForm-T128/tweak-preperm") || 0x00
        || byte(i) || key[32] || LE64(R), 8 bytes))
    (L, R) = (R, L ^ h)
enc.low64 = L
enc.high64 = R
```

`byte(i)`는 ASCII 숫자가 아니라 정수 0~5의 한 바이트다. 도메인 뒤의 `0x00`은 정확히 한 바이트다. 직렬화할 때 C 문자열의 종결 NUL을 중복해서 넣지 않는다.

고정 키에서 이 6라운드 Feistel은 tweak 공간의 전단사다. 역변환이 필요할 때는 연구 전용으로 `i=5..0`에서 현재 `L`을 SHAKE 입력에 사용하고 `(L,R)=(R ^ h, L)`로 되돌린다. 전단사라는 사실 자체는 관련-tweak 공격 저항성 증명이 아니다.

### 4.3 tweak별 서브키

```text
derived = SHAKE256(ASCII("DualForm-T128/derived") || 0x00
                   || key[32] || tweak[16], 192 bytes)
D[i] = LE64(derived[8*i : 8*i+8]), i = 0 .. 23

SK[0] = B[0] ^ enc.low64
SK[1] = B[1] ^ enc.high64
SK[i] = B[i] ^ D[i], i = 2 .. 23
```

`D[0]`, `D[1]`은 사용하지 않는다. 이 두 값을 SK0·SK1에 추가하는 것은 최적화가 아니라 설계 변경이다. 원본 tweak와 인코딩한 tweak를 혼동하지 않는다. `derived`에는 **원본 tweak**를 넣는다.

tweak별 컨텍스트는 계산을 끝낸 `SK[24]`를 자기 메모리에 보유한다. 생성이 끝나면 부모 키 컨텍스트에 의존하지 않는다. 따라서 부모를 해제해도 이미 만든 tweak 컨텍스트의 블록 연산은 가능하다. 새 tweak를 준비할 때는 살아 있는 키 컨텍스트가 필요하다.

### 4.4 준비 비용 경계

- 기본 키 준비: SHAKE API 1회, 192바이트 출력.
- 기본 키가 준비된 새 tweak: 인코딩 SHAKE 6회와 derived SHAKE 1회, 합계 7회.
- 새 키와 새 tweak: 합계 8회.
- 준비된 tweak로 블록 처리: SHAKE 호출 없음.

이것은 SHAKE API 호출 횟수이며 내부 Keccak 순열 호출 횟수나 실제 실행 시간과 같지 않다. 공개 tweak는 nonce가 아니다. 같은 키에서 tweak를 다시 쓰는 것을 API 오류로 처리하지 않는다. 같은 키·tweak·평문에는 같은 암호문이 나온다. 인증·태그·AD 기능은 제공하지 않는다.

## 5. 공개 C API와 오류 계약

`include/dualform.h`에는 다음 인터페이스를 제공한다. **기본 키와 준비된 tweak의 두 컨텍스트**를 분리한다.

```c
#include <stddef.h>
#include <stdint.h>

#define DUALFORM_KEY_BYTES 32u
#define DUALFORM_TWEAK_BYTES 16u
#define DUALFORM_BLOCK_BYTES 16u

typedef struct dualform_key_ctx dualform_key_ctx;
typedef struct dualform_tweak_ctx dualform_tweak_ctx;

typedef enum {
    DUALFORM_OK = 0,
    DUALFORM_EINVAL = 1,
    DUALFORM_ELENGTH = 2,
    DUALFORM_ECAPACITY = 3,
    DUALFORM_EOVERLAP = 4,
    DUALFORM_ENOMEM = 5,
    DUALFORM_ECRYPTO = 6
} dualform_status;

dualform_status dualform_key_ctx_create(
    dualform_key_ctx **out_ctx, const uint8_t *key, size_t key_len);
void dualform_key_ctx_free(dualform_key_ctx *ctx);

dualform_status dualform_tweak_ctx_create(
    dualform_tweak_ctx **out_ctx, const dualform_key_ctx *key_ctx,
    const uint8_t *tweak, size_t tweak_len);
void dualform_tweak_ctx_free(dualform_tweak_ctx *ctx);

dualform_status dualform_encrypt_block(
    const dualform_tweak_ctx *ctx,
    const uint8_t *input, size_t input_len,
    uint8_t *output, size_t output_capacity);
dualform_status dualform_decrypt_block(
    const dualform_tweak_ctx *ctx,
    const uint8_t *input, size_t input_len,
    uint8_t *output, size_t output_capacity);
```

### 5.1 길이와 생명주기

- 키 길이는 정확히 32, tweak와 입력 블록 길이는 정확히 16이다. 출력 용량은 16 이상이며 성공 시 앞 16바이트만 쓴다. 남은 출력 공간은 변경하지 않는다.
- `out_ctx`는 쓰기 가능한 포인터 슬롯을 가리켜야 하며 호출 전 `*out_ctx == NULL`이어야 한다. 슬롯은 입력 키·tweak 메모리와 겹치지 않는 별도 객체여야 한다. 성공 시에만 새 컨텍스트를 대입하고 실패 시 NULL을 유지한다. 이미 non-NULL인 슬롯은 `EINVAL`로 거부하고 그 값을 변경하지 않는다.
- 컨텍스트는 생성 이후 불변이다. 유효한 컨텍스트의 읽기 전용 블록 연산은 병행 가능하지만 같은 출력 버퍼를 병행해서 쓰거나 다른 연산 중 컨텍스트를 해제하는 것은 호출자 오류다.
- `free(NULL)`은 허용한다. 해제 전 원본 키·기본 키·서브키를 소거한다. 해제 후 포인터 사용과 이중 해제, 가짜 컨텍스트 포인터를 안전하게 감지할 수 있다고 약속하지 않는다.
- 모든 non-NULL 입력은 필요한 크기의 살아 있는 메모리 객체여야 한다. 포인터와 전달 길이만으로 실제 할당 크기나 접근 가능성을 확인할 수는 없다.

### 5.2 실패와 버퍼

| 조건 | 반환 |
| --- | --- |
| 필요한 NULL 포인터, non-NULL 생성 출력 슬롯 | `DUALFORM_EINVAL` |
| 키·tweak·블록 길이 불일치 | `DUALFORM_ELENGTH` |
| 출력 용량 16 미만 | `DUALFORM_ECAPACITY` |
| 입력과 출력의 부분 중첩 | `DUALFORM_EOVERLAP` |
| 컨텍스트 메모리 할당 실패 | `DUALFORM_ENOMEM` |
| OpenSSL SHAKE 초기화·갱신·종료 실패 | `DUALFORM_ECRYPTO` |

여러 인자가 동시에 잘못된 경우 위 표의 구체 오류 간 우선순위는 보장하지 않지만 성공을 반환하거나 버퍼를 변경해서는 안 된다. 블록 입력과 출력이 정확히 같은 주소이면 in-place를 지원한다. 서로 다른 시작 주소의 16바이트 처리 영역이 겹치면 거부한다. 컨텍스트 내부 메모리를 입출력으로 넘기는 것은 허용하지 않는다.

NULL·길이·용량·중첩 검사를 연산 전에 수행한다. 블록 입력을 지역 상태로 읽고 완성한 결과를 성공 시에만 출력에 복사한다. **오류 반환 시 호출자 데이터 출력은 변경하지 않는다.** 생성 실패 시 부분 할당과 SHAKE 임시 상태를 소거·해제한다. 시험 전용 실패 주입으로 메모리 및 SHAKE 실패 경로를 검사하되 공개 allocator 훅이나 실패 스위치를 만들지 않는다.

중첩 검사는 unrelated pointer끼리 `<` 비교하거나 뺄셈하지 않는다. Windows x64·macOS ARM64 등 지원하는 flat-address 플랫폼의 `uintptr_t` 주소 표현을 사용하여 범위 끝 계산의 overflow를 먼저 검사한다. `uintptr_t`를 제공하지 않는 대상은 지원하지 않으며 명확한 컴파일 진단을 낸다. 비교할 범위는 실제 읽고 쓰는 16바이트다. 정수 변환이 잘못된 포인터의 유효성을 보증하는 것은 아니다.

OpenSSL 호출의 반환값을 모두 확인하고 비밀 데이터의 소거에는 `OPENSSL_cleanse` 등 최적화로 제거되지 않도록 설계된 기능을 사용한다. 레지스터·컴파일러 복사본·swap까지 완전 소거한다고 주장하지 않는다. 비밀값에 따라 달라지는 분기와 메모리 접근을 피하고 실제 검사한 빌드의 범위를 기록한다.

## 6. 고정 벡터와 독립 구현 대조

다음은 이 후보의 재현용 고정 벡터다. 외부 표준 암호 인증 벡터가 아니다. 새 구현은 이 값을 그대로 검증하며, 불일치가 나면 도메인·NUL·순서·엔디언·swap부터 조사한다.

### 6.1 모두 0인 입력

| 항목 | Hex |
| --- | --- |
| key | `0000000000000000000000000000000000000000000000000000000000000000` |
| tweak | `00000000000000000000000000000000` |
| plaintext | `00000000000000000000000000000000` |
| 인코딩 tweak, LE64 L 및 R | `1f077752a0853e27a373bfc1c72aa0c7` |
| SK0, 정수 표기 | `bdd7239a08f69e71` |
| SK1, 정수 표기 | `e22dc2ee713134df` |
| 1라운드 출력 | `00000000000000001630897b462f7e09` |
| 2라운드 출력 | `1630897b462f7e090ae2b5845d595634` |
| 24라운드 출력 | `3a6831014e21bb35e87d4e4e458ddda0` |

### 6.2 순차 바이트 입력

| 항목 | Hex |
| --- | --- |
| key | `000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f` |
| tweak 및 plaintext | 각각 `000102030405060708090a0b0c0d0e0f` |
| 인코딩 tweak | `5a0fefd45741845ef750071d43f497f1` |
| SK0, 정수 표기 | `04c152132b664e63` |
| SK1, 정수 표기 | `cd5254628c0ff444` |
| 1라운드 출력 | `08090a0b0c0d0e0fa7900b432a21c54b` |
| 2라운드 출력 | `a7900b432a21c54b92fd8f9ae3d592a0` |
| 24라운드 출력 | `cab683110ef545617d4e892a779daacf` |

키·tweak·평문을 모두 `ff`로 채운 세 번째 fixture의 24라운드 출력은 `d653cb3700ff90e79631907cbebe9d8e`다.

SHAKE 어댑터는 빈 입력의 64바이트 출력도 아래 값과 비교한다. 별도로 출력 길이 8·192바이트, 분할 update와 한 번 update의 동일성을 검사한다.

```text
46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762fd
75dc4ddd8c0f200cb05019d67b592f6fc821c49479ab48640292eacb3b7c4be
```

고정 fixture 외에 1,024개 키·tweak·평문 조합을 결정적으로 생성한다. 아래 시험 생성 함수를 사용한다.

```text
sample(label,j,r,i,n) = SHAKE256(
    ASCII("DualForm-T128/test/") || ASCII(label) || 0x00
    || LE64(j) || LE32(r) || LE64(i), n bytes)
```

교차 대조 조합 `j=0..1023`은 label `kat-key`, `kat-tweak`, `kat-plain`, `r=i=0`으로 각각 32·16·16바이트를 얻는다. 이 함수는 공개 시험 입력 생성 규칙이며 사용자 키 생성 API가 아니다.

`check-differential`은 **차분 공격이 아니라 C/Python 두 구현의 결과 대조**다. 각 조합의 24개 서브키, 인코딩 tweak, 0..24라운드의 25개 prefix, 양쪽 복호화 결과를 대조한다. 최소 25,600개 prefix 비교가 이루어진다. Python이 C를 불러 정답을 생성하거나 C가 Python의 암호문을 하드코딩하면 독립 대조가 아니다.

`tests/vector_runner.c`는 테스트 전용으로 hex 입출력을 사용하는 작은 실행 파일로 만든다. Python이 JSON fixture를 읽고 이 실행 파일에 전달한다. C에 범용 JSON 라이브러리를 추가할 필요는 없다. trace·서브키 출력 기능은 이 검증 프로그램에서만 제공한다.

## 7. 재현할 연구 항목과 유한 실행 범위

아래 역사 관측은 이전 감사에서 보고된 결과다. 새 저장소에서 실행한 결과로 복사하지 않는다. 이 절의 규칙으로 새 결과와 실제 소스·명세 SHA-256을 남긴다.

### 7.1 라운드 함수 역변환과 2라운드 서브키 복구

짝수 함수의 선형층은 `L=I+ROL11+ROL37`이다. 다음 방법으로 역을 독립 구현할 수 있다.

```text
inverse_linear(y):
    for j = 0 .. 5:
        y = y ^ ROL64(y, 11 * 2^j) ^ ROL64(y, 37 * 2^j)
    return y

inverse_F_even(y) = inverse_S(inverse_linear(y))

inverse_F_odd(y):
    a = low32(y); b = high32(y)
    b = (b - ROL32(a, 5)) mod 2^32
    a = a ^ ROL32(b, 17)
    b = b ^ ROL32(a, 11)
    a = (a - ROL32(b, 7)) mod 2^32
    return uint64(a) | (uint64(b) << 32)
```

`inverse_S`는 공개 S-box의 역치환이다. `ROL64`의 회전수는 64로 줄인다. 각 반복 줄의 우변은 그 줄 갱신 전 `y`로 계산한다. 64개 기저 벡터로 선형층 rank 64와 역행렬 관계를 확인하고, 두 F 함수에 경계값과 결정적 10,000개 워드를 넣어 양방향 역변환을 검사한다.

평문 `(L,R)`와 **2라운드** 암호문 `(C_L,C_R)` 한 쌍에서:

```text
SK0 = R ^ inverse_F_even(C_L ^ L)
SK1 = C_L ^ inverse_F_odd(C_R ^ R)
```

1,024개 키·tweak 조합에서 복구하고 별도 평문 한 개로 검증한다. 공격 함수에는 확장키나 중간 상태를 전달하지 않는다. 정답 키와의 비교는 시험 하네스에서만 한다. 이것은 SK0·SK1 복구이며 마스터키나 전체 24라운드 복구가 아니다.

### 7.2 공개 tweak XOR 상쇄 대조군

옛 전체 암호를 복제하지 않는다. 다음 **두 라운드만 있는 최소 양성 대조군**을 연구 시험 안에서 구성한다.

```text
control_SK0 = B[0] ^ tweak.low64
control_SK1 = B[1] ^ tweak.high64
```

공개 tweak 차분이 `(a,b)`이고 평문 차분을 `(b,a)`로 주면 이 대조군의 2라운드 암호문 차분은 항상 `(b,a)`다. 1,024개 사례에서 그 등식이 성립하는지 검사한다. 같은 키·tweak·평문 선택 규칙을 현 4절 스케줄에 적용하여 동일 등식의 성립 횟수를 따로 기록한다.

대조군의 확률 1 등식은 결정적 회귀 조건이다. 현 스케줄의 성공 횟수를 반드시 0으로 강제하거나, 0회 관측을 모든 관련-tweak 공격의 부재로 해석하지 않는다. 연구 전용 최소 대조군은 설치 라이브러리에 포함하지 않는다.

### 7.3 고정된 4라운드 차분 후속

- 입력 차분: `ΔL = 0x0000000080000000`, `ΔR = 0`.
- 관측 출력: 직렬화된 128비트 출력의 **0-based bit 1**, 즉 출력 `L`의 bit 1. byte 0의 마스크 `0x02`다.
- 고정 키에서 같은 tweak를 쓰는 두 평문을 암호화하고 두 암호문의 해당 비트 XOR가 1인 횟수를 센다.
- 이전 보고: 8개 키·tweak 조합에서 각각 32,768쌍을 측정했을 때 4라운드 반전율 약 99.9359~99.9664%. 새 코드가 이 정확한 소수점 값을 출력해야 한다는 의미는 아니다.
- full 실행: 키·tweak 조합 8개, 라운드 `{4,5,6,7,8,24}`, 조합별·라운드별 **32,768쌍**. 총 48개 사전 고정 검정, 1,572,864쌍이다.

키와 tweak는 `sample("bias-key",j,0,0,32)`, `sample("bias-tweak",j,0,0,16)`에서 얻는다. 평문은 `sample("bias-plain",j,r,i,16)`이고 두 번째 평문에는 위 차분을 XOR한다. 관측 후 출력 비트·차분을 바꿔 같은 표본에서 유리한 결과를 다시 고르지 않는다.

`k/n`, 표본 수, 원시 횟수를 반드시 남긴다. 기본 분석은 독립 Bernoulli 근사 아래 다음 보수적인 Hoeffding 상한을 기록한다.

```text
d = abs(k/n - 0.5)
log10_p_upper = min(0, log10(2) - 2*n*d*d / ln(10))
```

48개 검정의 다중비교 보정은 이 상한에 48을 곱하되 1을 넘지 않도록 한다. 로그값도 보존해 수치 underflow를 수학적 확률 0으로 표기하지 않는다. 이는 특정 입력 분포에서 선택한 출력 비트의 통계 실험이지 차분 trail/hull의 확률 증명이나 24라운드 안전성 하한이 아니다. 작은 표본의 p-value를 기본 CI 성공·실패 조건으로 삼지 않는다.

### 7.4 한정된 관련-tweak 스크리닝

추가 full 실험은 아래 범위로 한정한다.

- 키 8개, 라운드 `{2,4,8,12,24}`.
- tweak 차분 `(a,b)`는 `(1,0)`, `(0,1)`, `(1,1)`, `(2^63,2^63)`의 4개.
- 각 조합 4,096쌍, 두 평문 차분은 `(b,a)`, tweak 차분은 `(a,b)`.
- 총 160개 설정, 655,360쌍. 각 설정에서 출력 128개 bit의 flip count를 기록한다.

키는 `sample("related-key",j,0,0,32)`로 생성한다. 차분 목록의 인덱스를 `v=0..3`으로 두고, tweak와 평문은 각각 `sample("related-tweak",4*j+v,r,i,16)`, `sample("related-plain",4*j+v,r,i,16)`에서 얻는다. 이 범위에서 통계 유의를 주장할 때는 조사한 20,480개 bit 검정 전체를 보정 대상에 포함한다. 추가 탐색을 한다면 새로운 독립 holdout을 별도 실행으로 기록한다.

### 7.5 연구 실행기 계약

| 대상 | 실행 범위 |
| --- | --- |
| `research-smoke` | 역함수 기저·경계 검사, 서브키 복구·상쇄 대조 각 16사례, 고정 4라운드 1키×256쌍 및 결과 스키마 검사 |
| `research-full` | 7.1의 10,000워드·1,024조합, 7.2의 1,024대조, 7.3의 48검정, 7.4의 160설정 |

full 기본 실행 예산은 실제 계산 30분이다. 정해진 표본 또는 예산에 도달하면 종료한다. 예산 초과·환경 장애는 `incomplete`로 기록하며 표본 수를 몰래 줄여 완료로 처리하지 않는다. 사용자가 별도로 요청하면 예산을 늘려 재개할 수 있다. 시간 예산은 안전성 합격 기준이 아니다.

결과 JSON은 실험명, 코어·명세 해시, 도구·컴파일러 버전, 입력 생성 규칙, 라운드, 키·tweak fixture 또는 생성 식, 차분·bit 번호, 요청/실행 표본 수, counts, elapsed, `completed/incomplete/error`를 포함한다. 결정적 등식의 반례는 테스트 실패로 남긴다. 통계 예상과 다른 결과는 조사 대상으로 기록하고 알고리즘을 조용히 바꾸지 않는다.

SAT/MILP 전 범위 증명, 차분 hull 합산, 모든 관련키·integral·boomerang·MITM 분석은 이번 34커밋의 필수 완료 조건이 아니다. 완료하지 않은 연구 주제로 보안 문서에 남긴다. 특정 공격이 발견되지 않았다는 이유로 모든 공격을 막는다고 표현하지 않는다.

## 8. 성능 측정과 비트 호환 최적화

벤치의 다섯 측정 모드를 분리한다.

| 모드 | 측정하는 실제 작업 |
| --- | --- |
| 준비된 블록 | 생성해 둔 tweak 컨텍스트로 전체 24라운드 암호화 또는 복호화 |
| 새 키 준비 | 기본 키 컨텍스트 생성·소거·해제 |
| 새 tweak 준비 | 기본 키를 재사용하면서 tweak 컨텍스트 생성·소거·해제 |
| 새 tweak 및 블록 | 기본 키 재사용, 새 tweak 준비·블록 연산·tweak 해제 |
| 새 키·tweak 및 블록 | 두 컨텍스트 생성·블록 연산·두 컨텍스트 해제 |

키·tweak·평문 입력 생성은 측정 밖에서 하며, 새 tweak 모드는 순서가 고정된 서로 다른 tweak를 사용한다. 공개 API의 할당·소거 비용은 위 정의대로 포함한다. 내부 순수 함수 비용을 추가로 잴 때는 별도 진단값으로 표시하고 공개 API 결과와 섞지 않는다. SHAKE 호출 수 검사도 시간 측정과 분리한다.

같은 호스트의 Release 빌드에서 warmup 뒤 최소 5개 표본을 수집한다. 각 표본은 보정한 반복 수로 약 200ms 이상 측정하고 체크섬을 관측하여 최적화 제거를 방지한다. 단조 시간원을 사용하며 플랫폼별 구현을 구분한다. ns/op와 ops/s를 기록하고, 블록을 실제 처리하는 모드에만 16바이트 기준 MiB/s를 함께 기록한다. 준비만 하는 작업에 암호화 처리량을 붙이지 않는다.

CPU·OS·컴파일러·최적화 옵션·OpenSSL 버전·하드웨어 가속 여부·표본 원시값·중앙값·최솟값·최댓값을 남긴다. AES-256 단일 블록 등 OpenSSL 비교는 같은 호출·준비 경계와 다른 기능 차이를 명시한 참고값이다. AES-XTS를 함께 측정한다면 최소 단위와 데이터 단위·키 길이·tweak 준비 비용이 다른 별도 사용 사례로 표시하고 동등한 primitive라고 주장하지 않는다.

이전 감사 구현의 참고값은 준비된 블록 약 0.164µs, 새 tweak 준비와 블록 약 5.085µs였다. 이는 당시 호스트·할당 전략의 수치이며 새 코드의 결과나 합격선이 아니다. 새 저장소는 위 다섯 모드를 직접 측정한다.

최적화는 현재 키base 재사용과 **한 번의 tweak 준비 호출 안에서 EVP 컨텍스트를 순차 재사용하는 것**부터 시도한다. 각 SHAKE 메시지마다 digest를 완전히 재초기화해야 하며 여러 도메인을 한 스트림으로 합치지 않는다. 공유 전역 SHAKE 컨텍스트를 쓰지 않고 동시 호출의 상태를 분리한다. 변경 전후 고정 벡터·독립 구현 대조·공격 회귀가 일치해야 한다. 유의미한 개선이 없으면 결과를 기록하고 무리한 최적화를 유지하지 않는다.

## 9. 빌드·검증·CI와 Git 규칙

### 9.1 공통 명령

각 target은 담당 커밋에서 구현된 이후부터 실행한다. 아래는 전체 기능이 들어온 뒤의 동일한 개발 명령이다.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCRYPTO_BUILD_RESEARCH=ON -DCRYPTO_BUILD_BENCHMARKS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target check-differential
cmake --build build --config Release --target research-smoke
cmake --build build --config Release --target research-full
cmake --build build --config Release --target bench
```

`CRYPTO_BUILD_RESEARCH`와 `CRYPTO_BUILD_BENCHMARKS`의 기본값은 OFF다. `BUILD_TESTING=ON`에서 CTest와 `check-differential`을 제공하고, 연구/벤치 옵션이 ON일 때 해당 target을 제공한다. Python이 필요한 옵션을 켰는데 Python이 없으면 명확한 configure 오류를 낸다. 라이브러리만 빌드할 때는 Python을 요구하지 않는다.

```sh
cmake -S . -B build-sanitize -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCRYPTO_ENABLE_SANITIZERS=ON
cmake --build build-sanitize --config Debug
ctest --test-dir build-sanitize -C Debug --output-on-failure
```

sanitizer 옵션은 Linux GCC/Clang의 ASan+UBSan에 적용한다. 지원하지 않는 조합에는 잘못된 플래그를 전달하지 말고 명확히 알려야 한다. 도구 제약 때문에 일부 검사만 생략했다면 검사명과 사유를 남긴다. 모든 기능이 완성되기 전부터 빈 테스트나 성공만 반환하는 stub으로 CI를 녹색 처리하지 않는다.

### 9.2 커밋과 진행 기록

커밋 제목은 `feat:`, `fix:`, `test:`, `docs:` 네 접두어로 통일한다. 10절의 번호와 Phase는 진행표에서만 쓴다. 제목에 `DU-001`, `CR-002`, `IN-001`, `Commit 1`, `Phase 2` 등을 붙이지 않는다. 계획 번호를 기준으로 완료 범위를 사용자에게 보고하되 실제 SHA도 함께 남긴다.

각 행은 관련 테스트를 포함한 빌드 가능한 변경이다. 로컬 검사 후 commit하고, 권한이 있으면 push한 뒤 원격 CI를 확인한다. CI가 실패하면 다음 기능을 진행하기 전에 원인을 해결한다. 권한·환경 문제와 코드 결함은 구분하며, 실패를 숨기려고 검사를 삭제하지 않는다. 초기에 빌드 scaffold만 있는 커밋은 configure/compile smoke만 검사했다고 정확히 보고한다.

34개는 사전 작업 계획이며 빈 커밋을 만들기 위한 숫자가 아니다. 예상 밖 결함을 고치는 추가 `fix:` 커밋이 필요하면 허용하되 계획번호와 추가 SHA를 구분해 이유를 기록한다. 이미 큰 변경을 만든 뒤 기록만 맞추려고 인위적으로 쪼개거나 과거 커밋을 재작성하지 않는다. 새 저장소의 정상 이력도 사용자 승인 없이 reset·squash·rebase·force-push하지 않는다.

진행표 한 행에는 계획번호, 상태, 실제 완료 SHA, 마지막 실행 명령과 결과, push 여부, CI run 상태를 적는다. 자기 커밋 SHA는 그 커밋을 만들기 전에는 알 수 없으므로 다음 상태 갱신에 기록하거나 Git 이력에서 조회한다. SHA를 적으려고 무한히 amend하지 않는다.

## 10. 확정 커밋로그

### Phase 1. 신규 저장소와 공통 기반 — 1~4

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| --- | --- | --- | --- |
| 1 | `docs: define fresh repository scope and commit plan` | 새 대상, 본 규범 사양, 34개 진행표, 연구용 README와 제외 범위 | 옛 원격·외부 필수 첨부 의존 없음, 수식·KAT·계획 번호 대조 |
| 2 | `feat: add portable cmake build and test harness` | C11/OpenSSL 탐색, 공통 옵션, 정수 폭 compile/run smoke, 생성물 제외 | 깨끗한 build configure·compile·실제 smoke 통과; 가짜 암호 API 없음 |
| 3 | `feat: implement endian and rotation primitives` | 명시 LE32/LE64 load/store, UB 없는 회전, 관련 단위 시험 | 정렬되지 않은 유효 버퍼, 0/1/31/32/63/64 회전과 왕복 검사 |
| 4 | `feat: add checked shake256 backend` | EVP SHAKE256 어댑터, 오류 처리·임시 상태 소거 | 빈 입력 64B KAT, 8/192B 출력, 분할 update 동일성 |

### Phase 2. 규범 암호와 공개 API — 5~12

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| --- | --- | --- | --- |
| 5 | `feat: implement constant-flow nibble substitution` | 비밀 인덱스 없는 16니블 S-box | 16개 값 전수·각 니블 위치 대조, C의 비밀 테이블 조회 없음 |
| 6 | `feat: implement alternating round functions` | F_even의 선형층과 순차 F_odd ARX | 경계 워드·carry·32/64비트 결합, 고정 수식 기준 결과 대조 |
| 7 | `feat: implement cached base key context` | opaque 기본 키 생성/해제, 원본키와 B24 복사 소유 | 길이·NULL·출력 슬롯·생성 실패 처리, SHAKE 직접 계산과 B24 일치 |
| 8 | `feat: implement keyed tweak permutation` | 6라운드 전처리 및 연구 전용 역변환 | 0/순차 tweak 인코딩 KAT, 역변환 왕복, byte(i)·도메인 경계 |
| 9 | `feat: implement independent python reference` | hashlib 기반 단일 oracle, 수식·키스케줄·prefix trace | 고정 벡터와 자체 역변환 통과, C 실행 결과를 정답으로 사용하지 않음 |
| 10 | `feat: implement prepared tweak context` | derived와 SK24 생성, 부모에 독립적인 준비 컨텍스트 | SK0·SK1 KAT, D0·D1 미사용 확인, 부모 해제 후 준비 키 보존 |
| 11 | `feat: implement full-round block encryption` | 길이·capacity·overlap 검사 포함 공개 24라운드 암호화 | 3개 고정 암호문, exact in-place, 실패 출력불변 |
| 12 | `feat: implement full-round block decryption` | 공개 24라운드 복호화, 역라운드·최종 swap 규칙 | 3개 고정 암호문 복호화, 암복호화 왕복, 동일 오류 계약 |

### Phase 3. 기능 정합성과 구현 안전성 — 13~18

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| --- | --- | --- | --- |
| 13 | `test: add fixed vectors and round traces` | vectors.json, 테스트 전용 vector runner, 0..24 trace | 본문의 중간값·최종값 일치, 설치 대상에서 연구 심볼 제외 |
| 14 | `test: cross-check independent implementations` | check-differential target, 1,024조합 C/Python 대조 | 25,600개 prefix, SK24·인코딩·양쪽 복호화 불일치 0 |
| 15 | `test: enforce context ownership and failure contracts` | 소유권·부모 해제·생성 실패 주입 회귀 | 각 할당/SHAKE 실패 위치에서 누출·출력 슬롯 변경·잔여 사용 없음 |
| 16 | `test: cover buffer boundaries and aliasing` | 유효 경계 버퍼, NULL·길이·capacity·in-place·partial overlap 회귀 | 앞뒤 sentinel, 잘못된 길이, 출력 잔여 영역 불변, 실패 입력/출력 보존 |
| 17 | `test: add sanitizer regression checks` | sanitizer 옵션과 테스트 경로, 반복 생성/해제·블록 시험 | Linux ASan+UBSan Debug 결과, 생략된 검사와 실제 실행 환경 기록 |
| 18 | `test: verify portable compiler and platform builds` | Linux GCC/Clang·Windows MSVC·macOS CI | 플랫폼별 configure/build/CTest·oracle 대조, 미검증 환경 별도 표시 |

### Phase 4. 한정된 공격 재현과 연구 — 19~25

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| --- | --- | --- | --- |
| 19 | `feat: add reproducible research runner` | smoke/full target, deterministic sample, JSON 스키마·시간 예산 | 같은 입력 반복 일치, timeout은 incomplete, 중단 시 실행 표본 보존 |
| 20 | `test: verify round function inversion` | S-box 역·선형층 rank·F_even/F_odd 역함수 연구 검사 | 64기저·경계·10,000워드 양방향 검사, round0/1 대응 확인 |
| 21 | `test: reproduce reduced-round subkey recovery` | 외부 P/C만 받는 2라운드 SK0·SK1 복구 | 1,024조합 복구와 두 번째 평문 확인, 마스터키 공격으로 표기하지 않음 |
| 22 | `test: reproduce public tweak cancellation control` | 2라운드 최소 양성 대조군과 현 스케줄 비교 | 대조 등식 1,024사례 성립, 현 후보 성공 횟수는 관측값으로 출력 |
| 23 | `test: reproduce fixed four-round differential` | 8키·6라운드·고정 ΔL와 bit1 후속 | 정확한 bit 정의, 48검정 counts·로그 상한·다중비교, CI p-value gate 없음 |
| 24 | `test: screen bounded related-tweak differences` | 4개 공개 차분의 160설정 스크리닝 | 655,360쌍 예산, 128bit counts, 20,480검정 보정과 탐색 한계 명시 |
| 25 | `docs: record attack models and security limitations` | 현재 코어의 실제 성공 공격·불발·미해결을 보안 문서에 정리 | 결과 해시·실행 상태 대조; 과거 수치를 새 결과로 복사하지 않음 |

### Phase 5. 성능과 비트 호환 최적화 — 26~30

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| --- | --- | --- | --- |
| 26 | `feat: add separated setup and block benchmarks` | 다섯 모드와 준비된 복호화, C 내부 루프·시간원·체크섬 | 준비모드 SHAKE 호출 경계, 키/tweak 입력 생성 비용 제외, 실제 결과 체크섬 |
| 27 | `test: validate benchmark timing and workload accounting` | warmup·반복수 보정·5표본·소거/할당 포함 계약 검증 | 단위 환산, 준비 전용 throughput 미표기, 잘못된 측정값과 0시간 거부 |
| 28 | `feat: add labeled openssl comparison benchmarks` | 준비 조건이 명시된 OpenSSL 참고 측정 | 하드웨어 가속·키·블록/데이터 단위 차이 표시, 같은 호스트에서 비교 |
| 29 | `feat: reuse shake contexts within tweak preparation` | 호출 내부 EVP 컨텍스트 재사용과 오류 정리, SHAKE마다 재초기화 | 최적화 전후 전체 KAT·독립 대조·공격 회귀 일치, 전역 공유 상태 없음 |
| 30 | `test: audit secret handling and optimized equivalence` | 비밀 접근·소거·최적화 회귀 및 빌드별 기계어 점검 기록 | sanitizer 재실행, 검사한 함수·옵션 명시, 상수시간 보편 증명 주장 금지 |

### Phase 6. 설치·재현·완료 기록 — 31~34

| 번호 | 실제 커밋 제목 | 구현·산출물 | 이 커밋의 검증 |
| --- | --- | --- | --- |
| 31 | `docs: publish measured performance and research results` | 실측 JSON·성능 문서·최적화 전후 표·연구 결과 | 원시 5표본·환경·코어 해시 일치, 과거 참고값과 새 실측 분리 |
| 32 | `feat: add library installation and consumer smoke tests` | CMake install, 공개 헤더·라이브러리·외부 소비자 예제 시험 | 임시 prefix 설치 후 별도 C 소비자 링크·KAT, 내부/연구 헤더 미설치 |
| 33 | `test: verify clean checkout and complete validation targets` | 완전히 새 build의 전체검사 및 옵션조합 회귀 | CTest·check-differential·research-smoke, full 상태·bench·설치 시험 확인 |
| 34 | `docs: finalize research baseline and handoff status` | README 사용법·API/위험/성능 연결·완료표·남은 연구 | 실제 SHA/push/CI·full 완료 여부 구분, 전체 24라운드 안전 승인으로 표기하지 않음 |

Phase 범위 합계는 4 + 8 + 6 + 7 + 5 + 4 = **34**다. 추가 결함 수정은 별도 실제 SHA로 기록하며 이 표의 번호를 커밋 제목에 복사하지 않는다.

## 11. 완료 판정과 보고

다음 항목을 각각 판정한다.

1. **기능:** 규범 수식·키스케줄·API·고정 벡터·독립 대조가 일치한다.
2. **구현 검사:** 실패 원자성·수명·중첩·sanitizer·CI에서 실제 실행한 검사를 기록한다.
3. **연구:** 정해진 deterministic 공격은 재현했고 통계 full 실행은 completed인지 incomplete인지 분명하다. 미실행을 성공으로 기록하지 않는다.
4. **성능:** 준비된 코어와 새 tweak/새 키 비용을 분리한 같은 호스트 실측이 있다.
5. **배포 상태:** 마지막 로컬 SHA, 원격 SHA, CI 상태를 확인한다. GitHub Release 공개는 별도 요청이 있을 때만 한다.

필수 기능 시험의 실패를 남겨 놓고 완료라고 하지 않는다. 반대로 모든 공격 기법의 안전성 증명이나 임의의 평균 속도를 무기한 기다리지는 않는다. 기능 구현 완료, 연구 실행 완료, 미해결 보안 위험은 병기할 수 있다. 구조적 결함이 나왔다면 연구 결과를 남기되 실제 기밀용 사용을 승인하지 않는다.

루트 라이선스는 소유자가 제공한 것이 있으면 보존한다. 없으면 배포 라이선스 선택이 남았음을 보고하고 임의로 타인 코드의 라이선스를 지우거나 승인되지 않은 저작권자를 기입하지 않는다. 의존성과 라이선스 고지는 README에 정리한다.

최종 보고는 `완료한 계획번호 / 마지막 SHA / 실행한 검사와 결과 / 원격 push·CI / 성능 다섯 모드 / 성공한 공격의 정확한 라운드 / 미해결·다음 작업` 순서로 간결하게 작성한다. 문서 작성만으로 구현을 완료했다고 보고하지 않는다.

## 12. 표준 의존성 참고

다음 자료는 SHAKE256의 정의와 외부 API 사용을 확인하기 위한 참고다. DualForm 자체의 암호학적 안전성을 입증하는 자료는 아니다.

- [NIST FIPS 202 — SHAKE256 정의](https://csrc.nist.gov/pubs/fips/202/final)
- [OpenSSL 3 — EVP digest 및 XOF API](https://docs.openssl.org/3.0/man3/EVP_DigestInit/)
