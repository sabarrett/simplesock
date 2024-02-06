#include <iostream>
#include <sstream>
#include <string>

#include "socklib.h"

std::string do_client(std::istream &in_stream);
int run_test();

int main(int argc, char *argv[]) {
  /* TODO: ADD SETUP CODE HERE */

  return run_test();
}

std::string do_client(std::istream &in_stream) {
  /* TODO: ADD FUNCTION IMPLEMENTATION HERE */
  return std::string();
}

std::string build_string(std::istream &in_stream) {
  /* TODO (optional): implement this function */
  return std::string();
}

int run_test() {
  std::stringstream input("30\n500\n-12\n3.6\n"
			  "not a number\n200also not a number\n"
			  "88.1\ndone");

  std::cout << "Testing on input '" << input.str() << "'.\n";

  std::string msg = do_client(input);
  std::string expected("SORTED -12 3.6 30 500 88.1");
  std::cout << "do_client() returned '" << msg << "'.\n";

  if (msg != expected) {
    std::cout << "TEST FAILED. Expected '" << expected << "' but was '" << msg
              << "'.\n";
    return 1;
  }
  
  std::cout << "Recieved expected message '" << msg << "'. Test passed 😄\n";
  return 0;
}
