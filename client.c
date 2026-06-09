#include "csapp.h"

int main(int argc, char **argv) {
    int clientfd;
    char buf[MAXLINE];
    rio_t rio;

    clientfd = Open_clientfd(argv[1], argv[2]);
    rio_readinitb(&rio, clientfd);

    while (1) {
    Rio_readlineb(&rio, buf, MAXLINE);

    if (strcmp(buf, "INVALID\n") == 0) {
        printf("Invalid move. Try again.\n");
        continue;
    }

    if (strcmp(buf, "BOARD\n") == 0) {
        for (int i = 0; i < 3; i++) {
            Rio_readlineb(&rio, buf, MAXLINE);
            printf("%s", buf);
        }

        Rio_readlineb(&rio, buf, MAXLINE);

        if (strcmp(buf, "YOURTURN\n") == 0) {
            printf("Your turn. Enter move (row col): ");
            fgets(buf, MAXLINE, stdin);
            Rio_writen(clientfd, buf, strlen(buf));
        }
        else if (strcmp(buf, "WAIT\n") == 0) {
            printf("Waiting for opponent...\n");
        } else if (strcmp(buf, "WIN\n") == 0) {
            printf("You win!\n");
            break;
        }
        else if (strcmp(buf, "LOSE\n") == 0) {
            printf("You lose.\n");
            break;
        }
        else if (strcmp(buf, "DRAW\n") == 0) {
            printf("Draw game.\n");
            break;
        }
    }
    }
}