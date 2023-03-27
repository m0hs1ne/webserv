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
# include <fstream>
# include <string>

#define MAX 5000

class WebServ
{

	public:
		void RunServer();
		void SetUpSockets();
		int NewConnections_Handler(struct kevent *revents, size_t kq_return);
		void Read_connections(struct kevent *revents, size_t kq_return);
		void Answer_Connections(int fd, std::vector<Connections>::iterator it2);
		void read_socket();
		void write_socket();
		int is_readable(struct kevent *revents, size_t kq_return, uint64_t socket_fd);
		int is_writable(struct kevent *revents, size_t kq_return, uint64_t socket_fd);
		void Get_ContentSize(std::string buffer, std::vector<Connections>::iterator it2);
		void drop_clients();


		void DeleteEvent(int fd);
		void AddEvent(int fd, int16_t filter, uint16_t flag);
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