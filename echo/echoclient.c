/*
클라이언트 프로그램의 구현
이 프로그램은 사용자로부터 입력받은 데이터를 서버에 보내고, 서버로부터의 응답을 받아 출력하는 기능을 수행한다.
*/

#include "csapp.h"

int main(int argc , char **argv)
{
    int clientfd; //서버와의 연결을 위한 소켓 파일 디스크립터
    char *host, *port, buf[MAXLINE]; //연결할 서버의 호스트 이름과 포트 번호를 저장할 포인터와 데이터를 저장할 버퍼
    rio_t rio; // robust I/O 구조체

    /*입력 인자 검증,*/
    //프로그램은 반드시 호스트 이름과 포트 번호를 입력 인자로 받아야함. 그렇지 않을 경우 사용 방법을 출력하고 종료. 
    if (argc != 3) { 
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = argv[2];

    /*서버 연결 설정*/
    //지정된 호스트와 포트에 대한 클라이언트 소켓을 연결, 이 함수는 내부적으로 소켓을 생성하고 서버에 연결함.
    clientfd = Open_clientfd(host, port); 
    Rio_readinitb(&rio, clientfd); // 입력/출력 버퍼를 초기화함.

    /*사용자 입력 처리 및 서버와의 통신*/
    //사용자로부터 표준 입력을 받아 buf에 저장.
    while (Fgets(buf, MAXLINE, stdin) != NULL){
        Rio_writen(clientfd, buf, strlen(buf)); //사용자로부터 입력받은 데이터를 서버로 보냄.
        Rio_readlineb(&rio, buf, MAXLINE); //서버로부터의 응답을 받음
        Fputs(buf, stdout); //서버로부터 받은 응답을 표준 출력에 출력.
    }
    Close(clientfd); // 소켓 연결을 종료
    exit(0); //프로그램을 정상저긍로 종료
}