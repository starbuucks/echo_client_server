#include <stdio.h> // for perror
#include <stdlib.h>
#include <string.h> // for memset
#include <unistd.h> // for close
#include <arpa/inet.h> // for htons
#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket

#include <vector>
#include <thread>
#include <mutex>

#include "pcap_handle.h"

using namespace std;

vector<int> connected_socket;
mutex m;

void echo_session(int childfd){

	while (true) {
		const static int BUFSIZE = 1024;
		char buf[BUFSIZE];

		ssize_t received = recv(childfd, buf, BUFSIZE - 1, 0);
		if (received == 0 || received == -1) {
			perror("recv failed");
			break;
		}
		buf[received] = '\0';
		printf("%s\n", buf);

		ssize_t sent = send(childfd, buf, strlen(buf), 0);
		if (sent == 0) {
			perror("send failed");
			break;
		}
	}
}

void broadcast_session(int childfd){
	m.lock();
	connected_socket.push_back(childfd);
	m.unlock();

	while (true) {
		const static int BUFSIZE = 1024;
		char buf[BUFSIZE];

		ssize_t received = recv(childfd, buf, BUFSIZE - 1, 0);
		if (received == 0 || received == -1) {
			perror("recv failed");
			break;
		}
		buf[received] = '\0';
		printf("%s\n", buf);

		m.lock();
		for(vector<int>::iterator it = connected_socket.begin();
			it != connected_socket.end();
			it++)
		{
			ssize_t sent = send(*it, buf, strlen(buf), 0);
			if (sent == 0) {
				perror("send failed");
				break;
			}	
		}
		m.unlock();
	}

	m.lock();
	for(vector<int>::iterator it = connected_socket.begin();
		it != connected_socket.end();
		it++)
	{
		if(*it == childfd){
			connected_socket.erase(it);
			break;
		}
	}
	m.unlock();
}

void usage() {
	printf("syntax : echo_server <port> [-b]\n");
	printf("sample : echo_server 1234 -b\n");
}

int main(int argc, char* argv[]) {
	if (argc != 2 && argc != 3) {
		usage();
		return -1;
	}

	void (*thread_session)(int);
	if (argc == 3) thread_session = broadcast_session;
	else		thread_session = echo_session;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("socket failed");
		return -1;
	}

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[1]));
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	int res = bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(struct sockaddr));
	if (res == -1) {
		perror("bind failed");
		return -1;
	}

	res = listen(sockfd, 2);
	if (res == -1) {
		perror("listen failed");
		return -1;
	}

	while (true) {
		struct sockaddr_in addr;
		socklen_t clientlen = sizeof(sockaddr);
		int childfd = accept(sockfd, reinterpret_cast<struct sockaddr*>(&addr), &clientlen);
		if (childfd < 0) {
			perror("ERROR on accept");
			break;
		}
		printf("connected\n");

		thread session_thread(thread_session, childfd);
		session_thread.detach();
	}

	close(sockfd);
}
