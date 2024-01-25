#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <sys/time.h>

char square[10] = { 'o', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    fd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("Error in connecting to server");
    }

    printf("Connected To Main Server!\n");
    return fd;
}

void board()
{   
    printf("\n");
    printf("\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", square[1], square[2], square[3]);
    printf("_____|_____|_____\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", square[4], square[5], square[6]);
    printf("_____|_____|_____\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", square[7], square[8], square[9]);
    printf("     |     |     \n\n");
    printf("\n");
    printf("\n");
}

int checkwin()
{
    if (square[1] == square[2] && square[2] == square[3])
        return 1;   
    else if (square[4] == square[5] && square[5] == square[6])
        return 1;
    else if (square[7] == square[8] && square[8] == square[9])
        return 1;
    else if (square[1] == square[4] && square[4] == square[7])
        return 1;
    else if (square[2] == square[5] && square[5] == square[8])
        return 1;
    else if (square[3] == square[6] && square[6] == square[9])
        return 1;
    else if (square[1] == square[5] && square[5] == square[9])
        return 1;
    else if (square[3] == square[5] && square[5] == square[7])
        return 1;
    else if (square[1] != '1' && square[2] != '2' && square[3] != '3' &&
        square[4] != '4' && square[5] != '5' && square[6] != '6' && square[7] 
        != '7' && square[8] != '8' && square[9] != '9')
        return 0;
    else
        return  - 1;
}

int play(int choice, char x)
{
    int i;
    char mark = (x == 'A') ? 'X' : 'O';
    if (choice == 1 && square[1] == '1')
        square[1] = mark;
    else if (choice == 2 && square[2] == '2')
        square[2] = mark;
    else if (choice == 3 && square[3] == '3')
        square[3] = mark; 
    else if (choice == 4 && square[4] == '4')
        square[4] = mark;  
    else if (choice == 5 && square[5] == '5')
        square[5] = mark;  
    else if (choice == 6 && square[6] == '6')
        square[6] = mark;
    else if (choice == 7 && square[7] == '7')
        square[7] = mark;
    else if (choice == 8 && square[8] == '8')
        square[8] = mark; 
    else if (choice == 9 && square[9] == '9')
        square[9] = mark;  
    else
    {
        printf("Invalid move\n");
    }
    board();
    i = checkwin();
    if (i != -1) {
        if ( x == 'A') {
            printf("<<< Player One Won The Game >>>");
        } else {
            printf("<<< Player Two Won The Game >>>");
        }

    }
}

void play_with_opponent(int port, char x) {
    printf("port is %d\n", port);
    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    struct sockaddr_in bc_address;
    char opponent_id;
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));

    if (x == 'A') {
        opponent_id = 'B';
    } else {
        opponent_id = 'A';
    }

    if (x == 'A') {
        memset(buffer, 0, 1024);
        board();
        read(0, buffer, 1024);
        int choice;
        sscanf(buffer, "%d", &choice);
        play(choice, x);
        buffer[1] = x;
        strcat(buffer, square);
        printf("%s\n", buffer);
        int a = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
        printf("a is : %d\n", a);
        printf("sock: %d\n", sock);
        printf(".....................................%s................\n", buffer);
        printf("Sent By A\n");
    }

    printf("out of if\n");

    while (1) {
        printf("sock: %d\n", sock);
        memset(buffer, 0, 1024);
        printf("WATING FOR OPPONENT ...\n");
        recv(sock, buffer, 1024, 0);
        printf("%s\n", buffer);
        printf("GOT IT ............................\n");

        if (buffer[1] != x) {
        printf("GOT IT ............................\n");
            int choice;
            sscanf(buffer, "%d", &choice);
            play(choice, opponent_id);
            printf("YOUR TURN ...\n");
            memset(buffer, 0, 1024);
            read(0, buffer, 1024);
            sscanf(buffer, "%d", &choice);
            play(choice, x);
            buffer[1] = x;
            strcat(buffer, square);
            int a = sendto(sock, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_address, sizeof(bc_address));
        }
    }
}

void watch_game(int port) {
    int sock, broadcast = 1, opt = 1;
    char buffer[1024] = {0};
    struct sockaddr_in bc_address;
    char opponent_id;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET; 
    bc_address.sin_port = htons(port); 
    bc_address.sin_addr.s_addr = inet_addr("255.255.255.255");

    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));

    recv(sock, buffer, 1024, 0);


    while (1) {
        memset(buffer, 0, 1024);
        recv(sock, buffer, 1024, 0);
        square[1] = buffer[3];
        square[2] = buffer[4];
        square[3] = buffer[5];
        square[4] = buffer[6];
        square[5] = buffer[7];
        square[6] = buffer[8];
        square[7] = buffer[9];
        square[8] = buffer[10];
        square[9] = buffer[11];
        board();
    }
}

int main(int argc, char const *argv[]) {
    int fd;
    char buff[1024] = {0};
    fd = connectServer(8080);
    memset(buff, 0, 1024);
    recv(fd, buff, 1024, 0);
    printf("%s\n", buff);
    memset(buff, 0, 1024);
    int choice;
    read(0, buff, 1024);
    choice = atoi(buff);

    if (choice != 0){
        send(fd, buff, strlen(buff), 0);
    }

    if (choice == 2) {
        printf("Waiting to find a server...\n");
        memset(buff, 0, 1024);
        recv(fd, buff, 1024, 0);
        printf("You are now on port %s.\n", buff);
        int broadcast_port = atoi(buff);
        memset(buff, 0, 1024);
        recv(fd, buff, 1024, 0);
        printf("You are Player  %s.\n", buff);
        char player_id = buff[0];
        play_with_opponent(broadcast_port, player_id);
    }

    if (choice == 1) {
        memset(buff, 0, 1024);
        recv(fd, buff, 1024, 0);
        printf("%s\n", buff);
        memset(buff, 0, 1024);
        read(0, buff, 1024);
        printf("Watchin Game On Port: %s\n", buff);
        int game_port = atoi(buff);
        watch_game(game_port);
    }

    return 0;
}