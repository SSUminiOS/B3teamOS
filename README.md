# miniOS

miniOS-project/
├── README.md               # 프로젝트 설명 및 사용 방법 문서
├── Makefile                # 전체 프로젝트 빌드 자동화를 위한 메이크파일
├── boot/                   # 부트로더 소스 코드
│   └── boot.asm            # 부트로더 어셈블리 코드
├── kernel/                 # 커널 소스 코드
│   ├── kernel.c            # 커널 메인 C 소스 파일
│   └── ...
├── drivers/                # 디바이스 드라이버 코드
│   ├── keyboard.c          # 키보드 드라이버
│   ├── screen.c            # 화면(비디오) 드라이버
│   └── ...
├── lib/                    # 커널 라이브러리 및 공통 유틸리티
│   ├── stdio.c             # 기본 입출력 함수
│   ├── string.c            # 문자열 처리 함수
│   └── ...
├── include/                # 헤더 파일
│   ├── kernel.h            # 커널 관련 공통 헤더
│   ├── drivers/            # 드라이버 헤더 파일
│   └── lib/                # 라이브러리 헤더 파일
└── scripts/                # 빌드 및 유틸리티 스크립트
    ├── build.sh            # 빌드 스크립트
    └── run_qemu.sh         # QEMU를 통해 OS 이미지 실행 스크립트


