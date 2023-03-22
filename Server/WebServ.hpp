# ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <string>
# include "Sockets.hpp" // Sockets Header
# include "../includes/handlingRequest.hpp" // parsing Header
# include <sys/socket.h>
# include <cstdio>
# include <unistd.h>
# include <iostream>
# include <netinet/in.h>
# include <sys/time.h>
# include <sys/event.h>

#define MAX 5000

class WebServ
{

	public:
		void RunServer();
		void SetUpSockets();
		void NewConnections_Handler(struct kevent *revents, size_t kq_return);
		void Established_connections_handler(struct kevent *revents, size_t kq_return);
		void read_socket();
		void write_socket();

		WebServ(char *av);
		// WebServ( WebServ const & src );
		// ~WebServ();

		// WebServ &		operator=( WebServ const & rhs );

	private:
		int kq;
		std::vector<SocketConnection> Sockets;
		struct timespec timeout;
		std::vector<parsingConfig::server> servers;
		parsingConfig a(char *av);
		

};


#endif /* ********************************************************* WEBSERV_H */