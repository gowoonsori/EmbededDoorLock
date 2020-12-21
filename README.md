# EmbededDoorLock

라즈베리파이와 FND, BUZZER, LED, PUSH_SWITCH, STEP_MOTOR를 이용한 잠금장치 경보 시스템

## 사용방법

```
bash deviceDriver.sh
```

쉘 파일을 통해 deivceDriver를 등록해준다.

```
chmod 777 doorlock
sudo ./doorlock 1234
```

doorlock 실행파일을 chmod로 실행권한을 부여하고 sudo권한으로 실행

이때, argument로 초기 비밀번호 숫자 4자리를 입력해주어야한다.

실행 중에는 `ctrl+c`로 프로그램 종료가 가능하며, 사이렌 경보 해제는 `1`을 입력하여 해제가 가능하다.
