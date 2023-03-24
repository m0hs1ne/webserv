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
		int NewConnections_Handler(struct kevent *revents, size_t kq_return);
		void Read_connections(struct kevent *revents, size_t kq_return);
		void Answer_Connections();
		void read_socket();
		void write_socket();
		int ready_to_read(struct kevent *revents, size_t kq_return, uint64_t socket_fd);
		void drop_clients();
		void DeleteEvent(int fd);
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
		int Answer;

};


#endif /* ********************************************************* WEBSERV_H */