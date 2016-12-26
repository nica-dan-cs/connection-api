#include "connection.h"

bool connection_t::is_connected(){
    return am_i_connected;
}

void connection_t::assign_connected_fd(int assigned_fd){
    socket_fd = assigned_fd;
    am_i_connected = true;
}

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

    am_i_connected = true;
    return true;
}
bool connection_t::await_connection_on(unsigned short port){
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

    am_i_connected = true;
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
        if(!connections[i].is_connected()){
            output_buffer[i] = false;
            break;
        }

        if(poll_fds[i].revents == POLLIN)
            output_buffer[i] = true;
        else
            output_buffer[i] = false;
    }

    return true;
}

void* __await_multiple_connections(void* input){
    
    connection_t* connections = ((listen_info_t*)input)->connections;
    unsigned short connections_count = ((listen_info_t*)input)->connections_count;
    unsigned short port = ((listen_info_t*)input)->port;
    
    int listen_socket_fd = socket(AF_INET,SOCK_STREAM,0);
    if(listen_socket_fd == -1){
        std::cerr << "socket() failed\n";
        return nullptr;
    }

    struct sockaddr_in this_side;
    this_side.sin_family = AF_INET;
    this_side.sin_addr.s_addr = INADDR_ANY;
    this_side.sin_port = htons(port);
     
    int bind_status = bind(listen_socket_fd,(struct sockaddr *)&this_side,sizeof(this_side));

    if(bind_status == -1){
        std::cerr << "bind() failed\n";
        return nullptr;
    }

    unsigned short unresolved_connections = connections_count;

    while(unresolved_connections != 0){
        connection_t* unused_connection = connections;
        while(unused_connection->is_connected())
            unused_connection++;

        listen(listen_socket_fd , unresolved_connections);

        socklen_t struct_sizeof = sizeof(struct sockaddr_in);
        int accepted_fd = accept(listen_socket_fd,(struct sockaddr *)&this_side,(socklen_t*)&struct_sizeof);
        unused_connection->assign_connected_fd(accepted_fd);
        unresolved_connections--;
    }

    return nullptr;
}

pthread_t* connection_t::await_multiple_connections(connection_t* connections,unsigned short connections_count,unsigned short port){

    struct listen_info_t *listen_info = new listen_info_t;
    listen_info->connections = connections;
    listen_info->connections_count = connections_count;
    listen_info->port = port;

    pthread_t* listen_thread = new pthread_t;
    int creation_status = pthread_create(listen_thread,NULL,__await_multiple_connections,(void*)listen_info);

    if(creation_status){
        std::cerr << "pthread_create() failed with error code " << creation_status << "\n";
        return nullptr;
    }

    return listen_thread;
}