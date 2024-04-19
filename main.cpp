#include <iostream>
#include <sstream>
#include <string>
#include <time.h>

#include "socklib.h"
#include "constants.h"
#include "udp_client.h"
#include "tests.h"

// Use this function in main() to try simple tests.
int udp_assignment() {
  {
    std::cout << "Testing without request ids...\n";
    UDPClient client(consts::HOST, consts::ECHO_PORT);
    std::string result;
    int error = client.send_message_by_character("Beautiful is better than ugly", result);
    if (error == -1) {
      std::cout << "Timed out.\n";
      return -1;
    } else {
      std::cout << "Got result '" << result << "'.\n";
    }
  }

  {
    std::cout << "Testing without request ids...\n";
    UDPClient client(consts::HOST, consts::REQUEST_ID_PORT, true);
    std::string result;
    int error = client.send_message_by_character("To be or not to be, that is the question", result);
    if (error == -1) {
      std::cout << "Timed out.\n";
      return -1;
    } else {
      std::cout << "Got result '" << result << "'.\n";
    }
  }

  return 0;
}

int run_server();
int run_client();

int main(int argc, char *argv[]) {
    SockLibInit();
    atexit(SockLibShutdown);

    if (argc > 1) {
        return run_server();
    }

    return run_client();
}

int run_server() {
    Socket listen_sock(Socket::Family::INET, Socket::Type::STREAM);
    listen_sock.Bind(Address("0.0.0.0", 23500));
    listen_sock.Listen();

    Socket* conn_sock = new Socket();
    listen_sock.AcceptInto(*conn_sock);

    char buffer[4096];
    int nbytes_recvd = conn_sock->Recv(buffer, sizeof(buffer));
    if (nbytes_recvd <= 0) {
        fprintf(stderr, "BLEHHH");
        return -1;
    }
    std::string msg(buffer, nbytes_recvd);
    std::cout << "Received '" << msg << "'\n";
    
    std::string msg_to_send("Wow cool story client");
    conn_sock->Send(msg_to_send.data(), msg_to_send.size());

    return 0;
}

int run_client() {
    Socket sock(Socket::Family::INET, Socket::Type::STREAM);
    sock.Connect(Address("127.0.0.1", 23500));
    std::string msg_to_send("This one time at band camp");
    sock.Send(msg_to_send.data(), msg_to_send.size());

    char buffer[4096];
    int nbytes_recvd = sock.Recv(buffer, sizeof(buffer));
    if (nbytes_recvd <= 0) {
        fprintf(stderr, "BLEHHH");
        return -1;
    }
    std::string msg(buffer, nbytes_recvd);
    std::cout << "Received '" << msg << "'\n";

    return 0;
}