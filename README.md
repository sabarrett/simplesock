# Programming Assignment 4 -- UDP Sockets

The purpose of this assignment is to help you practice creating UDP
sockets and dealing with the various shortcomings of UDP. You will
create a client program that communicates with a server via several
UDP sends and receives.

As always, to begin this assignment:
- Create a new repository from this template repository.
- Make your new repository private and owned by the CC-GPR-430
  organization.
- You can check to ensure that your assignment is working as expected
  by observing the workflow results on GitHub.
  - As always, you only need the tests to pass on one platform. It
    doesn't matter which.
- When you're ready to submit, simply submit a link to your
  repository.


## Assignment Details

1. Add whatever global setup and teardown is necessary for the sockets
   library at the beginning/end of the `main()` function.
2. Seed the random number generator in `main()` using `srand()`.
3. Create the `UDPClient` class as described below. You may do this
   directly in `udp_client.h`, or you may make a new `udp_client.cpp`
   file, which will need to be properly added to `CMakeLists.txt`.

### Q1
Implement the `UDPClient` class. This has two methods:

#### UDPClient Constructor
This takes two arguments: 
- `const char* host`: the host to which the client will send messages.
- `int port`: the UDP port number to send messages to.

#### `send_message_by_character`

This method has the following signature:

```c++
int send_message_by_character(const std::string& str, std::string& result)
```

This sends `str` to the server one character at a time, awaiting a
response after each transmission (aka sending a character).

The client must use **truncated exponential backoff**. **For each
transmission** (each individual character it is sending) it should
begin with an initial timeout of `INITIAL_TIMEOUT` (see contants.h) and keep
doubling the timeout and re-sending the character, until a response is
received or the `MAX_TIMEOUT` is reached.

If `MAX_TIMEOUT` is reached, return `-1`. Otherwise, this method must
**set `result` to the value of all of the received characters in the
order they were received.** The value of `result` may be undefined if
`-1` is returned.

You should see that some characters get repeated and some are missing
due to the unreliableness of UDP.

### Q2

Incorporate the optional third `bool include_ids` argument in the
constructor so that your class keeps track of it for its lifetime. If
this argument is set to `true`, `send_message_by_character` should
have its behavior changed as described below. If the parameter is
`false`, your class should retain its behavior from Q1.

Update the `send_message_by_character` method so that its behavior
depends on whether request IDs are being included or not. If they are
not, nothing different from Q1 should happen, but if they are then:

Each message should be of the form `REQUEST_ID|character` where
`REQUEST_ID` is an integer randomly chosen using the `rand()`
function, and `character` is the character you are sending. Note that
the pipe symbol `|` is used to separate the `REQUEST_ID` from the
data. Responses from the server will follow the same format. The code
should keep attempting to receive responses until one with the
matching ID is received. Any response with an incorrect ID should be
discarded, but responses with the correct ID should be concatenated
together into a string that this function sets `result` to. With this
protection in place you should see that this function is able to
receive an exact copy of the provided message, meaning that it has
guaranteed all responses are received in the correct order.

As before, this function should return `-1` if `MAX_TIMEOUT` is tried
and reached for any single transmission, and `0` if it completes
successfully. The value of `result` may be undefined if `-1` is
returned.

## Grading

- Q1: 20 points
- Q2: 20 points
- Submission and code style: 10 points

**Total: 50 points.**
