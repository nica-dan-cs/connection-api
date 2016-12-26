#### Documentation

 1. `is_connected()` returns true if the connection is established


     
2. `connect_to` and `await_connection_on` attempt to establish a single connection and return true upon success.


3. `send_as` and `receive_as` are templated functions that operate on any data structure.


4. `receive` and `receive_as` place the received data at the address pointed to by `*buffer` and `*output_buffer` respectively. If this value is `NULL`, the function will allocate the required memory for you. The allocation is done using `new`, so remember to use `delete` and NOT `free()`.


5. `poll` checks for incomming data over an array of connections. If nothing happens in `timeout_useconds`, the function returns. If there is waiting data to be read on `connections[i]` (meaning a `receive` or `receive_as` call would not block), then `output_buffer[i]` will be `true`, and `false` otherwise.


6. `await_multiple_connections` creates a thread that listens for a specified number of incoming connections on a given port. The connections are established by the thread as they become available, so you must remember to check `is_connected()` before performing any operation. `poll` is safe, since it ignores unestablished connections.


7. Remember to tell `g++` to link with the pthread library: `-lpthread`