# 구현 검증 기록

Linux x86-64, GCC 14.2.0, OpenSSL 3.5.7에서 실행합니다. Release 테스트에서도 assert를 활성화합니다. 테스트 전용 라이브러리에만 실패 주입 심볼을 넣습니다.

ASan+UBSan은 Debug 빌드 전체 CTest를 실행하며 매번 새 컨텍스트를 만드는 1,000회 수명 시험을 포함합니다. Linux sanitizer 실행 파일은 PIE 주소 공간 충돌을 줄이도록 비 PIE로 빌드합니다. 지원하지 않는 운영체제/컴파일러 조합에서 옵션을 켜면 configure 오류를 냅니다. OpenSSL 의존성 자체는 이 빌드로 재계측하지 않습니다.

실제 로컬 실행: 이 환경의 프로세스 추적 제한 때문에 LeakSanitizer가 시작 시 `does not work under ptrace`로 종료됩니다. 따라서 로컬 재실행에서만 `ASAN_OPTIONS=detect_leaks=0`을 지정하고 ASan 주소 검사와 UBSan을 유지합니다. 누출 검사 전체 성공으로 기록하지 않습니다. 프로젝트 소유 메모리 누출은 실패 주입의 live allocation 계수로 별도 확인하며, 원격 Linux CI는 LeakSanitizer를 비활성화하지 않습니다.

CI는 Linux GCC·Clang, macOS AppleClang, Windows MSVC x64와 별도의 Linux ASan+UBSan 작업을 실행합니다. 원격 실제 결과는 Actions run에서 확인하며 workflow를 작성했다는 이유로 해당 플랫폼을 검증 완료로 표시하지 않습니다.

## SHAKE 컨텍스트 재사용 실험의 채택 판정

계획 29에서 한 tweak 준비 안의 7개 SHAKE 메시지마다 재초기화하며 EVP 컨텍스트를 재사용했습니다. 전후 KAT·25,600 prefix·전체 연구 counts는 같습니다. 같은 호스트의 5회 표본에서 tweak 준비는 약 3.0% 느려지고, tweak+블록은 약 8.5% 빨라지며, 키+tweak+블록은 약 9.4% 느려졌습니다. 전체적인 유의미한 이득을 입증하지 못했으므로 최종 라이브러리는 단순한 메시지별 컨텍스트 소유 방식으로 복원했습니다. 기본 키의 B24 캐시는 유지합니다. 실험 결과와 소스 해시는 별도 JSON에 남겨 최종 코어의 성능으로 섞지 않습니다.

## 비밀 접근과 기계어 점검

Linux x86-64 GCC 14.2.0 Release `-O3 -DNDEBUG`(LTO 없음)의 `df_substitute`, `df_even`, `df_odd`를 objdump로 점검했습니다. 세 함수에서 조건 점프와 비밀 인덱스 조회를 관측하지 않았으며 고정 레지스터 연산과 stack 저장/복원만 확인했습니다. 컨텍스트 해제 객체의 `OPENSSL_cleanse` relocation도 남아 있습니다. [machine_code_audit.json](machine_code_audit.json)에 소스·객체 해시와 검사 범위를 기록합니다. 다른 CPU·컴파일러의 기계어, OpenSSL 내부, 컴파일러 복사본·레지스터·swap의 완전 소거까지 보증하지 않습니다.

최종 코어 복원 후 ASan+UBSan 18/18, 독립 대조 25,600 prefix, 전체 연구 counts 재대조를 통과했습니다. 로컬 LeakSanitizer 환경 제한은 앞 절과 같습니다.

재사용 실험 커밋의 Windows CI에서 `test_primitives`가 OpenSSL include 경로를 받지 않은 상태로 내부 헤더를 통해 `openssl/evp.h`를 요구하는 의존성 오류가 발견되었습니다. 최종 코어 복원으로 정수 연산 내부 헤더에서 EVP 의존성도 제거했습니다. Linux 암호 시험의 실패가 아니라 MSVC 빌드 의존성 결함이며, 후속 원격 빌드 결과를 별도 확인합니다.
