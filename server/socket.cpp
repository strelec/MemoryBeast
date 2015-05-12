#include <sys/socket.h>
#include <arpa/inet.h>

#include <functional>

struct Socket {

	int socket_desc, client_sock;
	struct sockaddr_in server, client;
	char message[1024*1024];

	Socket(int port) {
		create(port);
		bind();
	}

	string loop(function<void(string)> f) {
		while(1) {
			puts("Waiting for incoming connections...");
			int c = sizeof(struct sockaddr_in);

			//accept connection from an incoming client
			client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
			if (client_sock < 0)
				return "accept failed";
			puts("Connection accepted");

			//Receive a message from client
			int size;
			string buffer;
			while( (size = recv(client_sock, message, sizeof(message), 0)) > 0 ) {
				for (int i=0; i<size; ++i) {
					if (message[i] == '\n') {
						f(buffer);
						buffer.clear();
					} else {
						buffer += message[i];
					}
				}
			}
			if (!buffer.empty())
				f(buffer);

			if (size == 0) {
				puts("Client disconnected");
			} else if (size == -1) {
				return "recv failed";
			}
		}
	}

	void write(string result) {
		::write(client_sock , result.c_str(), result.size());
	}


private:

	void create(int port) {
		//Create socket
		socket_desc = socket(AF_INET , SOCK_STREAM , 0);
		if (socket_desc == -1) {
			printf("Could not create socket");
		}
		puts("Socket created");

		//Prepare the sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons(port);
	}

	void bind() {
		//Bind
		if( ::bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
			//print the error message
			perror("bind failed. Error");
			exit(1);
		}
		puts("bind done");

		//Listen
		listen(socket_desc , 3);
	}
};
