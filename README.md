# HW1_SPRC

This Project refers to the implementation of client and server components that adhere to the authentication model/paradigm described in the project documentation, namely **OAuth**, using the **RPC** protocol. The entire logic written in the sources started from visualizing the diagrams provided in the resource directory and in the project file, which illustrate, with great ease, the entire design of the architecture to be implemented.

Well, considering that the project was done in C and that a database was desired within the server, I chose to define, for simplicity, a linked list containing nodes with information extracted from the input files. Moreover, there are some global variables in the _auth.h_ file so that these references to the lists of interest can be seen in the context of functions from the _rpc_server.c_ file (I spent some time here to find a way to avoid global variables, but from what I found out, the **RPC** protocol doesn't expose any technique in this regard). The entire defined interface is in the _auth.h_ file, and in _auth.x_ are the structures and functions that serve the conceived server.

I also added newlines in the test files to validate the correctness of the programs.

Implementation details:

First of all, it should be mentioned that I only changed the server stub, namely the _auth_svc.c_ file, because it had to parse the lines from the input files in order to put them in the corresponding linked lists. Of course, no changes were necessary in the client stub, I made sure that all references/dependencies were resolved so that there would be no problems during compilation/link-editing.

An important aspect of the implementation is that I chose to define in the _rpc_client.c_ source a vector of clients that deals with each user separately, based on their IDs obtained from the _client.in_ input file. There is a list that retains the IDs in a unique way (in other words, this list is very similar, from this point of view, to a *set* of IDs). What I wanted from this approach was the _logical ordering_ of the execution flow in the case of operations/requests, in other words I found that those input IDs _are not sorted_ (this affected me on day 2 of implementation due to _REQUEST_ operations interspersed with other operations), and I decided to do a traversal, in that _for_ loop in the client, in an "ordered" style (a client will execute the operation from its structure _only_ when its turn comes, that's why there is that _list traversal_ at each iteration).

Consider this _scenario_ example:

*client.in*:
1. oD0prOgBqAsXBW8, REQUEST, 0
2. oD0prOgBqAsXBW8, MODIFY, Files
3. OVotQBYz418Ozkz, REQUEST, 1
4. OVotQBYz418Ozkz, EXECUTE, Applications
5. OVotQBYz418Ozkz, DELETE, Files
6. oD0prOgBqAsXBW8, INSERT, UserData
7. OVotQBYz418Ozkz, READ, System Settings

There are 2 clients: `client[0].user_id = "oD0prOgBqAsXBW8"` and `client[1].user_id = "OVotQBYz418Ozkz"`.
The pointer that refers to the head of the _names_ list will refer to the string "oD0prOgBqAsXBW8".
This indicates that the first client will execute the first operation from its field. When it reaches number 3, the second client will execute the first operation from its own field. When the pointer reaches 6, client[1] will execute the operation with index 2 (the first operation is at index 0).

Another problem was that the server couldn't modify the fields of the structure sent by _reference_, I chose to return a dynamically allocated string that contained certain helper information for the client so that it could update the respective fields (that's why there are many uses of the _strtok_ function). To mark the client's initiation regarding the remaking of the access token, I decided to "sign" with the number 2 the refresh token so as to transmit to the server the fact that the generation of the new _access_ token is desired using a _refresh_ one.

Regarding the server, the exposed functions are those described in the statement, with a small exception, namely the _valid_permission_ function (which helps to verify if the respective action has the necessary permission on the resource of interest). There are comments meant to clarify any aspect related to the writing logic in the _rpc_server.c_ source.

In the _Makefile_ file there are 2 rules: _run_client_ and _run_server_. In these rules, the index of the test to be executed can be modified.

Used resources:

https://docs-archive.freebsd.org/44doc/psd/22.rpcgen/paper.pdf -> rpcgen manual
