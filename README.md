# Programming Assignment 3 -- TCP Clients

The purpose of this assignment is to help you practice creating
sockets, opening connections, and formatting, sending, and receiving
data. You will create a client program that communicates with a server
via a socket connection and displays the results sent back from the
server.

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
2. Implement the `do_client()` function. This will do several things:
   1. Create a socket, and connect to the server at **IP ADDRESS
	  68.183.63.165** and **PORT 7778**.
   2. Create a string to send to the server using the `build_string()`
      function (described below).
   3. Send a properly-formatted sort string to the server.
   4. Receive the server's response.
   5. Create a `std::string` from the server's response and return it.
3. Implement the `build_string()`[^1] function. This will do several
   things:
   1. Get a line from the given `istream`.
   2. Take a line of input from the given `istream` and parse it into
      an integer (if it is an integer) or a float (if it is a float).
   3. Add the parsed number to a string, which will ultimately be sent
      to the server.
   4. When the user enters the string `done`, the string is complete
      and should be returned.

The server expects a string of the format "LIST n1 n2 ... nn", where
ni is the ith number of the list to be sorted.

| **If you send...**  | **The server will return...**        |
|:-------------------:|:------------------------------------:|
| LIST 5 4 3 2 1      | SORTED 1 2 3 4 5                     |
| LIST 4.2 7.8 -9 404 | SORTED -9 4.2 404 7.8                |
| 4.2 7.8 -9 404      | ERROR: LIST prefix not present.      |
| LIST                | ERROR: No elements provided to sort. |
| LIST 1 2 3 4 x      | ERROR: Non-numeric input provided    |
| LIST 127a 0 2 4     | ERROR: Non-numeric input provided    |

## Grading

- The socket library is correctly initialized and shutdown (3 points)
- Code connects to the server via a socket (4 points)
- Code sends input to the server using the proper format and encoding
  (8 points)
- Code accepts, decodes, and prints responses from the server (5
  point)
- `build_string()` logic is correctly implemented (regardless of
  whether it is an independent function or is simply inlined). (5
  points)

**Note:** Once you have the above bullet points correct, the test
that occurs when the program runs will pass.

- Code is formatted well (consistently indented, clear variable names,
  etc.) (5 points)

**Total: 30 points.**

[^1]: Note that the exact details of this function aren't important -- for example, it doesn't matter if build_string() returns a string with "LIST" prepended or if that is done by do_client(). In fact, it's not important that you implement a discrete build_string() function at all. However, this logic must occur somehwere in your program.
