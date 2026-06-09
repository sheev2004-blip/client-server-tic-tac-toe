#include "csapp.h"

#define MAX_CLIENTS 100

typedef struct {
    int connfd;
    int in_game;
} client_t;

client_t *waiting_client = NULL;
pthread_mutex_t lock;

typedef struct {
    int player1_fd;
    int player2_fd;
    char board[3][3];
    int turn; // 1 or 2
} game_t;

void init_board(char board[3][3]) {
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            board[i][j] = '-';
}

int check_win(char board[3][3], char symbol) {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == symbol &&
            board[i][1] == symbol &&
            board[i][2] == symbol) {
            return 1;
        }
    }
    for (int j = 0; j < 3; j++) {
        if (board[0][j] == symbol &&
            board[1][j] == symbol &&
            board[2][j] == symbol) {
            return 1;
        }
    }
    if (board[0][0] == symbol &&
        board[1][1] == symbol &&
        board[2][2] == symbol) {
        return 1;
    }

    if (board[0][2] == symbol &&
        board[1][1] == symbol &&
        board[2][0] == symbol) {
        return 1;
    }
    return 0;
}

int check_draw(char board[3][3])
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == '-') {
                return 0; // not a draw yet
            }
        }
    }

    return 1; // board is full
}

void send_game_state(game_t *game) {
    printf("SERVER: send_game_state called\n");
    fflush(stdout);
    char buffer[MAXLINE];
    int active_fd;
    int waiting_fd;

    if (game->turn == 1) {
        active_fd = game->player1_fd;
        waiting_fd = game->player2_fd;
    } else {
        active_fd = game->player2_fd;
        waiting_fd = game->player1_fd;
    }

    printf("SERVER: writing BOARD to active_fd %d\n", active_fd);
    fflush(stdout);

    Rio_writen(active_fd, "BOARD\n", strlen("BOARD\n"));
    
    printf("SERVER: writing BOARD to waiting_fd %d\n", waiting_fd);
    fflush(stdout);
    
    Rio_writen(waiting_fd, "BOARD\n", strlen("BOARD\n"));
    
    sprintf(buffer, 
        "%c %c %c\n%c %c %c\n%c %c %c\n",
        game->board[0][0], game->board[0][1], game->board[0][2],
        game->board[1][0], game->board[1][1], game->board[1][2],
        game->board[2][0], game->board[2][1], game->board[2][2]);

    Rio_writen(active_fd, buffer, strlen(buffer));
    Rio_writen(active_fd, "YOURTURN\n", strlen("YOURTURN\n"));

    Rio_writen(waiting_fd, buffer, strlen(buffer));
    Rio_writen(waiting_fd, "WAIT\n", strlen("WAIT\n"));
}

void send_final_state(int fd, game_t *game, char *result)
{
    char buf[MAXLINE];

    Rio_writen(fd, "BOARD\n", strlen("BOARD\n"));

    snprintf(buf, MAXLINE, "%c %c %c\n",
             game->board[0][0], game->board[0][1], game->board[0][2]);
    Rio_writen(fd, buf, strlen(buf));

    snprintf(buf, MAXLINE, "%c %c %c\n",
             game->board[1][0], game->board[1][1], game->board[1][2]);
    Rio_writen(fd, buf, strlen(buf));

    snprintf(buf, MAXLINE, "%c %c %c\n",
             game->board[2][0], game->board[2][1], game->board[2][2]);
    Rio_writen(fd, buf, strlen(buf));

    Rio_writen(fd, result, strlen(result));
}

void *game_thread(void *vargp) {
    game_t *game = (game_t *)vargp;
    char buf[MAXLINE];
    int row, col;

    init_board(game->board);
    game->turn = 1;

    send_game_state(game);

    while (1) {
        int current_fd = (game->turn == 1) ? game->player1_fd : game->player2_fd;

        rio_t rio;
        Rio_readinitb(&rio, current_fd);
        Rio_readlineb(&rio, buf, MAXLINE);
        sscanf(buf, "%d %d", &row, &col);
        row--;
        col--;

        char symbol = (game->turn == 1) ? 'X' : 'O';

        if (row < 0 || row > 2 || col < 0 || col > 2) {
            Rio_writen(current_fd, "INVALID\n", strlen("INVALID\n"));
            send_game_state(game); // same player still has turn
            continue;
        }

        if (game->board[row][col] != '-') {
            Rio_writen(current_fd, "INVALID\n", strlen("INVALID\n"));
            send_game_state(game); // same player still has turn
            continue;
        }

        game->board[row][col] = symbol;

        if (check_win(game->board, symbol)) {
            int winner_fd = (game->turn == 1) ? game->player1_fd : game->player2_fd;
            int loser_fd  = (game->turn == 1) ? game->player2_fd : game->player1_fd;

            send_final_state(winner_fd, game, "WIN\n");
            send_final_state(loser_fd, game, "LOSE\n");

            break;
        }

        if (check_draw(game->board)) {
            send_final_state(game->player1_fd, game, "DRAW\n");
            send_final_state(game->player2_fd, game, "DRAW\n");
            break;
        }

        // only switch turns if no one won
        game->turn = (game->turn == 1) ? 2 : 1;

        send_game_state(game);
    }

    return NULL;
}

void *client_handler(void *vargp) {
    int connfd = *((int *)vargp);
    Free(vargp);

    Pthread_detach(pthread_self());

    pthread_mutex_lock(&lock);

    if (waiting_client == NULL) {
        waiting_client = Malloc(sizeof(client_t));
        waiting_client->connfd = connfd;
        waiting_client->in_game = 0;

        pthread_mutex_unlock(&lock);

        // just wait
    } else {
        game_t *game = Malloc(sizeof(game_t));
        game->player1_fd = waiting_client->connfd;
        game->player2_fd = connfd;

        waiting_client = NULL;

        pthread_t tid;
        Pthread_create(&tid, NULL, game_thread, game);

        pthread_mutex_unlock(&lock);
    }

    return NULL;
}

int main(int argc, char **argv) {
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    pthread_mutex_init(&lock, NULL);

    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfdp = Malloc(sizeof(int));
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        Pthread_create(&tid, NULL, client_handler, connfdp);
    }
}