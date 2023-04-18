# ifndef WEBSERV_HPP
# define WEBSERV_HPP

# define MAX_INT 10000
# define RD_BUFFER 1025
# define WR_BUFFER 1025
# include <iostream>
# include <string>

# include "Sockets.hpp" // Sockets Header
# include "ParsingConfig.hpp"
# include "Request.hpp" // parsing Header
#include "../includes/handlingGet.hpp"
#include "../includes/handlingPost.hpp"
#include "../includes/handlingDelete.hpp"
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
		int AcceptNewConnections(SocketConnection *Socket);
		void HandleEstablishedConnections(SocketConnection *Connection, int16_t  filter);
		int CheckEvents(struct kevent *revents, size_t kq_return);
		void AddEvent(int fd, int16_t filter, SocketConnection *udata);
		void DeleteEvent(int fd, int16_t filter);
		void Reciev(SocketConnection *Connection);
		void Send(SocketConnection *Connection);
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
		std::ofstream file;

};


#endif /* ********************************************************* WEBSERV_H */