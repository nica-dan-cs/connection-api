#include "connection.cpp"
#include <string.h>

/*Await for 2 connections on port 1111 and display everything that is received from these connections*/

int main(int argc , char *argv[]){
    connection_t* connections = new connection_t[2];

    /*Populate the connections array as connections are established on port 1111*/
    pthread_t* listen_thread = connection_t::await_multiple_connections(connections,2,1111);

    /*poll() will change this as data is available to be read*/
    bool status[2] = {false};

    void* receive_buffer = new unsigned char[100];

    while(true){
        connection_t::poll(connections,3,8,&status[0]);

        for(int i=0; i < 2; i++){
            if(status[i] == true){
                connections[i].receive(&receive_buffer,100);
                std::cout << "Received this from " << i << ": " << (char*)receive_buffer << "\n";
            }
        }
    }

    /*Remember to join the listen_thread*/
    pthread_join(*listen_thread,NULL);
}