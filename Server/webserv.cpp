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

/*
** --------------------------------- METHODS ----------------------------------
*/

void WebServ::RunServer()
{
    SetUpSockets();
    signal(SIGPIPE, SIG_IGN);
    while (true)
    {
        size_t kq_return = 0;
        struct kevent revents[10];
        kq_return = kevent(this->kq, 0, 0, revents, 10, &timeout);
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
        CheckEvents(revents, kq_return);
    }
}

void WebServ::SetUpSockets()
{
    int valread;
    for (size_t i = 0; i < servers.size(); i++)
    {
        SocketConnection *Socket = new SocketConnection;
        struct kevent event;
        if (!(Socket->socket_fd = socket(AF_INET, SOCK_STREAM, 0)))
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(Socket->socket_fd, SOL_SOCKET, SO_REUSEPORT, &valread, sizeof(valread)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        Socket->server = &servers[i];
        Socket->addr.sin_family = AF_INET;
        Socket->addr.sin_addr.s_addr = INADDR_ANY;
        Socket->addr.sin_port = htons(servers[i].port);
        Socket->socket_port = this->servers[i].port;
        Socket->IsPortSocket = 1;
        memset(Socket->addr.sin_zero, '\0', sizeof(Socket->addr.sin_zero));
        if (bind(Socket->socket_fd, (struct sockaddr *)&Socket->addr, sizeof(Socket->addr)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(Socket->socket_fd, 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        std::cout << "Listening on port " << servers[i].port << std::endl;
        EV_SET(&event, Socket->socket_fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, reinterpret_cast<void *>(Socket));
        kevent(this->kq, &event, 1, 0, 0, 0);
    }
}

int WebServ::AcceptNewConnections(SocketConnection *Socket)
{
    SocketConnection *new_Socket = new SocketConnection;
    struct kevent event[2];

    socklen_t len = sizeof(Socket->addr);
    new_Socket->drop_connection = 0;
    new_Socket->data_s = 0;
    new_Socket->content_size = 0;
    new_Socket->IsPortSocket = 0;
    new_Socket->server = Socket->server;
    new_Socket->socket_fd = accept(Socket->socket_fd, (sockaddr *)&Socket->addr, &len);

    // fcnt÷l(new_Socket->socket_fd, F_SETFL, O_NONBLOCK);
    EV_SET(&event[0], new_Socket->socket_fd, EVFILT_READ, EV_ADD, 0, 0, reinterpret_cast<void *>(new_Socket));
    EV_SET(&event[1], new_Socket->socket_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, reinterpret_cast<void *>(new_Socket));
    kevent(this->kq, event, 2, 0, 0, 0);
    return 1;
}

size_t fullsize = 0;

void WebServ::HandleEstablishedConnections(SocketConnection *Connection, int16_t  filter)
{
    std::map<int, std::string> *code = new std::map<int, std::string>;
    initHttpCode(*code);
    struct  kevent event[2];
    
    if (filter == EVFILT_READ)
    {
        int ret = 0;
        char buffer[4001] = {0};
        ret = read(Connection->socket_fd, buffer, 4000);
        if(ret == 0)
        {
            write(Connection->socket_fd, "hello", 5);
            close(Connection->socket_fd);
        }
        buffer[ret] = '\0';
        fullsize += ret;
        // std::cout << "fullsize --> " << fullsize << " | ";
        //std::cout << buffer << std::endl;
        Connection->request.buffer_size = ret;
        if (Connection->request.method.empty() || Connection->request.bFd != -2 || Connection->request.openedFd != -2)
        {
            Connection->response.codeMsg = code;
            Connection->response = Connection->request.handleRequest(ft_strdup(buffer, ret), this->servers[0]);
        }
        if (Connection->request.ok)
        {
            if (Connection->request.method == "POST")
                handlingPost(*Connection);
            else if (Connection->request.method == "GET")
                handlingGet(*Connection); // GET
            else if (Connection->request.method == "DELETE")
                handlingDelete(*Connection); // DELETE
        }
        Connection->response.formResponse(Connection->request.method, *(Connection->server));
        if (Connection->ended)
        {
            // std::cou÷t 
            EV_SET(&event[0],  Connection->socket_fd, EVFILT_READ, EV_ADD | EV_DELETE, 0, 0, reinterpret_cast<void *>(Connection));
            EV_SET(&event[1],  Connection->socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, reinterpret_cast<void *>(Connection));
            kevent(this->kq, event, 2, 0, 0, 0);
        }
    }
    else if (filter == EVFILT_WRITE)
    {
        char buffer[4001] = {0};
        int _return;

        if (!Connection->response.response.empty())
        {
            // Connection->response.response += "Content-Length: 11486837";
            write(Connection->socket_fd, Connection->response.response.c_str(), Connection->response.response.size());
            Connection->response.response.clear();
        }
        else if (!Connection->response.returnFile.empty())
        {
            _return = read(Connection->response.fileFD, buffer, 4000);
            if (_return > 0)
            {
                // std::cout << buffer << std::endl;
                write(Connection->socket_fd, buffer, _return);
            }
            else
            {
                Connection->drop_connection = 1;
                std::cout << "Answer " << std::endl;
                AddEvent(Connection->socket_fd, EVFILT_WRITE, EV_DELETE);
                close(Connection->socket_fd);
            }
        }
        else
        {
            write(Connection->socket_fd, Connection->response.body.c_str(), Connection->response.body.size());
            Connection->drop_connection = 1;
            std::cout << "Answer " << std::endl;
            AddEvent(Connection->socket_fd, EVFILT_WRITE, EV_DELETE);
            close(Connection->socket_fd);
        }
    }
    delete code;
}

int WebServ::CheckEvents(struct kevent *revents, size_t kq_return)
{
    for (size_t i = 0; i < kq_return; i++)
    {
        // std::cout << "0\n";
        SocketConnection *Connection = reinterpret_cast<SocketConnection *>(revents[i].udata);
        if(Connection->IsPortSocket)
        {
            AcceptNewConnections(Connection);
            // std::cout << "1\n";
        }
        else if(!Connection->IsPortSocket)
        {
            HandleEstablishedConnections(Connection, revents[i].filter);
            // std::cout << "2\n";m
        }
    }
    return 0;
}

// int WebServ::is_writable(struct kevent *revents, size_t kq_return, uint64_t socket_fd)
// {
//     for (size_t i = 0; i < kq_return; i++)
//     {
//         if (revents[i].ident == socket_fd)
//         {
//             if (revents[i].filter == EVFILT_WRITE)
//                 return 1;
//         }
//     }
//     return 0;
// }

// void WebServ::drop_clients()
// {
//     std::vector<SocketConnection>::iterator it;
//     std::vector<Connections>::iterator it2;
//     for (it = this->Sockets.begin(); it != this->Sockets.end(); it++)
//     {
//         for (it2 = it->connections.begin(); it2 != it->connections.end();)
//         {
//             if (it2->drop_connection)
//                 it2 = it->connections.erase(it2);
//             else
//                 it2++;
//         }
//     }
// }

void WebServ::AddEvent(int fd, int16_t filter, uint16_t flag)
{
    struct kevent event;
    EV_SET(&event, fd, filter, flag, 0, 0, NULL);
    int err = kevent(this->kq, &event, 1, 0, 0, 0);
    if (err == -1)
        std::cout << "An error occured\n";
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

/* ************************************************************************** */