# 개발 진행표

각 행은 실제 구현 후 갱신합니다. SHA는 다음 커밋에서 기록하며 현재 행은 Git 이력에서 확인할 수 있습니다.

| 계획 | 제목 | 상태 | SHA | 마지막 검사 | push / CI |
| --- | --- | --- | --- | --- | --- |
| 1 | docs: define fresh repository scope and commit plan | 완료 | 678c7ddc62c653a0b6839e6da19e1461948b3a71 | 수식·KAT·34행 대조 | 완료 / workflow 도입 전 |
| 2 | feat: add portable cmake build and test harness | 완료 | 98b314d1910ce74e00176691856d486674825f7f | — | 완료 / workflow 도입 전 |
| 3 | feat: implement endian and rotation primitives | 완료 | e3de95d0d4f47fc4df9ea58de7692704ef1a6dca | — | 완료 / workflow 도입 전 |
| 4 | feat: add checked shake256 backend | 완료 | c967798804a708465c91751415c5b0c5075205f1 | — | 완료 / workflow 도입 전 |
| 5 | feat: implement constant-flow nibble substitution | 완료 | 439fb53ddfcc6e13e2f74904f2ca5bbb9dda49d9 | — | 완료 / workflow 도입 전 |
| 6 | feat: implement alternating round functions | 완료 | 732d32db5d1000012f67231c7069944cbe55c647 | Release CTest 5/5; boundary/carry/reference equality | 완료 / workflow 도입 전 |
| 7 | feat: implement cached base key context | 완료 | fe9561189c387b9789e8667db2d8d6f61130d0eb | CTest test_key passes after correcting domain length; 7/7 including pending tweak test | 완료 / workflow 도입 전 |
| 8 | feat: implement keyed tweak permutation | 완료 | f2187dc19bcf68640488f8a4ee78393b208a2137 | CTest 7/7; zero/sequential encoding KAT and inverse roundtrip | 완료 / workflow 도입 전 |
| 9 | feat: implement independent python reference | 완료 | 93d55dcb15ec11174fbea8c7ea450c0ea357039f | CTest 8/8; Python 3 KATs and all 75 prefix inverses | 완료 / workflow 도입 전 |
| 10 | feat: implement prepared tweak context | 완료 | 현재 커밋 (Git 이력) | CTest 9/9; SK0/SK1 KAT, D0/D1 excluded, parent-independent ownership | 게시 후 원격 확인 |
| 11 | feat: implement full-round block encryption | 대기 | — | — | — |
| 12 | feat: implement full-round block decryption | 대기 | — | — | — |
| 13 | test: add fixed vectors and round traces | 대기 | — | — | — |
| 14 | test: cross-check independent implementations | 대기 | — | — | — |
| 15 | test: enforce context ownership and failure contracts | 대기 | — | — | — |
| 16 | test: cover buffer boundaries and aliasing | 대기 | — | — | — |
| 17 | test: add sanitizer regression checks | 대기 | — | — | — |
| 18 | test: verify portable compiler and platform builds | 대기 | — | — | — |
| 19 | feat: add reproducible research runner | 대기 | — | — | — |
| 20 | test: verify round function inversion | 대기 | — | — | — |
| 21 | test: reproduce reduced-round subkey recovery | 대기 | — | — | — |
| 22 | test: reproduce public tweak cancellation control | 대기 | — | — | — |
| 23 | test: reproduce fixed four-round differential | 대기 | — | — | — |
| 24 | test: screen bounded related-tweak differences | 대기 | — | — | — |
| 25 | docs: record attack models and security limitations | 대기 | — | — | — |
| 26 | feat: add separated setup and block benchmarks | 대기 | — | — | — |
| 27 | test: validate benchmark timing and workload accounting | 대기 | — | — | — |
| 28 | feat: add labeled openssl comparison benchmarks | 대기 | — | — | — |
| 29 | feat: reuse shake contexts within tweak preparation | 대기 | — | — | — |
| 30 | test: audit secret handling and optimized equivalence | 대기 | — | — | — |
| 31 | docs: publish measured performance and research results | 대기 | — | — | — |
| 32 | feat: add library installation and consumer smoke tests | 대기 | — | — | — |
| 33 | test: verify clean checkout and complete validation targets | 대기 | — | — | — |
| 34 | docs: finalize research baseline and handoff status | 대기 | — | — | — |
