#include "csapp.h"

/*함수 선언*/
// 클라이언트와의 연결을 처리하는 함수, 클라이언트로부터 데이터를 읽고, 그 데이터를 다시 클라이언트에게 보냄.
void echo(int connfd);

int main(int argc, char **argv)
{

    int listenfd, connfd; // 각각 서버의 리스닝 소켓, 클라이언트와의 연결 소켓
    /*클라이언트 주소 정보를 저장하고 처리하는데 사용*/
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; /* Enough space for any address */
    char client_hostname[MAXLINE], client_port[MAXLINE];

    /*포트 번호 검증*/
    // 서버는 시작할 때 명령줄 인자로 포트 번호를 받아야함, 이를 확인하는 코드.
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    /*리스닝 소켓 개설*/
    listenfd = Open_listenfd(argv[1]); // 주어진 포트 번호에 대한 리스닝 소켓을 열고 `listenfd`에 할당
    /*연결 수락 루프*/
    while (1)
    { // 무한 루프를 돌면서 새로운 연결을 기다리고 수락함.
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 클라이언트의 연결 요청을 수락하고, 클라이언트의 소켓 주소 정보를 `clienttaddr`에 저장

        /*클라이언트 정보 획득 및 출력*/
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0); // 함수를 사용하여 클라이언트의 주소 정보를 문자열로 변환
        printf("Connected to (%s, %s)\n", client_hostname, client_port);                              // 클라이언트의 호스트 이름과 포트 번호를 출력

        /* 에코 기능 수행 */
        echo(connfd);

        /*연결 종료*/
        Close(connfd);
    }
    exit(0);
}
    void echo(int connfd)
    {
        size_t n;
        char buf[MAXLINE];
        rio_t rio; // robust I/O 구조체인 `rio_t` 타입의 변수 `rio`를 선언, 이 구조체는 효율적인 버퍼링 입력을 위해 사용됨.

        /*`rio` 구조체를 초기화하고 클라이언트와의 연결 파일 디스크럽터인 `connfd`와 연결, 데이터를 읽기 전에 한 번만 호출되어야함.*/
        Rio_readinitb(&rio, connfd);

        /* 반복문 안에서, `Rio_readlineb` 함수를 사용해 클라이언트로부터 한 줄의 데이터를 읽어 `buf` 배열에 저장함.
        `MAXLINE`은 `buf`의 최대 크기를 정의함. 데이터가 더 이상 없을 때까지, 즉 `n`이 0이 될 때까지 이 과정을 반복. */
        while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
        {
            printf("server received %d bytes\n", (int)n); // 서버가 클라이언트로 부터 받은 파일의 바이트를 표시.
            Rio_writen(connfd, buf, n);                   // 서버가 buf에 저장된 `n` 바이트의 데이터를 클라이언트에게 다시 보냄.
        }
    }

/* 에코 서버의 핵심적인 부분으로 클라이언트로부터 데이터를 받고 그 데이터를 로깅한 후, 동일한 데이터를 클라이언트에게 되돌려 보내는 작업을 함
Rio_ 접두사가 붙은 함수들은 표준 입출력 함수들보다 더 안전하게 에러를 처리하고 더 robust한 동작을 보장하는 래퍼함수. */
