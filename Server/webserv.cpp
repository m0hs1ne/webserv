#include "WebServ.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

WebServ::WebServ(char *av)
{
	this->kq = kqueue();
	this->servers = parsingConfig(av).getServers();
	this->timeout.tv_sec = 1, this->timeout.tv_nsec = 0;
}

// WebServ::WebServ( const WebServ & src )
// {
// }


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

// WebServ::~WebServ()
// {
// }


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

// WebServ &				WebServ::operator=( WebServ const & rhs )
// {
// 	//if ( this != &rhs )
// 	//{
// 		//this->_value = rhs.getValue();
// 	//}
// 	return *this;
// }

// std::ostream &			operator<<( std::ostream & o, WebServ const & i )
// {
// 	//o << "Value = " << i.getValue();
// 	return o;
// }


/*
** --------------------------------- METHODS ----------------------------------
*/

void WebServ::RunServer()
{
    SetUpSockets();
    while(true)
    {
        size_t kq_return = 0;
        
        struct kevent revents[10];
        kq_return = kevent(this->kq, 0, 0, revents, 10, &timeout);
        std::cout << "waiting fot new connections" << std::endl;
        if (kq_return < 0)
        {
            perror("failed");
            exit(1);
        }
        if (kq_return == 0)
        {
            std::cout << "timed out\n" << std::endl;
            continue;
        }
        NewConnections_Handler(revents, kq_return);
        Established_connections_handler(revents, kq_return);
    }

}

void WebServ::SetUpSockets()
{
    int valread;
    
    for (size_t i = 0; i <  servers.size(); i++)
    {
        SocketConnection Socket;
        struct kevent event;
        if (!( Socket.socket_fd = socket(AF_INET, SOCK_STREAM, 0)))
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(Socket.socket_fd, SOL_SOCKET, SO_REUSEPORT, &valread, sizeof(valread)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        Socket.addr.sin_family = AF_INET;
        Socket.addr.sin_addr.s_addr = INADDR_ANY;
        Socket.addr.sin_port = htons(servers[i].port);
        Socket.socket_port = this->servers[i].port;
        memset(Socket.addr.sin_zero, '\0', sizeof(Socket.addr.sin_zero));
        if (bind(Socket.socket_fd, (struct sockaddr *)&Socket.addr, sizeof(Socket.addr)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(Socket.socket_fd, 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        std::cout << "Listening on port " << servers[i].port << std::endl;
        this->Sockets.push_back(Socket);
        EV_SET(&event, Socket.socket_fd, EVFILT_READ | EVFILT_WRITE,EV_ADD, 0, 0, NULL);
        kevent(this->kq, &event, 1, 0,0,0);
    }
}

void WebServ::NewConnections_Handler(struct kevent *revents, size_t kq_return)
{
    for(size_t i = 0; i < this->servers.size(); i++)
    {
        for(size_t j = 0; j < kq_return ; j++)
        {
            if(revents[j].ident == this->Sockets[i].socket_fd)
            {
                struct kevent event;
                Connections new_socket;
                socklen_t len = sizeof(Sockets[i].addr);
                new_socket.Connec_fd = accept(Sockets[i].socket_fd, (sockaddr *)&Sockets[i].addr, &len);
                new_socket.is_write = 0;
                fcntl(new_socket.Connec_fd, F_SETFL, O_NONBLOCK);
                new_socket.is_read = 0;
                EV_SET(&event, new_socket.Connec_fd, EVFILT_READ | EVFILT_WRITE ,  EV_ADD, 0, 0, NULL);
                kevent(this->kq, &event, 1, 0,0,0);
                this->Sockets[i].connections.push_back(new_socket);
            }
        }
    }
}

void WebServ::Established_connections_handler(struct kevent *revents, size_t kq_return)
{
    for(size_t i = 0; i < kq_return; i++)
    {
        for(size_t j = 0; j < this->Sockets.size(); j++)
        {
            for(size_t k = 0; k < this->Sockets[j].connections.size(); k++)
            {
                if(revents[i].ident == this->Sockets[j].connections[k].Connec_fd)
                {
                    if(revents[i].filter ==  EVFILT_READ)
                    {
                        int ret;
                        char *buffer = new char[revents[i].data];
                        ret = recv(Sockets[j].connections[k].Connec_fd, buffer, revents[i].data, 0);
                        write(1, buffer, revents[i].data);
                        this->Sockets[j].connections[k].request.append(buffer);
                        // std::cout << "buffer -> " << this->Sockets[j].connections[k].request << std::endl;
                    }
                    else if(revents[i].filter ==  EVFILT_WRITE)
                    {
                        
                        std::cout << "Answer me" << std::endl;
						exit(1);
                    }
                }
                std::cout << this->Sockets[j].connections[k].request << std::endl;
            }   
        }
    }
}


/*
** --------------------------------- ACCESSOR ---------------------------------
*/


/* ************************************************************************** */