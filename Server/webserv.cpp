#include "WebServ.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

WebServ::WebServ(char *av)
{
    this->kq = kqueue();
    this->Answer = 0;
    this->servers = parsingConfig(av).getServers();
    this->timeout.tv_sec = 1, this->timeout.tv_nsec = 100 * 500000;
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
        if (kq_return == 0)
        {
            std::cout << "timed out\n"
                      << std::endl;
            continue;
        }
        std::cout << "kq return --> " << kq_return << std::endl;
        if (NewConnections_Handler(revents, kq_return))
            continue;
        Read_connections(revents, kq_return);
        // Answer_Connections();
        drop_clients();
        // exit(0);
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
        EV_SET(&event, Socket.socket_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
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
                struct kevent event[2];
                Connections new_socket;
                socklen_t len = sizeof(it->addr);
                new_socket.Connec_fd = accept(it->socket_fd, (sockaddr *)&it->addr, &len);
                fcntl(new_socket.Connec_fd, F_SETFL, O_NONBLOCK);
                new_socket.drop_connection = 0;
                new_socket.data_s = 0;
                new_socket.content_size = 0;
                EV_SET(&event[0], new_socket.Connec_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                EV_SET(&event[1], new_socket.Connec_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
                kevent(this->kq, event, 2, 0, 0, 0);
                it->connections.push_back(new_socket);
                this->Answer++;
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
    for (it = this->Sockets.begin(); it != this->Sockets.end(); it++)
    {
        for (it2 = it->connections.begin(); it2 != it->connections.end(); it2++)
        {
            if (is_readable(revents, kq_return, it2->Connec_fd))
            {
                int ret = 0;
                if (access("vedio.mp4", F_OK))
                    this->file.open("vedio.mp4", std::ios::binary | std::ios::app);
                char buffer[2048] = {0};
                ret = read(it2->Connec_fd, buffer, 2047);
                buffer[ret] = '\0';
                if (it2->header.empty())
                {
                    it2->header.append(buffer);
                    if (RequestType(it2->header) == "POST")
                        Get_ContentSize(buffer, it2);
                    else
                        it2->content_size = 0;
                }
                else
                {          
                    
                    // exit(0);
                    this->file.write(buffer, ret);
                    it2->content_size -= ret;
                    std::cout << it2->content_size << std::endl;
                }
                
                // it2->Body.append(buffer);
                // std::cout << it2->Body << std::endl;
                // exit(0);
                // std::cout << it2->content_size << std::endl;
                if (it2->content_size <= 0)
                {
                    AddEvent(it2->Connec_fd, EVFILT_READ, EV_DELETE);
                    AddEvent(it2->Connec_fd, EVFILT_WRITE, EV_ENABLE);
                }
                // outfile.close();
            }
            else if (is_writable(revents, kq_return, it2->Connec_fd))
            {
                // if()
                // it2->response = handleRequest(it2->request, this->servers[0]);
                write(it2->Connec_fd, "Hello", 5);
                AddEvent(it2->Connec_fd, EVFILT_WRITE, EV_DELETE);
                close(it2->Connec_fd);
                it2->drop_connection = 1;
                std::cout << "Answer \n"
                          << std::endl;
                // std::cout << it2->Body << std::endl;
                exit(1);
            }
        }
    }
}

int WebServ::is_readable(struct kevent *revents, size_t kq_return, uint64_t socket_fd)
{
    for (size_t i = 0; i < kq_return; i++)
    {
        if (revents[i].ident == socket_fd)
        {
            if (revents[i].filter == EVFILT_READ)
                return 1;
        }
    }
    return 0;
}

int WebServ::is_writable(struct kevent *revents, size_t kq_return, uint64_t socket_fd)
{
    for (size_t i = 0; i < kq_return; i++)
    {
        if (revents[i].ident == socket_fd)
        {
            if (revents[i].filter == EVFILT_WRITE)
                return 1;
        }
    }
    return 0;
}

void WebServ::Get_ContentSize(std::string buffer, std::vector<Connections>::iterator it2)
{
    std::string Con_Len;
    size_t pos = buffer.find("Content-Length: ");
    if (pos != std::string::npos)
    {
        pos += 16;
        size_t end = buffer.find("\r\n", pos);
        Con_Len = buffer.substr(pos, end - pos);
        it2->content_size = std::stoi(Con_Len);
    }
}

void WebServ::drop_clients()
{
    std::vector<SocketConnection>::iterator it;
    std::vector<Connections>::iterator it2;
    for (it = this->Sockets.begin(); it != this->Sockets.end(); it++)
    {
        for (it2 = it->connections.begin(); it2 != it->connections.end();)
        {
            if (it2->drop_connection)
                it2 = it->connections.erase(it2);
            else
                it2++;
        }
    }
}

void WebServ::DeleteEvent(int fd)
{
    struct kevent event[2];
    EV_SET(&event[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    EV_SET(&event[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    kevent(this->kq, event, 2, 0, 0, 0);
}

void WebServ::AddEvent(int fd, int16_t filter, uint16_t flag)
{
    struct kevent event;
    EV_SET(&event, fd, filter, flag, 0, 0, NULL);
    int err = kevent(this->kq, &event, 1, 0, 0, 0);
    if (err == -1)
        std::cout << "An error occured\n";
}

std::string WebServ::RequestType(std::string buffer)
{
    return buffer.substr(0, 4);
}
/*
** --------------------------------- ACCESSOR ---------------------------------
*/

/* ************************************************************************** */