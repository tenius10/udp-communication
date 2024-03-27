#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define MAXBUF 1024

/**
 * argc : 명령줄 인자 개수
 * argv[0] : 프로그램의 이름 ex) ./receiver
 * argv[1] : 통신 상대의 IP 주소 ex) 127.0.0.1
 * argv[2] : 파일을 저장할 디렉터리 경로 ex) ./download
 * 
 * <Program 실행 가이드>
 * : <Program> <IP> <SAVE_DIR>
 * 
 * ex) ./receiver 127.0.0.1 ./download
*/

void main(int argc, char *argv[]) {
    
    printf("# Receiver > Start\n");

    if (argc != 3) {
        fprintf(stderr, "명령어 형식을 다음과 같이 맞춰주세요.\n%s <IP> <SAVE_DIR>\ex) ./receiver 127.0.0.1 ./download\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 프로그램 인자 설정
    char* IP_ADDR = argv[1];
    char* SAVE_DIR_PATH = argv[2];

    const int PORT_RECV = 51000;
    const int PORT_SEND = 51001;

    // 1. 수신 소켓 설정

    // 1-1) 소켓 생성
    int sockfd_recv;  // Socket의 파일 디스크립터
    struct sockaddr_in dest_recv;  // 송신자의 IP 주소와 Port 번호
    char buffer[MAXBUF];  // send(), recv()에서 사용할 버퍼
    if ((sockfd_recv = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }

    // 1-2) 소켓 설정
    memset(&dest_recv, 0, sizeof(dest_recv));  //초기화
    dest_recv.sin_family = AF_INET;  // 주소 체계
    dest_recv.sin_port = htons(PORT_RECV);  // 포트 번호
    // IP 주소
    if (inet_aton(IP_ADDR, &dest_recv.sin_addr) == 0) {
        perror("Socket IP");
        exit(errno);
    }

    // 1-3) 소켓 묶기 (bind)
    if (bind(sockfd_recv, (struct sockaddr *)&dest_recv, sizeof(dest_recv)) != 0) {
        perror("Bind");
        exit(errno);
    }


    // 2. 송신 소켓 설정

    // 2-1) 소켓 생성
    int sockfd_send;  // Socket의 파일 디스크립터
    struct sockaddr_in dest_send;  // 송신자의 IP 주소와 Port 번호
    if ((sockfd_send = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket");
        exit(errno);
    }

    // 2-2) 소켓 설정
    memset(&dest_send, 0, sizeof(dest_send));
    dest_send.sin_family = AF_INET;
    dest_send.sin_port = htons(PORT_SEND);
    if (inet_aton(IP_ADDR, &dest_send.sin_addr) == 0) {
        perror("Socket IP");
        exit(errno);
    }

    // 2-3) 소켓 연결
    if (connect(sockfd_send, (struct sockaddr *)&dest_send, sizeof(dest_send)) != 0) {
        perror("Connect");
        exit(errno);
    }

    printf("# Receiver > create Socket successly\n");


    // 3. 패킷 송수신

    // 3-1) Greeting 메세지 수신
    memset(buffer, 0, MAXBUF);
    recv(sockfd_recv, buffer, sizeof(buffer), 0);
    printf("# Receiver > receive message from sender : %s\n", buffer);

    // 만약 Greeting 메세지를 못 받았으면 에러 출력
    if(strcmp(buffer, "Greeting")){    
        fprintf(stderr, "Failed to receive Greeting message\n");
        exit(EXIT_FAILURE);
    }

    // 3-2) Filename 수신
    memset(buffer, 0, MAXBUF);
    recv(sockfd_recv, buffer, sizeof(buffer), 0);
    printf("# Receiver > receive message from sender : %s\n", buffer);
    
    // 만약 Filename을 못 받았으면 에러 출력
    if(!buffer){
        fprintf(stderr, "Failed to receive Filename\n");
        exit(EXIT_FAILURE);
    }

    // 파일 path 설정    
    char filepath[MAXBUF]="";
    strcat(filepath, SAVE_DIR_PATH);

    // 저장할 디렉터리 없으면 생성
    struct stat st = {0};
    if (stat(filepath, &st) == -1) {
        mkdir(filepath, 0700);
    }

    if(SAVE_DIR_PATH[strlen(SAVE_DIR_PATH)-1]!='/'){
        char separator[]="/";
        strcat(filepath, separator);
    }
    strcat(filepath, buffer);

    // 3-3) 송신자에게 OK 메세지 전송
    strcpy(buffer, "OK");
    send(sockfd_send, buffer, strlen(buffer), 0);
    printf("# Receiver > send message to sender : %s\n", buffer);


    // 3-4) 대용량 File을 packet 단위로 수신해서 파일에 저장

    // 파일 open
    FILE *file = fopen(filepath, "wb");

    if(!file){
        perror("File Open");
        exit(errno);
    }
    
    // 패킷 수신 후 저장
    while (1) {
        memset(buffer, 0, MAXBUF);
        ssize_t count = recv(sockfd_recv, buffer, sizeof(buffer), 0);
        if (count <= 0) {
            break;
        }
        
        // 3-5) Finish 메세지를 수신하면 파일 작성 종료
        if(!strcmp(buffer, "Finish")){
            printf("# Receiver > send message to sender : %s\n", buffer);

            // 3-6) WellDone 메세지 전송
            strcpy(buffer, "WellDone");
            send(sockfd_send, buffer, strlen(buffer), 0);
            printf("# Receiver > send message to sender : %s\n", buffer);
            
            break;
        }
        
        printf("# Receiver > receive File Packet\n");
        fwrite(buffer, sizeof(char), count, file);
    }

    // 파일 close
    fclose(file);
    
    // 4. 소켓 close
    close(sockfd_recv);
    close(sockfd_send);
    
    exit(0);
}
