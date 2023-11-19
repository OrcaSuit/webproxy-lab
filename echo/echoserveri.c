#include "csapp.h"

/*함수 선언*/
//클라이언트와의 연결을 처리하는 함수, 클라이언트로부터 데이터를 읽고, 그 데이터를 다시 클라이언트에게 보냄.
void echo(int connfd);

int main(int argc, char **argv)
{

    int listenfd, connfd; //각각 서버의 리스닝 소켓, 클라이언트와의 연결 소켓 
    
    /*클라이언트 주소 정보를 저장하고 처리하는데 사용*/
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */
    char client_hostname[MAXLINE], client_port[MAXLINE];

    /*포트 번호 검증*/
    //서버는 시작할 때 명령줄 인자로 포트 번호를 받아야함, 이를 확인하는 코드.
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    /*리스닝 소켓 개설*/
    listenfd = Open_listenfd(argv[1]); //주어진 포트 번호에 대한 리스닝 소켓을 열고 `listenfd`에 할당
    /*연결 수락 루프*/
    while (1) { //무한 루프를 돌면서 새로운 연결을 기다리고 수락함.
        clientlen = sizeof(struct sockaddr_storage); 
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //클라이언트의 연결 요청을 수락하고, 클라이언트의 소켓 주소 정보를 `clienttaddr`에 저장 
        
        /*클라이언트 정보 획득 및 출력*/
        Getnameinfo((SA*) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0); //함수를 사용하여 클라이언트의 주소 정보를 문자열로 변환
        printf("Connected to (%s, %s)\n", client_hostname, client_port); //클라이언트의 호스트 이름과 포트 번호를 출력
        
        /* 에코 기능 수행 */
        echo(connfd); 

        /*연결 종료*/
        Close(connfd);
    }
    exit(0);
}