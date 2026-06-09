# Multiplayer Client-Server Tic-Tac-Toe
## Overview

A networked multiplayer Tic-Tac-Toe game developed in C using a client-server architecture. The server manages game state, validates moves, coordinates communication between players, and supports multiple simultaneous games through multithreading.

Developed for CSCI 3240 – Intro to Computer Systems at Middle Tennessee State University.

## Features

- Multiplayer gameplay over a network
- Client-server architecture
- Player matchmaking
- Multithreaded server
- Move validation
- Turn management
- Win detection
- Draw detection
- Board state synchronization

## Technologies

- C
- POSIX Sockets
- Pthreads
- Linux
 
## Architecture

### Server Responsibilities

- Accept client connections
- Match players into games
- Maintain authoritative game state
- Validate moves
- Manage turns
- Detect wins and draws
- Broadcast board updates
 
### Client Responsibilities

- Connect to the server
- Display the game board
- Accept player input
- Send moves to the server
- Display game updates and results

  The server acts as the authoritative source of truth for the game state. Clients send move requests and receive validated game updates from the server.
 
## Threading Model

The server uses a multithreaded architecture.

- A main server thread accepts incoming client connections.
- A client handler thread manages matchmaking.
- A dedicated game thread is created for each active game.
 
A mutex protects shared matchmaking data to prevent race conditions when multiple clients connect simultaneously.

## Project Structure

server.c     - Server implementation

client.c     - Client implementation

Makefile     - Build automation

## How to Build

```bash
make
```

or

```bash
gcc server.c -o server -lpthread
gcc client.c -o client
```

## How to Run

Start the server:
```bash
./server <port>
```

Start two clients:
```bash
./client <server_ip> <port>
```


## Concepts Demonstrated

- Client-server networking
- Socket programming
- Concurrent programming
- Thread synchronization
- Shared state management
- Systems programming
 
## Technical Challenges

- Designing a communication protocol between clients and the server
- Managing concurrent client connections using threads
- Synchronizing shared matchmaking data safely
- Maintaining a consistent game state across both clients

 
## Future Improvements

- Support player reconnection
- Add configurable board sizes
- Add a graphical user interface
- Improve matchmaking and session management
  
