#pragma once

#include "constants.h"
#include "socklib.h"
#include <sstream>

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
      float wait_time = consts::INITIAL_TIMEOUT;
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
	socket.SendTo(request.data(), request.size(), server_addr);
	char buffer[consts::MAX_BYTES];
	Address from_addr;
	int nbytes_recvd = socket.RecvFrom(buffer, sizeof(buffer), from_addr);
	if (nbytes_recvd == -1 && socket.GetLastError() == Socket::Error::SOCKLIB_ETIMEDOUT) {
	  wait_time *= 2;
	  if (wait_time > consts::MAX_TIMEOUT) {
	    return -1;
	  }
	  continue;
	}
	std::string raw_response(buffer, nbytes_recvd);
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
