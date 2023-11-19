#include "csapp.h"

void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio; // robust I/O 구조체인 `rio_t` 타입의 변수 `rio`를 선언, 이 구조체는 효율적인 버퍼링 입력을 위해 사용됨.

    /*`rio` 구조체를 초기화하고 클라이언트와의 연결 파일 디스크럽터인 `connfd`와 연결, 데이터를 읽기 전에 한 번만 호출되어야함.*/
    Rio_readinitb(&rio, connfd); 
    
    /* 반복문 안에서, `Rio_readlineb` 함수를 사용해 클라이언트로부터 한 줄의 데이터를 읽어 `buf` 배열에 저장함. 
    `MAXLINE`은 `buf`의 최대 크기를 정의함. 데이터가 더 이상 없을 때까지, 즉 `n`이 0이 될 때까지 이 과정을 반복. */
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { 
    printf("server received %d bytes\n", (int)n); //서버가 클라이언트로 부터 받은 파일의 바이트를 표시.
    Rio_writen(connfd, buf, n); //서버가 buf에 저장된 `n` 바이트의 데이터를 클라이언트에게 다시 보냄.
}

}

