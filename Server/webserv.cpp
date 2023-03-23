#include "WebServ.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

WebServ::WebServ(char *av)
{
    this->kq = kqueue();
    this->Answer = 0;
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
    while (true)
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
        // if (kq_return == 0)
        // {
        //     std::cout << "timed out\n"
        //               << std::endl;
        //     continue;
        // }
        std::cout << "kq return --> " << kq_return << std::endl;

        if (NewConnections_Handler(revents, kq_return))
            continue;
        Read_connections(revents, kq_return);
        Answer_Connections();
        // emxit(0);
    }
}

void WebServ::SetUpSockets()
{
    int valread;

    for (size_t i = 0; i < servers.size(); i++)
    {
        SocketConnection Socket;
        struct kevent event;
        if (!(Socket.socket_fd = socket(AF_INET, SOCK_STREAM, 0)))
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
        EV_SET(&event, Socket.socket_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        kevent(this->kq, &event, 1, 0, 0, 0);
    }
}

int WebServ::NewConnections_Handler(struct kevent *revents, size_t kq_return)
{
    std::vector<SocketConnection>::iterator it;
    std::vector<Connections>::iterator it2;
    for (it = this->Sockets.begin(); it != this->Sockets.end(); it++)
    {
        for (size_t j = 0; j < kq_return; j++)
        {
            if (revents[j].ident == it->socket_fd)
            {
                struct kevent event;
                Connections new_socket;
                socklen_t len = sizeof(it->addr);
                new_socket.Connec_fd = accept(it->socket_fd, (sockaddr *)&it->addr, &len);
                new_socket.is_write = 0;
                fcntl(new_socket.Connec_fd, F_SETFL, O_NONBLOCK);
                new_socket.is_read = 0;
                EV_SET(&event, new_socket.Connec_fd, EVFILT_READ | EVFILT_WRITE, EV_ADD, 0, 0, NULL);
                kevent(this->kq, &event, 1, 0, 0, 0);
                it->connections.push_back(new_socket);
                std::cout << "loop\n";
                return 1;
            }
        }
    }
    return 0;
}

void WebServ::Read_connections(struct kevent *revents, size_t kq_return)
{
    std::vector<SocketConnection>::iterator it;
    std::vector<Connections>::iterator it2;
    for (it = this->Sockets.begin(); it !=  this->Sockets.end(); it++)
    { 
        for (it2 = it->connections.begin(); it2 !=  it->connections.end(); it2++)
        {
            if (ready_to_read(revents, kq_return, it2->Connec_fd))
            {
                std::cout << "Read Me\n" << std::endl;
                int ret;
                char buffer[1024] = {0};
                ret = recv(it2->Connec_fd, buffer, 1023, 0);
                if(ret < 1023)
                    this->Answer++;
                buffer[ret] = '\0';
                it2->request.append(buffer);
            }
            else
            {
                it2->is_write = true;
                this->Answer++;
            }
            
        }
    }
}

int WebServ::ready_to_read(struct kevent *revents, size_t kq_return, uint64_t socket_fd)
{
    for (size_t i = 0; i < kq_return; i++)
    {
        if (revents[i].ident == socket_fd)
        {
            if (revents[i].filter == EVFILT_READ)
                return 1;
            else
            {
                std::cout << "No Event Occured" << std::endl;
                exit(0);
            }
        }
    }
    return 0;
}

void WebServ::Answer_Connections()
{
    std::vector<SocketConnection>::iterator it;
    std::vector<Connections>::iterator it2;
    for (it = this->Sockets.begin(); it !=  this->Sockets.end(); it++)
    {
        for (it2 = it->connections.begin(); it2 !=  it->connections.end(); it2++)
        {
            if (it2->is_write)
            {
                std::cout << "Answer Me" << std::endl;
                it2->is_write = 0;
                it->connections.erase(it2);
                break;
            }
        }
    }
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

/* ************************************************************************** */