#include "connection.h"

bool connection_t::create_socket(int* file_descriptor){
    *file_descriptor = socket(AF_INET,SOCK_STREAM,0);
    if (*file_descriptor == -1){
        std::cerr << "socket() failed\n";
        return false;
    }
    return true;
}

bool connection_t::send(void* buffer,size_t buffer_length){
    int send_status = ::send(socket_fd,buffer,buffer_length,0);
    if(send_status == -1){
        std::cerr << "send() failed\n";
        return false;
    }
    return true;
}
bool connection_t::receive(void** buffer,size_t buffer_length){
    if(*buffer == nullptr)
        *buffer = new unsigned char[buffer_length];
    
    int receive_status = recv(socket_fd,*buffer,buffer_length,0);
    if(receive_status == -1){
        std::cerr << "recv() failed\n";
        return false;
    }
    return true;
}

bool connection_t::connect_to(const char* ip_address,unsigned short port){
    struct sockaddr_in other_side;

    if(!create_socket(&socket_fd))
        return false;

    other_side.sin_addr.s_addr = inet_addr(ip_address);
    other_side.sin_family = AF_INET;
    other_side.sin_port = htons(port);
 
    int connection_status = connect(socket_fd, (struct sockaddr *)&other_side, sizeof(other_side));
    if(connection_status == -1){
        std::cerr << "connect() failed\n";
        return false;
    }
    return true;
}
bool connection_t::listen_on(unsigned short port){
    struct sockaddr_in this_side;

    int liste_socket_fd;
    if(!create_socket(&liste_socket_fd))
        return false;
     
    this_side.sin_family = AF_INET;
    this_side.sin_addr.s_addr = INADDR_ANY;
    this_side.sin_port = htons(port);
     
    int bind_status = bind(liste_socket_fd, (struct sockaddr *)&this_side, sizeof(this_side));
    if(bind_status == -1){
        std::cerr << "bind() failed\n";
        return false;
    }
     
    listen(liste_socket_fd,1);

    socklen_t struct_sizeof = sizeof(struct sockaddr_in);
    socket_fd = accept(liste_socket_fd, (struct sockaddr *)&this_side, &struct_sizeof);
    if (socket_fd == -1){
        std::cerr << "accept() failed\n";
        return false;
    }

    return true;
}

template <class struct_type>
void connection_t::send_as(struct_type* input){
    void* send_buffer = new unsigned char[sizeof(struct_type)];
    
    memcpy(send_buffer,input,sizeof(struct_type));
    this->send(send_buffer,sizeof(struct_type));

    delete send_buffer;
}

template <class struct_type>
void connection_t::receive_as(struct_type** output_buffer){
    if(*output_buffer == nullptr)
        output_buffer = new struct_type;

    void* receive_buffer = new unsigned char[sizeof(struct_type)];

    receive(&receive_buffer,sizeof(struct_type));
    memcpy(*output_buffer,receive_buffer,sizeof(struct_type));

    delete receive_buffer;
}

bool connection_t::poll(connection_t** connections,unsigned short connections_count,unsigned short timeout_useconds,bool* output_buffer){
    struct pollfd* poll_fds = new pollfd[connections_count];

    for(int i=0; i < connections_count; i++){
        poll_fds[i].fd = connections[i]->socket_fd;
        poll_fds[i].events = 0;
        poll_fds[i].events |= POLLIN;
        poll_fds[i].revents = 0;
    }

    int poll_status = ::poll(poll_fds,connections_count,timeout_useconds);

    if(poll_status == -1){
        std::cerr << "poll() failed\n";
        return false;
    }

    for(int i=0; i< connections_count; i++){
        if(poll_fds[i].revents == POLLIN)
            output_buffer[i] = true;
        else
            output_buffer[i] = false;
    }

    return true;
}

bool connection_t::poll(connection_t* connections,unsigned short connections_count,unsigned short timeout_useconds,bool* output_buffer){
    struct pollfd* poll_fds = new pollfd[connections_count];

    for(int i=0; i < connections_count; i++){
        poll_fds[i].fd = connections[i].socket_fd;
        poll_fds[i].events = 0;
        poll_fds[i].events |= POLLIN;
        poll_fds[i].revents = 0;
    }

    int poll_status = ::poll(poll_fds,connections_count,timeout_useconds);

    if(poll_status == -1){
        std::cerr << "poll() failed\n";
        return false;
    }

    for(int i=0; i< connections_count; i++){
        if(poll_fds[i].revents == POLLIN)
            output_buffer[i] = true;
        else
            output_buffer[i] = false;
    }

    return true;
}