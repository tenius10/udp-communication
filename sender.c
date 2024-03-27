#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define MAXBUF 1024

/**
 * argc : 명령줄 인자 개수
 * argv[0] : 프로그램의 이름 ex) ./sender
 * argv[1] : 통신 상대의 IP 주소 ex) 127.0.0.1
 * argv[2] : 전송할 파일 경로 ex) file.txt
 * 
 * <Program 실행 가이드>
 * : <Program> <IP> <FILE>
 * 
 * ex) ./sender 127.0.0.1 file.txt
*/

void main(int argc, char *argv[]) {
    
    printf("# Sender > Start\n");

    // 명령어를 제대로 호출했는지 확인
    if (argc != 3) {
        fprintf(stderr, "명령어 형식을 다음과 같이 맞춰주세요.\n%s <IP> <FILE>\nex) ./sender 127.0.0.1 file.txt\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // 프로그램 인자 설정
    char* IP_ADDR = argv[1];
    char* FILE_NAME = argv[2];

    const int PORT_SEND = 51000;
    const int PORT_RECV = 51001;

    // 1. 송신 소켓 설정
    
    // 1-1) 소켓 생성
    int sockfd_send;  // Socket의 파일 디스크립터
    struct sockaddr_in dest_send;  // 수신자의 IP 주소와 Port 번호
    char buffer[MAXBUF];  // send(), recv()에서 사용할 버퍼
    // socket(주소 체계, 소켓 타입, 프로토콜)
    if ((sockfd_send = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }

    // 1-2) 소켓 설정
    memset(&dest_send, 0, sizeof(dest_send));  //초기화
    dest_send.sin_family = AF_INET;  // 주소 체계 (AF_INET : IPv4 인터넷 프로토콜)
    // htons() : host byte order에서 network byte order로 변환
    dest_send.sin_port = htons(PORT_SEND);  //포트 번호
    // IP 주소
    // inet_aton(IP 주소, 설정할 위치)
    if (inet_aton(IP_ADDR, &dest_send.sin_addr) == 0) {
        perror("Socket IP");
        exit(errno);
    }

    // 1-3) 소켓 연결
    if (connect(sockfd_send, (struct sockaddr *)&dest_send, sizeof(dest_send)) != 0) {
        perror("Connect");
        exit(errno);
    }



    // 2. 수신 소켓 설정

    // 2-1) 소켓 생성
    int sockfd_recv;  // Socket의 파일 디스크립터
    struct sockaddr_in dest_recv;  // 수신자의 IP 주소와 Port 번호
    if ((sockfd_recv = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }

    // 2-2) 소켓 설정
    memset(&dest_recv, 0, sizeof(dest_recv));
    dest_recv.sin_family = AF_INET;
    dest_recv.sin_port = htons(PORT_RECV);
    if (inet_aton(IP_ADDR, &dest_recv.sin_addr) == 0) {
        perror("Socket IP");
        exit(errno);
    }

    // 2-3) 소켓 묶기 (bind)
    // bind(소켓 파일 디스크립터, 주소 정보, 주소 크기)
    if (bind(sockfd_recv, (struct sockaddr *)&dest_recv, sizeof(dest_recv)) != 0) {
        perror("Bind");
        exit(errno);
    }


    printf("# Sender > create Socket successly\n");


    // 3. 패킷 송수신

    // 3-1) Greeting 메세지 전송
    strcpy(buffer, "Greeting");
    send(sockfd_send, buffer, strlen(buffer), 0);
    printf("# Sender > send message to receiver : %s\n", buffer);

    // 3-2) Filename 전송
    send(sockfd_send, FILE_NAME, strlen(FILE_NAME), 0);
    printf("# Sender > send message to receiver : %s\n", FILE_NAME);

    // 3-3) OK 메세지 수신
    memset(buffer, 0, MAXBUF);
    recv(sockfd_recv, buffer, sizeof(buffer), 0);
    printf("# Sender > receive message from receiver : %s\n", buffer);

    // 만약 OK 메세지를 못 받았으면 에러 출력
    if(strcmp(buffer, "OK")){    
        fprintf(stderr, "Failed to receive OK message\n");
        exit(EXIT_FAILURE);
    }

    // 3-4) 대용량 File을 packet 단위로 잘라서 전송
    
    // 파일 open
    FILE *file = fopen(FILE_NAME, "rb");
    if (!file) {
        perror("File Open");
        exit(errno);
    }

    while (!feof(file)) {
        size_t count = fread(buffer, 1, MAXBUF, file);
        if (count > 0) {
            send(sockfd_send, buffer, count, 0);
            printf("# Sender > send File Packet\n");
        }
    }
    

    // 파일 닫기
    fclose(file);


    // 3-5) Finish 메세지 전송
    strcpy(buffer, "Finish");
    send(sockfd_send, buffer, strlen(buffer), 0);
    printf("# Sender > send message to receiver : %s\n", buffer);


    // 3-6) 수신자에게 WellDone 메세지 수신
    memset(buffer, 0, MAXBUF);
    recv(sockfd_recv, buffer, sizeof(buffer), 0);
    printf("# Sender > receive message from receiver : %s\n", buffer);


    // 4. 소켓 close
    close(sockfd_send);
    close(sockfd_recv);
    
    exit(0);
}