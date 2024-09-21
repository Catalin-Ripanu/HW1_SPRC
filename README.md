# HW1 SPRC Report

## Project Overview

This project involves implementing client and server components that adhere to the **OAuth** authentication model using the **RPC** protocol. The implementation is based on diagrams provided in the resource directory and project file.

## Implementation Details

### General Approach

- Implemented in C
- Server uses a linked list as a simple database
- Global variables defined in `auth.h` for access in `rpc_server.c`
- Interface defined in `auth.h`
- Structures and functions for the server defined in `auth.x`

### Server Implementation

- Only modified the server stub (`auth_svc.c`)
- Parses input files to populate linked lists
- Uses global variables for list references (due to RPC protocol limitations)

### Client Implementation

- Defined a vector of clients in `rpc_client.c`
- Each client handles a user based on IDs from `client.in`
- Maintains a unique list of IDs (similar to a set)
- Implements logical ordering of execution flow for operations/requests

#### Execution Scenario Example

```
client.in:
1. oD0prOgBqAsXBW8, REQUEST, 0
2. oD0prOgBqAsXBW8, MODIFY, Files
3. OVotQBYz418Ozkz, REQUEST, 1
4. OVotQBYz418Ozkz, EXECUTE, Applications
5. OVotQBYz418Ozkz, DELETE, Files
6. oD0prOgBqAsXBW8, INSERT, UserData
7. OVotQBYz418Ozkz, READ, System Settings
```

- Two clients: `client[0].user_id = "oD0prOgBqAsXBW8"` and `client[1].user_id = "OVotQBYz418Ozkz"`
- Operations executed in order based on client ID

### Server-Client Communication

- Server returns dynamically allocated strings with helper information
- Uses `strtok` function extensively
- Refresh token "signed" with number 2 to indicate access token regeneration

### Server Functions

- Implements functions as described in the project statement
- Additional `valid_permission` function for action verification

## File Structure

- `auth.h`: Interface definition and global variables
- `auth.x`: Structures and functions for the server
- `rpc_server.c`: Server implementation
- `rpc_client.c`: Client implementation
- `auth_svc.c`: Modified server stub

## Makefile

- Contains two rules: `run_client` and `run_server`
- Test index can be modified in these rules

## Resources Used

- [rpcgen manual](https://docs-archive.freebsd.org/44doc/psd/22.rpcgen/paper.pdf)

## Notes

- Newlines added to test files for validation
- Implementation focused on logical ordering of execution flow
- Server unable to modify structure fields sent by reference; workaround implemented
