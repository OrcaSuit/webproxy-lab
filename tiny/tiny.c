/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h" // csapp 라이브러리 포함. 네트워크 프로그래밍 관련 함수 및 정의 포함

// 함수 선언
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv)
{
    int listenfd, connfd;                  // 리스닝 및 커넥션 파일 디스크립터
    char hostname[MAXLINE], port[MAXLINE]; // 클라이언트 호스트명 및 포트 저장 변수
    socklen_t clientlen;                   // 클라이언트 주소의 길이
    struct sockaddr_storage clientaddr;    // 클라이언트 주소 정보 저장 구조체

    /* 커맨드 라인 인자 검사 */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1); // 포트 번호가 제공되지 않으면 오류 메시지 출력 후 종료
    }

    listenfd = Open_listenfd(argv[1]); // 지정된 포트에서 리스닝 소켓 열기
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);                       // 클라이언트 연결 수락
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0); // 클라이언트 정보 가져오기
        printf("Accepted connection from (%s, %s)\n", hostname, port);                  // 연결된 클라이언트 정보 출력
        doit(connfd);                                                                   // 클라이언트 요청 처리
        Close(connfd);                                                                  // 연결 종료
    }
}
/* $end tinymain */

// 클라이언트 요청을 처리하는 함수
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("Request headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET"))
    {
        clienterror(fd, method, "501", "Not implemented",
                    "Tiny does not implement this method");
        return;
    }
    
    read_requesthdrs(&rio);

    /* Parse URI from GET request */
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0)
    {
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn’t find this file");
        return;
    }

    if (is_static)
    { /* Serve static content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn’t read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    else
    { /* Serve dynamic content */
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn’t run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

// 클라이언트에게 HTTP 오류 메시지를 보내는 함수
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=\"ffffff\">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

// HTTP 요청 헤더를 읽는 함수
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n"))
    {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

// URI를 파싱하여 정적/동적 컨텐츠 결정 함수
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin"))
    { /* Static content */
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "home.html");
        return 1;
    }
    else
    { /* Dynamic content */
        ptr = index(uri, '?');
        if (ptr)
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

// 정적 컨텐츠를 클라이언트에게 제공하는 함수
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    /*Send response body to client*/
    /*숙제 11.9 : 정적 컨텐츠를 처리할 때 요청한 파일을 mmap와 rio_rio_readn 대신,
     malloc, rio_readn, rio_written을 사용해서 연결 식별자에게 복사*/

    /*파일 내용을 저장할 메모리 할당*/
    srcp = (char*)Malloc(filesize);

    /* 파일 열기 */
    srcfd = Open(filename, O_RDONLY, 0);

    /* 파일 내용을 버퍼에 읽기 */
    Rio_readn(srcfd, srcp, filesize);

    /* 버퍼의 내용을 클라이언트에게 쓰기*/
    Rio_writen(fd, srcp, filesize);

    /* 할당된 메모리 해제*/
    Free(srcp);

    /* 파일 디스크립터 닫기*/
    Close(srcfd);

    /*파일 디스크립터를 열고, 파일 내용을 가상 메모리 영역에 매핑,
     그 내용을 네트워크로 전송 후 메모리 매핑을 해제 한 후 디스크립터를 닫음.*/
    //    srcfd = Open(filename, O_RDONLY, 0);
    //    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    //    Close(srcfd);
    //    Rio_writen(fd, srcp, filesize);
    //    Munmap(srcp, filesize); 

    /* - mmap()은 메모리의 특정 공간에 파일을 매핑한다. 
- 프로세스는 해당 파일을 읽기 위해서 저장매체에 접근하는 것이 아닌, 메모리의 데이터에 접근하는 방식이다.
- 그러므로 시스템 콜, 스케줄링, 인터럽트 등 OS의 개입과 저장매체로의 접근이 필요없다.
- 즉, 성능이 개선된다. 
출처: https://devraphy.tistory.com/428 [개발자를 향하여:티스토리]*/


}

/*
 * get_filetype - 파일 이름에서 파일 타입을 결정하는 함수
 */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else if (strstr(filename, ".mp4")) // 숙제 11.6 C MP4 파일 타입 추가
        strcpy(filetype, "video/mp4");
    else
        strcpy(filetype, "text/plain");
}

// 동적 컨텐츠를 클라이언트에게 제공하는 함수
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0)
    { /* Child */
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);              /* Redirect stdout to client */
        Execve(filename, emptylist, environ); /* Run CGI program */
    }
    Wait(NULL); /* Parent waits for and reaps child */
}
