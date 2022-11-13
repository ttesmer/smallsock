#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 50
#define BUFFER_SIZE 3000

void error(const char *msg) {
    do {
        perror(msg);
        exit(EXIT_FAILURE);
    } while (0);
}

void write_file(FILE *file_pointer, int sfd) {}

void http_response(int new_sfd) {
    int n;
    char *msg, *msg_body;
    char buffer[BUFFER_SIZE];

    memset(buffer, 0, BUFFER_SIZE);

    n = read(new_sfd, buffer, BUFFER_SIZE-1);
    if (n < 0) error("ERROR on read");

    printf("Received HTTP Request:\n%s", buffer);
    msg = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n";
    msg_body = "<h1 style='color:#dc143c'>Hello World!</h1>\n";
    n = write(new_sfd, msg, strlen(msg));
    n = write(new_sfd, msg_body, strlen(msg_body));
    if (n < 0) error("ERROR writing to socket");
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr, client_addr;
    int cli_len, PORT_NO, new_sfd, sfd;
    sfd = socket(AF_INET, SOCK_STREAM, 0); 
    PORT_NO = 8000;

    if (sfd < 0) 
        error("ERROR opening socket");
    if (argv[1]) 
        PORT_NO = atoi(argv[1]);

    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NO);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sfd, (struct sockaddr *) &serv_addr
                , sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    char* ip = inet_ntoa(serv_addr.sin_addr);
    printf("Server listening on %s port %d ...\n" , ip, PORT_NO);

    pid_t pid;

    if (listen(sfd, BACKLOG) < 0) error("ERROR on listening");

    for (;;) {
        cli_len = sizeof(client_addr);
        new_sfd = accept(sfd
                , (struct sockaddr*) &client_addr
                , &cli_len);
        if (new_sfd < 0) 
            error("ERROR on accept");

        char *client_ip = inet_ntoa(client_addr.sin_addr);
        printf("Connection established with client ip '%s'\n", client_ip);

        pid = fork(); // handle multiple connections
        if (pid < 0) {
            error("ERROR on fork");
        } else if (pid == 0) {
            close(sfd);
            http_response(new_sfd); // send HTTP response to client socket
            exit(0);
        } else {
            close(new_sfd);
        }
    }
    return 0;
}
