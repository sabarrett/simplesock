#pragma once

#include "constants.h"
#include "socklib.h"
#include <sstream>

class UDPClient {
public:
  UDPClient(const char* host, int port, bool include_ids = false)
  {
    /* TODO: IMPLEMENT THIS */
  }

  int send_message_by_character(const std::string& str, std::string& result)
  {
    /* TODO: IMPLEMENT THIS */
    return -1;
  }
};
