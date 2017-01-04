#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>

class connection_t{
    private:
        int socket_fd = -1;
        bool am_i_connected = false;
        bool create_socket(int* file_descriptor);

    public:
        ~connection_t();

        bool is_connected();
        void assign_connected_fd(int assigned_fd);

        bool send(void* buffer,size_t buffer_length);
        bool receive(void** buffer,size_t buffer_length);

        bool connect_to(const char* ip_address,unsigned short port);
        bool await_connection_on(unsigned short port);

        template <class struct_type>
        void send_as(struct_type* input);
        template <class struct_type>
        void receive_as(struct_type** output_buffer);

        static bool poll(connection_t* connections,unsigned short connections_count,unsigned short timeout_useconds,bool* output_buffer);
        static pthread_t* await_multiple_connections(connection_t* connections,unsigned short connections_count,unsigned short port);
};

struct listen_info_t{
    connection_t* connections;
    unsigned short connections_count;
    unsigned short port;
};