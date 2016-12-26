#include "connection.cpp"
#include <string>
#include <string.h>
#include <stdio.h>

/*Connect to 127.0.0.1 on port 1111 and send whatever is typed to stdin
If spaces are present, the line will be sent in multiple calls to send(). It is because of std::cin, but 
is also unimportant*/

int main(int argc , char *argv[]){
    
    connection_t connection;
    connection.connect_to("127.0.0.1",1111);

    char* send_buffer = new char[100];

    while(true){
        std::cin >> send_buffer;
        connection.send((void*)send_buffer,(size_t)100);
    }

}