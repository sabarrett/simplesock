#include <iostream>
#include <string.h>
#include "constants.h"
#include "udp_client.h"

#define LEN(x) sizeof(x) / sizeof(*x)

static const char *const test_cases[] = {"hello world", "abcdefghijklmnop",
                                         "Beautiful is better than ugly."};

static bool test_timeout(UDPClient& client) {
  int result = 0;
  std::string msg;

  // If solution doesn't implement timeout, this will loop forever
  // until github times out and fails.
  while (client.send_message_by_character("%", msg) != -1) {}
  std::cout << "PASS: "
	    << "Properly returned -1 when socket times out.\n";
  return true;
}

static bool improper_port_test(UDPClient& client) {
  int result;
  std::string msg;
  result = client.send_message_by_character(test_cases[0], msg);
  if (result != -1) {
    std::cout << "FAIL: "
	      << "No error came through when there was no route to host.\n";
    return false;
  }

  std::cout << "PASS: "
	    << "Properly let socket error through when no route to host.\n";
  return true;
}

static bool keep_retrying(UDPClient& client,
			  const char* msg, std::string& response) {
  for (int i = 0; i < 10; i++) {
    if (client.send_message_by_character(msg, response) == 0) {
      return true;
    }
  }
  return false;
}

class SrandTester
{
public:
  SrandTester()
  {
    srand(1);
    rand_val = rand();
    srand(1);
  }

  static int rand_val;
} __rand_tester__;

static bool srand_called = false;
int SrandTester::rand_val;

static bool test_q1() {
  bool passing_all = true;

  {
    UDPClient client(consts::HOST, consts::BAD_PORT);
    std::cout << "Testing improper port...\n";
    passing_all = passing_all && improper_port_test(client);
  }

  UDPClient client(consts::HOST, consts::ECHO_PORT);
  std::cout << "Testing timeout...\n";
  passing_all = passing_all && test_timeout(client);

  for (int i = 0; i < LEN(test_cases); i++) {
    const char* test = test_cases[i];
    size_t test_strlen = strlen(test);
    std::cout << "Testing '" << test << "'...\n";
    std::string response;
    bool got_resp = keep_retrying(client, test, response);
    if (!got_resp) {
      std::cout << "FAIL: " << "Timed out after 10 tries, giving up.\n";
      passing_all = false;
    } else if (response.size() == test_strlen && response != test) {
      std::cout << "PASS: " << "'" << test << "' properly sent, received '"
		<< response << "'.\n";
    } else if (response.size() == test_strlen) {
      std::cout << "FAIL: " << "'" << test
		<< "' should not have been received exactly.\n";
      passing_all = false;
    } else {
      std::cout << "FAIL: " << "Incorrect number of character returned "
		<< "for '" << test << "'. Should have received "
		<< test_strlen << ", but received " << response.size()
		<< ". Received '" << response << "' as response.\n";
      passing_all = false;
    }
  }
  
  return passing_all;
}

static bool test_q2() {
  bool passing_all = true;

  std::cout << "Testing if srand() was called...\n";

  if (!srand_called) {
    std::cout << "FAIL: random number generator is not seeded.\n";
    passing_all = false;
  } else {
    std::cout << "PASS: seeded random number generator.\n";
    passing_all = passing_all && true;
  }


  {
    UDPClient client(consts::HOST, consts::BAD_PORT);
    std::cout << "Testing improper port...\n";
    passing_all = passing_all && improper_port_test(client);
  }

  UDPClient client(consts::HOST, consts::REQUEST_ID_PORT, true);
  for (int i = 0; i < LEN(test_cases); i++) {
    const char* test = test_cases[i];
    std::cout << "Testing '" << test << "'...\n";
    std::string response;
    bool got_resp = keep_retrying(client, test, response);
    if (!got_resp) {
      std::cout << "FAIL: " << "Timed out after 10 tries, giving up.\n";
      passing_all = false;
    } else if (response == test) {
      std::cout << "PASS: " << "'" << test << "' properly sent, received '"
		<< response << "'.\n";
    } else {
      std::cout << "FAIL: " << "Incorrect string returned "
		<< "for '" << test << "'. Should have received '"
		<< test << "', but received '" << response << "'.\n";

      passing_all = false;
    }
  }

  return passing_all;
}

static struct {
    bool (*test)();
    const char* name;
    int value;
} tests[] = {
  {test_q1, "test_q1", 20},
  {test_q2, "test_q2", 20}
};

static bool tests_passed[sizeof(tests) / sizeof(tests[0])] = {false};

int run_all_tests() {
  int sum = 0;
  int total_possible = 0;
  bool passed_all_tests = true;
  srand_called = rand() != SrandTester::rand_val;
  for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
    {
      auto test = tests[i];
      total_possible += test.value;
      std::cout << test.name << "\n------------\n";
      if (test.test())
      {
        sum += test.value;
        tests_passed[i] = true;
        std::cout << "Test passed\n\n";
      }
      else
      {
        std::cout << "Test failed\n\n";
        passed_all_tests = false;
      }
    }

  std::cout << "\n***  Scores  ***\n------------\n";
  for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); i++)
    {
      auto test = tests[i];
      if (tests_passed[i])
      {
        std::cout << test.name << ": " << test.value << "/" << test.value << std::endl;
      }
      else
      {
        std::cout << test.name << ": " << 0 << "/" << test.value << std::endl;
      }
    }

  std::cout << "=====\nFinal score: " << sum << "/" << total_possible << std::endl;
  return passed_all_tests ? 0 : 1;
}
