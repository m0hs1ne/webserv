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
        struct kevent revents[MAX_INT];
        kq_return = kevent(this->kq, 0, 0, revents, MAX_INT, &timeout);
        if (kq_return < 0)
        {
            perror("Kevent Failed");
            exit(EXIT_FAILURE);
        }
        if (kq_return == 0)
        {
            std::cout << "timed out\n"
                      << std::endl;
            continue;
        }
        // std::cout << " Kq-> " << kq_return << std::endl;
        CheckEvents(revents, kq_return);
    }
}

void WebServ::SetUpSockets()
{
    int valread = 1;
    for (size_t i = 0; i < servers.size(); i++)
    {
        SocketConnection *Socket = new SocketConnection;
        struct kevent event;
        if (!(Socket->socket_fd = socket(AF_INET, SOCK_STREAM, 0)))
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(Socket->socket_fd, SOL_SOCKET, SO_REUSEADDR, &valread, sizeof(valread)))
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
        if (listen(Socket->socket_fd, 128) < 0)
        {
            perror("listen Failed");
            exit(EXIT_FAILURE);
        }
        if(fcntl(Socket->socket_fd, F_SETFL, O_NONBLOCK) < 0)
        {
            perror("fcntl failed");
            exit(EXIT_FAILURE);
        }
        std::cout << "Listening on port " << servers[i].port << std::endl;
        EV_SET(&event, Socket->socket_fd, EVFILT_READ, EV_ADD, NOTE_TRIGGER, 0, reinterpret_cast<void *>(Socket));
        kevent(this->kq, &event, 1, 0, 0, 0);
    }
}

int WebServ::AcceptNewConnections(SocketConnection *Socket)
{
    SocketConnection *new_Socket = new SocketConnection;

    socklen_t len = sizeof(Socket->addr);
    new_Socket->drop_connection = 0;
    new_Socket->data_s = 0;
    new_Socket->content_size = 0;
    new_Socket->IsPortSocket = 0;
    new_Socket->server = Socket->server;
    new_Socket->socket_fd = accept(Socket->socket_fd, (sockaddr *)&Socket->addr, &len);
    if(new_Socket->socket_fd < 0)
    {
        perror("Accept Failed");
        exit(EXIT_FAILURE);
    }
    if(fcntl(new_Socket->socket_fd, F_SETFL, O_NONBLOCK) < 0)
    {
        perror("fcntl fail");
        exit(EXIT_FAILURE);
    }
    AddEvent(new_Socket->socket_fd, EVFILT_READ, new_Socket);
    return 1;
}

size_t fullsize = 0;

void WebServ::HandleEstablishedConnections(SocketConnection *Connection, int16_t filter)
{

    if (filter == EVFILT_READ)
        Reciev(Connection);
    else if (filter == EVFILT_WRITE)
        Send(Connection);
    else
    {
        std::cout << "Error" << std::endl;
        exit(1);
    }
}

int WebServ::CheckEvents(struct kevent *revents, size_t kq_return)
{
    for (size_t i = 0; i < kq_return; i++)
    {
        SocketConnection *Connection = reinterpret_cast<SocketConnection *>(revents[i].udata);
        if (Connection->IsPortSocket)
            AcceptNewConnections(Connection);
        else if (!Connection->IsPortSocket)
            HandleEstablishedConnections(Connection, revents[i].filter);
    }
    return 0;
}

void WebServ::Reciev(SocketConnection *Connection)
{
    int ret = 0;
    char buffer[1025] = {0};
    std::map<int, std::string> *code = new std::map<int, std::string>;
    initHttpCode(*code);

    ret = read(Connection->socket_fd, buffer, 1024);
    if (ret == 0)
    {
        DeleteEvent(Connection->socket_fd, EVFILT_READ);
        close(Connection->socket_fd);
        delete Connection;
        std::cout << "here" << std::endl;
        // AddEvent(Connection->socket_fd, EVFILT_WRITE, Connection);
        return;
    }
    else if(ret < 0)
        exit(0);
    buffer[ret] = '\0';
    Connection->request.buffer_size = ret;
    if (Connection->request.method.empty() || Connection->request.bFd != -2 || Connection->request.openedFd != -2)
    {
        Connection->response.codeMsg = code;
        Connection->response = Connection->request.handleRequest(ft_strdup(buffer, ret), *(Connection->server));
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
    else
        Connection->ended = true;
    if (Connection->ended)
    {
        Connection->response.formResponse(Connection->request, *(Connection->server));
        //std::cout << Connection->response.response + Connection->response.body << std::endl;
        DeleteEvent(Connection->socket_fd, EVFILT_READ);
        AddEvent(Connection->socket_fd, EVFILT_WRITE, Connection);
    }
    delete code;
}

void WebServ::Send(SocketConnection *Connection)
{
    char buffer[1025] = {0};
    int _return;

    if (!Connection->response.response.empty())
    {
        if (0 > write(Connection->socket_fd, Connection->response.response.c_str(), Connection->response.response.size()))
        {
            DeleteEvent(Connection->socket_fd, EVFILT_WRITE);
            close(Connection->socket_fd);
            if(Connection->response.fileFD != -2)
                close(Connection->response.fileFD);
            delete Connection;
            return ;
        }
        Connection->response.response.clear();
    }
    else if (Connection->response.fileFD != -2)
    {
        _return = read(Connection->response.fileFD, buffer, 1024);
        if (_return > 0)
        {
            if (0 > write(Connection->socket_fd, buffer, _return))
            {
                DeleteEvent(Connection->socket_fd, EVFILT_WRITE);
                close(Connection->socket_fd);
                delete Connection;
                return ;
            }
        }
        else
        {
            DeleteEvent(Connection->socket_fd, EVFILT_WRITE);
            close(Connection->response.fileFD);
            close(Connection->socket_fd);
            delete Connection;
        }
    }
    else
    {
        write(Connection->socket_fd, Connection->response.body.c_str(), Connection->response.body.size());
        Connection->response.body.clear();
        DeleteEvent(Connection->socket_fd, EVFILT_WRITE);
        close(Connection->socket_fd);
        delete Connection;
        return ;
    }
}



void WebServ::AddEvent(int fd, int16_t filter, SocketConnection *udata)
{
    struct kevent event;
    EV_SET(&event, fd, filter, EV_ADD, 0, 0, reinterpret_cast<void *>(udata));
    int err = kevent(this->kq, &event, 1, 0, 0, 0);
    // std::cout << "err =>" << err << std::endl;
    if(err == -1)
    {
        perror("kevent Add Error");
        exit(EXIT_FAILURE);
    }
}


void WebServ::DeleteEvent(int fd, int16_t filter)
{
    struct kevent event;
    EV_SET(&event, fd, filter, EV_DELETE, 0, 0, NULL);
    int err = kevent(this->kq, &event, 1, 0, 0, 0);
    // std::cout << "err =>" << err << std::endl;
    if(err == -1)
    {
        perror("kevent Delete Error");
        exit(EXIT_FAILURE);
    }
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

/* ************************************************************************** */