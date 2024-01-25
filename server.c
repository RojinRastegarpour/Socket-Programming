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


int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}


int main(int argc, char const *argv[]) {
    int play_port = 3999;
    int playground_count = 0;
    int active_ports[9000] = {0};
    int is_wating_to_play[1024] = {0};
    int is_watching[1024] = {0};
    int is_playing[1024] = {0};
    char active_ports_str[1024];
    int player_one, player_two;
    int server_fd, new_socket, max_sd;
    char buffer[1024] = {0};
    fd_set master_set, working_set;
    server_fd = setupServer(8080);

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running\n", 18);

    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                
                if (i == server_fd) {  // new clinet
                    new_socket = acceptClient(server_fd);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                    char menu[] = "Play Or Watch?\nEnter 1 For Watching\nEnter 2 For Playing\n";
                    send(new_socket, menu, strlen(menu), 0);
                    printf("SENT\n");
                    is_watching[new_socket] = 0;
                    is_wating_to_play[new_socket] = 0;
                    is_playing[new_socket] = 0;
                }
                
                else { // client sending msg
                    memset(buffer, 0, 1024);
                    int bytes_received;
                    bytes_received = recv(i , buffer, 1024, 0);
                    
                    if (bytes_received == 0) { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }

                    if (is_watching[i] == 0 && is_wating_to_play[i] == 0) {
                        int pos = atoi(buffer);
                        switch (pos)
                        {
                        case 1:
                            memset(active_ports_str, 0,strlen(buffer));
                            for (int i = 3999; i < 4040; i++) {
                                char temp_port[100];
                                if (active_ports[i] == 1) {
                                    sprintf(temp_port, "%d ", i);
                                    strncat(active_ports_str, temp_port, 1000);
                                }
                            }

                            send(i, active_ports_str, 1024, 0);

                            is_watching[i] = 1;
                            break;
                        case 2:
                            if (playground_count == 0) {
                                is_wating_to_play[i] = 1;
                                player_one = i;
                                playground_count++;
                                break;
                            }

                            if (playground_count == 1) {
                                is_wating_to_play[i] = 1;
                                player_two = i;
                                playground_count++;
                                break;    
                            }

                        default:
                            printf("Input Not Valid\n");
                        }

                        if (playground_count == 2) {
                            printf("I'm Here 8");
                            char port[4];
                            printf("I'm Here 9");
                            sprintf(port, "%d", play_port);
                            send(player_one, port, 1024, 0);
                            send(player_one, "A", 1024, 0);
                            send(player_two, port, 1024, 0);
                            send(player_two, "B", 1024, 0);
                            is_playing[player_one] = 1;
                            is_playing[player_two] = 1;
                            playground_count = 0;
                            printf("I'm Here 10");
                            active_ports[play_port] = 1;
                            play_port++;
                        }
                    }       

                    printf("client %d: %s\n", i, buffer);
                }
            }
        }
    }

    return 0;
}