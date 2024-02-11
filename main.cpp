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

int main(int argc, char *argv[]) {
  /* TODO: SET UP / TEAR DOWN SOCKET LIBRARY HERE; SEED RANDOM
     NUMBERS */
  int result = run_all_tests();

  return result;
}
