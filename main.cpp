#include <iostream>
#include <sstream>
#include <string>
#include <time.h>

#include "socklib.h"

class UDPClient {
public:
  UDPClient(const char* host, int port, bool include_ids = false):
    server_addr(host, port),
    socket(Socket::Family::INET, Socket::Type::DGRAM),
    include_ids(include_ids)
  {
  }

  int send_message_by_character(const std::string& str, std::string& result)
  {
    for (char c : str) {
      float wait_time = 0.1f;
      while (true) {
	socket.SetTimeout(wait_time);
	int request_id = rand();
	std::string request;
	{
	  std::stringstream request_builder;
	  if (include_ids) {
	    request_builder << request_id << "|";
	  }
	  request_builder << c;
	  request = request_builder.str();
	}
	std::cout << "Sending request " << request << "...\n";
	socket.SendTo(request.data(), request.size(), server_addr);
	char buffer[4096];
	Address from_addr;
	int nbytes_recvd = socket.RecvFrom(buffer, sizeof(buffer), from_addr);
	if (nbytes_recvd == -1 && socket.GetLastError() == Socket::Error::SOCKLIB_ETIMEDOUT) {
	  wait_time *= 2;
	  if (wait_time > 10) {
	    std::cout << "Timed out with wait_time = " << wait_time << ". Giving up.\n";
	    return -1;
	  }
	  std::cout << "Timed out. Retrying with wait_time = " << wait_time << "\n";
	  continue;
	}
	std::string raw_response(buffer, nbytes_recvd);
	std::cout << "Received response " << raw_response << "\n";
	if (include_ids) {
	  std::stringstream ss(raw_response);
	  std::string id;
	  std::string response;
	  std::getline(ss, id, '|');
	  std::getline(ss, response);
	  if (id == std::to_string(request_id)) {
	    result += response;
	    break;
	  }
	} else {
	  result += raw_response;
	  break;
	}
      }
    }
    return 0;
  }

  Address server_addr;
  bool include_ids;
  Socket socket;
};

void do_client();
int do_server();

int udp_assignment() {
  {
    std::cout << "Testing without request ids...\n";
    UDPClient client("68.183.63.165", 9998);
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
    UDPClient client("68.183.63.165", 9999, true);
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

int main(int argc, char *argv[]) {
  SockLibInit();
  srand(time(NULL));
  int result = udp_assignment();
  SockLibShutdown();
  return result;
}

void do_client() {
  Socket send_sock(Socket::Family::INET, Socket::Type::DGRAM);
  
  Address addr("127.0.0.1", 7778);
  Address my_addr("127.0.0.1", 52000);
  send_sock.Bind(my_addr);

  std::string to_send("Hi server!");
  std::cout << "[Client] sending to '" << addr << "'...\n";
  size_t nbytes_sent = send_sock.SendTo(to_send.data(), to_send.size(), addr);

  std::cout << "[Client] sent " << nbytes_sent << " bytes.\n";
}

int do_server() {
  std::cout << "[Server] running...\n";
  Socket conn_sock(Socket::Family::INET, Socket::Type::DGRAM);

  char buffer[4096];

  std::cout << "[Server] receiving...\n";
  Address addr("127.0.0.1", 7778);
  conn_sock.Bind(addr);

  Address from_addr;

  float max_timeout = 5;
  float timeout = 1.2;
  int nbytes_received = 0;
  bool recv_failed = false;
  conn_sock.SetTimeout(timeout);

  time_t before_retry = time(NULL);

  while (timeout < max_timeout) {
    nbytes_received = conn_sock.RecvFrom(buffer, sizeof(buffer), from_addr);
    if (nbytes_received == -1 && conn_sock.GetLastError() == Socket::Error::SOCKLIB_ETIMEDOUT) {
      std::cout << "Timeout occurred.\n";
      timeout *= 2;
      if (timeout >= max_timeout) {
	std::cout << "Recv() failed.\n";
	recv_failed = true;
      } else {
	std::cout << "Retrying with timeout=" << timeout << "...\n";
	conn_sock.SetTimeout(timeout);
      }
    }
  }

  time_t after_retry = time(NULL);
  time_t seconds_passed = after_retry - before_retry;
  if (!recv_failed) {
    std::cout << "Recv succeeded for some reason?\n";
    return 1;
  }
  if (seconds_passed < 7) {
    std::cout << seconds_passed << " seconds spent retrying,"
	      << "but should have been at least 7\n";
    return 1;
  }
  std::cout << seconds_passed << " seconds spent retrying. Success.\n";
  return 0;
  
  std::cout << "[Server] received message '" << std::string(buffer, nbytes_received)
	    << "' from host '" << from_addr << "'\n";
  return 1;
}

std::string build_string(std::istream &in_stream) {
  /* TODO (optional): implement this function */
  return std::string();
}

