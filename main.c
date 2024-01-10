#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#include "log.h"

const char* PORT = "710";
const int MSG_BACKLOG = 25;
const int ROOM_SIZE = 25;
const int BACKLOG = 10;

const int USERNAME_MAX = 25;
const int MESSAGE_MAX = 100;

int sfd;

int in_room = 0;

int running = 1;

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handle_client(int cfd) {
    uint8_t buff[1024];
    ssize_t received = recv(cfd, buff, 1024, 0);

    if (received < 1) {
        perror("recv");
        return;
    }

    int b = 0;
    char username[USERNAME_MAX + 1];
    char message[MESSAGE_MAX + 1];

    int u = 0;
    while (buff[b] != '|' && u != USERNAME_MAX) {
        username[u++] = buff[b++];
    }
    username[u] = 0;

    if (buff[b++] != '|') {
        perror("parsing");
        return;
    }

    if (buff[b++] != '\"') {
        perror("parsing");
        return;
    }

    int m = 0;
    while (buff[b] != '\"' && m != MESSAGE_MAX) {
        message[m++] = buff[b++];
    }
    message[m] = 0;

    if (buff[b] != '\"') {
        perror("parsing");
        return;
    }

    char to_add[USERNAME_MAX + MESSAGE_MAX + 4];
    memset(to_add, 0, USERNAME_MAX + MESSAGE_MAX + 4);
    sprintf(to_add, "%s | %s", username, message);

    push_log(to_add);
}

void sig_handler(int sig) {
    switch (sig) {
        case SIGINT:
            running = 0;
            close(sfd);
            free_log();
            printf("PopChat server shutting down...\n\n");
            exit(0);
        default:
            printf("Unhandled signal");
    }
}

int main()
{
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        perror("Failed to set up signal handler");
        return 1;
    }

    struct addrinfo hints;
    struct addrinfo *p;

    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &p)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    if ((sfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1) {
        perror("server: socket");
        exit(1);
    }

    int y = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &y,
                   sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(sfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sfd);
        perror("server: bind");
        exit(1);
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    init_log();

    printf("PopChat server: waiting for connections...\n");

    struct sockaddr_storage c_addr;
    socklen_t sin_size;

    while(running) {
        sin_size = sizeof c_addr;
        int cfd = accept(sfd, (struct sockaddr *)&c_addr, &sin_size);
        if (cfd == -1) {
            perror("accept");
            continue;
        }

        handle_client(cfd);

        close(cfd);
    }

    free_log();
    close(sfd);
    return 0;
}