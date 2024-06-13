# MiniOS_RR

MiniOS는 간단한 운영체제 시뮬레이션 환경으로, 다양한 핵심 기능들을 구현하여
프로세스 관리, 스케줄링, 사용자 입력 처리, 그리고 시스템 상태 시각화를 제공
합니다. 이 프로젝트는 운영체제의 기본 개념을 이해하고, 실제 커널의 기능을 모
방하여 다양한 기능을 구현하는 것을 목표로 합니다.

## 주요 기능

- **사용자 입력 처리**
  - 명령어 입력: `OS_Input` 컴포넌트는 사용자가 입력한 명령어를 받아들입니다.
  `readline` 라이브러리를 사용하여 명령어를 입력받고, 유효성을 검증한 후 이를
  처리합니다.
  - 명령어 전달: 입력된 명령어는 공유 메모리 영역인 "INPUT"을 통해 `OS_Core`
  컴포넌트로 전달됩니다. 세마포어를 사용하여 데이터 접근을 동기화하고, 입력이
  올바르게 전달되도록 보장합니다.

- **프로세스 생성 및 관리**
  - 프로세스 생성: `OS_Core` 컴포넌트는 `spawn_proc` 함수를 사용하여 새로운 프로
  세스를 생성합니다. 이 과정에서 `fork` 시스템 호출을 사용하여 부모와 자식 프로
  세스를 생성하고, 프로세스 그룹을 설정하여 독립적인 작업 단위로 동작하게 합
  니다.
  - 프로세스 등록: 생성된 프로세스는 `proc_register` 함수를 통해 프로세스 테이블
  에 등록됩니다. 이 테이블은 모든 프로세스의 상태와 정보를 관리하며, 실행 대기
  큐와 완료된 작업 큐를 통해 프로세스를 관리합니다.
  - 프로세스 관리: `OS_Core`는 프로세스 상태를 모니터링하고, 필요한 경우 프로세
  스를 중단하거나 재개합니다. 이를 통해 효율적인 자원 사용과 프로세스 관리가 가
  능합니다.

- **라운드 로빈 스케줄링**
  - 스케줄링 알고리즘: MiniOS는 라운드 로빈 스케줄링 알고리즘을 사용하여 모든
  프로세스에 공평한 실행 시간을 할당합니다. 각 프로세스는 일정 시간 동안 실행된
  후, 다음 프로세스로 전환됩니다.
  - 타이머 기반 스케줄링: `init_timer` 함수는 일정한 시간 간격마다 시그널을 발
  생시키는 타이머를 설정합니다. 타이머 핸들러인 `loop_handler` 함수는 주기적으
  로 호출되어 스케줄링 작업을 수행합니다.
  - 프로세스 전환: `sched_handler` 함수는 현재 실행 중인 프로세스를 중단하고, 다
  음 프로세스를 실행합니다. `sched_next` 함수는 선택된 프로세스를 실행하고, 다
  른 모든 프로세스를 일시 중지시킵니다.

- **시스템 상태 시각화**
  - 상태 모니터링: `OS_Stat` 컴포넌트는 주기적으로 CPU와 메모리 사용량을 계산하
  고, 이를 시각적으로 표시합니다. 이를 통해 사용자는 시스템의 상태를 실시간으로
  모니터링할 수 있습니다.
  - 프로세스 정보 출력: `stat_hdlr` 함수는 프로세스 테이블의 정보를 읽어와 각 프
  로세스의 상태를 출력합니다. 프로세스의 실행 시간, 대기 시간, 메모리 사용량 등
  을 확인할 수 있습니다.

- **터미널 연결 관리 (Reptyr)**
  - 터미널 전환: Reptyr 도구를 사용하여 특정 프로세스를 다른 터미널로 이동시킬
  수 있습니다. 이를 통해 사용자는 터미널 세션을 전환하여도 작업을 중단 없이 계
  속할 수 있습니다.
  - Tmux 통합: MiniOS는 Tmux와 통합되어 있으며, 각 프로세스는 Tmux 창과 연결되
  어 있어 사용자는 여러 작업을 동시에 모니터링하고 관리할 수 있습니다.

- **링크드 리스트를 통한 큐 관리**
  - 실행 대기 큐 및 완료된 작업 큐: 실행 대기 큐와 완료된 작업 큐는 링크드 리스트
  를 사용하여 구현되었습니다. `list_head` 구조체를 사용하여 리스트 내의 항목을 
  연결하고, 효율적으로 프로세스를 관리합니다.
  - 리스트 연산: 리스트 항목을 추가, 삭제, 순회하는 다양한 함수와 매크로가 제공
  되어 리스트 관리가 용이합니다.

## 설치 및 실행

1. **프로젝트 클론**
   ```sh
   git clone https://github.com/username/MiniOS.git
   cd MiniOS
   ```

2. **빌드**
    ```sh
    make
    ```

3. **실행**
    ```sh
    # 실행 오류날 경우, sudo su 환경에서 다음 명령 수행.
    echo 0 > /proc/sys/kernel/yama/ptrace_scope 
    ./minios
    ```


