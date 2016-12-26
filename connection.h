#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

class connection_t{
    private:
        int socket_fd;
        bool create_socket(int* file_descriptor);

    public:

        bool send(void* buffer,size_t buffer_length);
        bool receive(void** buffer,size_t buffer_length);

        bool connect_to(const char* ip_address,unsigned short port);
        bool listen_on(unsigned short port);

        template <class struct_type>
        void send_as(struct_type* input);
        template <class struct_type>
        void receive_as(struct_type** output_buffer);

        static bool poll(connection_t** connections,unsigned short connections_count,unsigned short timeout_useconds,bool* output_buffer);
        static bool poll(connection_t* connections,unsigned short connections_count,unsigned short timeout_useconds,bool* output_buffer);
};