#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<signal.h>

void handle_client(int client_socket){
	char buffer[1024];
	printf("New client connected (fd=%d)\n", client_socket);

	//处理客户端请求
	while(1){
	    int bytes = recv(client_socket, buffer, sizeof(buffer), 0);
	    if(bytes <= 0){
		printf("Client disconnected (fd=%d)\n", client_socket);
	        break;
	    }
	    //调试输出
	    printf("Received %d bytes from fd = %d: %.*s\n", bytes, client_socket, bytes, buffer);
	    /*  send()的作用是客户端回显
	    int sent_bytes = send(client_socket, buffer, bytes, 0);
	    if(sent_bytes == -1){
	        perror("send failed");
		break;
	    }

	    if(sent_bytes != bytes){
	        printf("warning: Partial send (%d/%d bytes\n)", sent_bytes, bytes);
	    }
	    */
	}
	close(client_socket);
}

int main(){
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr = {0};
	server_addr.sin_family = AF_INET;
	//server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(8080);

	int ret;
	ret = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret == -1){
	    perror("bind failed");
	    close(server_socket);
	    exit(EXIT_FAILURE);
	}

	ret = listen(server_socket, 5);//最大允许5个客户端排队
	if(ret == -1){
	    perror("listen failed");
	    close(server_socket);
	    exit(EXIT_FAILURE);
	}
	signal(SIGCHLD, SIG_IGN); //避免僵尸进程

	while(1){
	    struct sockaddr_in client_addr = {0};
	    socklen_t client_size = sizeof(client_addr);
	    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_size);
	    if(client_socket == -1){
	        perror("accept failed");
		close(server_socket);
		return EXIT_FAILURE;
	    }

	    pid_t pid = fork();
	    if(pid == 0){  //子进程
	        close(server_socket);  //子进程不需要监听套接字
		handle_client(client_socket);
		close(client_socket);
		exit(0);
	    }else if(pid<0){
	        perror("fork failed");
		close(client_socket);
	    }else{
	        close(client_socket); //父进程不需要客户端套接字
	    }
	}
	return 0;
}
