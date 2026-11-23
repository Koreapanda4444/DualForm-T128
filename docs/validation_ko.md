# 구현 검증 기록

Linux x86-64, GCC 14.2.0, OpenSSL 3.5.7에서 실행합니다. Release 테스트에서도 assert를 활성화합니다. 테스트 전용 라이브러리에만 실패 주입 심볼을 넣습니다.

ASan+UBSan은 Debug 빌드 전체 CTest를 실행하며 매번 새 컨텍스트를 만드는 1,000회 수명 시험을 포함합니다. Linux sanitizer 실행 파일은 PIE 주소 공간 충돌을 줄이도록 비 PIE로 빌드합니다. 지원하지 않는 운영체제/컴파일러 조합에서 옵션을 켜면 configure 오류를 냅니다. OpenSSL 의존성 자체는 이 빌드로 재계측하지 않습니다.

실제 로컬 실행: 이 환경의 프로세스 추적 제한 때문에 LeakSanitizer가 시작 시 `does not work under ptrace`로 종료됩니다. 따라서 로컬 재실행에서만 `ASAN_OPTIONS=detect_leaks=0`을 지정하고 ASan 주소 검사와 UBSan을 유지합니다. 누출 검사 전체 성공으로 기록하지 않습니다. 프로젝트 소유 메모리 누출은 실패 주입의 live allocation 계수로 별도 확인하며, 원격 Linux CI는 LeakSanitizer를 비활성화하지 않습니다.

CI는 Linux GCC·Clang, macOS AppleClang, Windows MSVC x64와 별도의 Linux ASan+UBSan 작업을 실행합니다. 원격 실제 결과는 Actions run에서 확인하며 workflow를 작성했다는 이유로 해당 플랫폼을 검증 완료로 표시하지 않습니다.
