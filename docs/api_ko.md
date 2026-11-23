# C API 사용과 계약

[dualform.h](../include/dualform.h)가 설치하는 유일한 헤더입니다. 공개 API는 항상 24라운드이며 연구용 라운드 수나 trace API를 노출하지 않습니다. 외부 소비자 전체 예제는 [examples/consumer](../examples/consumer/main.c)에 있습니다.

키 32바이트, tweak 16바이트, 블록 16바이트를 정확히 전달합니다. 출력 capacity는 16 이상이며 성공 시 앞 16바이트만 변경합니다. 키·tweak 생성 출력 슬롯은 별도 살아 있는 포인터 객체이며 처음 NULL이어야 합니다. 성공 시에만 컨텍스트가 대입됩니다. non-NULL 슬롯은 EINVAL로 거부하고 기존 값을 유지합니다.

키 컨텍스트는 입력 키와 B24를 복사 소유합니다. 준비된 tweak 컨텍스트는 SK24를 직접 소유하므로 부모를 해제한 뒤에도 블록 처리가 됩니다. 새 tweak 준비에는 살아 있는 부모가 필요합니다. 컨텍스트는 불변이어서 서로 다른 입출력 객체에 대한 동시 읽기 연산이 가능하지만 사용 중 해제하거나 같은 출력 버퍼를 동시에 쓰는 것은 호출자 오류입니다.

| 조건 | 반환 |
| --- | --- |
| 필요한 NULL 또는 이미 채워진 생성 슬롯 | EINVAL |
| 키·tweak·입력 길이 불일치 | ELENGTH |
| 출력 capacity < 16 | ECAPACITY |
| 서로 다른 시작 주소의 처리 영역 부분 중첩 | EOVERLAP |
| 컨텍스트 할당 실패 | ENOMEM |
| OpenSSL SHAKE 처리 실패 | ECRYPTO |

정확한 in-place는 지원합니다. 부분 중첩은 거부하며 오류 시 출력 및 입력을 보존합니다. 여러 오류의 반환 우선순위는 보장하지 않습니다. 범위 검사는 실제 읽고 쓰는 16바이트에만 적용하고 uintptr_t 끝 주소 overflow를 먼저 검사합니다. 전달 길이가 실제 메모리 할당 크기를 보증하지는 않습니다. 컨텍스트 내부를 입출력으로 전달하지 않습니다.

`free(NULL)`은 허용하며 해제 전 소유 키·기본키·서브키를 OPENSSL_cleanse로 소거합니다. 해제된/가짜 컨텍스트, 이중 해제, 실제로 할당되지 않은 버퍼를 안전하게 감지할 수 있다고 약속하지 않습니다. 레지스터·컴파일러 복사본·swap의 완전 소거도 보증하지 않습니다.

공개 tweak는 nonce가 아닙니다. 같은 key/tweak/plaintext는 같은 ciphertext를 만듭니다. 인증·태그·AD·패딩·파일 암호화·운용모드는 제공하지 않습니다. [보안 한계](security_ko.md)를 함께 읽어야 합니다.

## 설치와 외부 링크

```sh
cmake -S . -B build-lib -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build-lib --config Release
cmake --install build-lib --prefix /your/prefix --config Release
cmake -S examples/consumer -B build-consumer -DCMAKE_PREFIX_PATH=/your/prefix
cmake --build build-consumer --config Release
ctest --test-dir build-consumer -C Release --output-on-failure
```

외부 CMake 프로젝트에서 `find_package(DualForm CONFIG REQUIRED)`와 `target_link_libraries(your_target PRIVATE DualForm::dualform)`를 사용합니다. OpenSSL 3.x는 외부 의존성이며 패키지가 탐색합니다. 라이브러리 전용 빌드에는 Python이 필요하지 않습니다. 연구·테스트 헤더와 실행 파일은 설치하지 않습니다.
