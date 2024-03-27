## ◼️ 도입
데이터 통신 수업 과제용 레포지토리<br/><br/>
수업 내용에 따라 조금씩 버전업할 예정입니다.<br/>
아직 이론 수업을 듣지 못 한 상태에서 제출하려고 만든 코드라 부족한 점이 많습니다.
<br/>
<br/>
<br/>

## ✅ v1 (Term Project #1)
UDP로 파일을 전송하는 프로그램 작성 (sender.c, receiver.c)
<br/>
<br/>

### ◾ 조건
- Linux 환경에서 실행
- c/c++ 사용
- 서로 다른 machine에서 동작할 필요없음. localhost에서만 통신 가능하면 OK
<br/>

UDP는 reliability가 낮기 때문에 파일 전송처럼 패킷 손실에 민감한 서비스에서는 사용하지 않는 것이 좋다.<br/>
현재 과제는 로컬 호스트 간의 통신이기 때문에 패킷이 손실될 일이 없기도 하고, UDP가 TCP보다 구현하기 쉬워서 이런 과제를 내주셨다고 한다.<br/>
수업 시간에 전송은 UDP처럼 하고 손실된 패킷에 대해서 TCP처럼 복구하는 방법을 많이 사용한다고 설명해주셨는데, <br/>
아마 이번 과제가 끝나고 다음 과제에서 버전업 시키도록 하시지 않을까 싶다.<br/>
<br/>

### ◾ 실행 화면
![image](https://github.com/tenius10/udp-communication/assets/108507183/03a8bac2-5d44-44b7-85f8-92f8829be2aa)

<br/>

### ◾ 실행 가이드

./receiver [통신 상대의 IP] [저장할 디렉터리 경로] & <br/>
./sender [통신 상대의 IP] [전송할 파일명] & <br/>

송신자와 수신자는 모두 백그라운드 실행<br/>
<br/>

```
cd ..
git clone https://github.com/tenius10/udp-communication.git file_transfer
cd file_transfer
gcc -o receiver receiver.c
gcc -o sender sender.c
./receiver 127.0.0.1 ./download &
./sender 127.0.0.1 file.txt &
```

