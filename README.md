# Blackjack Client–Server (TCP Sockets)

A networked multiplayer Blackjack game implemented in C/C++ using a TCP client–server architecture.

This project was developed as part of a university networking assignment and focuses on socket programming, concurrent client management, and application-level protocol design.

The application allows multiple users to connect to a server, authenticate or register, and play Blackjack matches through a console interface.

---

# Project Overview

This project implements a simplified version of Blackjack played over a TCP connection.

Players connect to a central server using a client application. The server manages:

- User registration and authentication
- Connected clients
- Matchmaking
- Game state and turn control
- Communication between players
- Concurrent client handling

Only two-player matches are allowed.

The server supports multiple simultaneous games and manages all game logic centrally.

---

# Technologies Used

- C / C++
- TCP sockets
- Client–server architecture
- Linux
- Concurrent server management (`select()`)
- Console-based communication

---

# Game Rules

This project implements a restricted version of Blackjack.

Rules:

- Standard 52-card deck
- Cards from 2–10 keep their value
- J, Q and K count as 10 points
- Ace can count as 1 or 11
- Maximum of 2 players per match

Gameplay:

- Each player receives two initial cards
- Players may request additional cards
- Players may stop at any moment
- If a player exceeds 21 points, they lose automatically
- The player closest to 21 wins
- Equal score results in a draw

---

# Client Commands

The game is controlled entirely through commands sent from the client.

## Register a New User

```text
REGISTRO -u username -p password
```

Example:

```text
REGISTRO -u player1 -p 1234
```

Creates a new user if the username does not already exist.

---

## Login

Send username:

```text
USUARIO username
```

Example:

```text
USUARIO player1
```

Then send password:

```text
PASSWORD password
```

Example:

```text
PASSWORD 1234
```

---

## Start a Match

```text
INICIAR-PARTIDA
```

Behaviour:

- If another player is waiting → the game starts
- Otherwise → the player waits until another player joins

---

## Gameplay Commands

Request a new card:

```text
PEDIR-CARTA
```

Stop playing and keep current hand:

```text
PLANTARME
```

---

## Exit

```text
SALIR
```

Disconnects the player from the server.

If the player leaves during a match, the opponent is notified.

---

# Example Users

Example users are included in:

```text
usuarios.txt
```

The included accounts are generic test users with simple passwords intended only for demonstration purposes.

Additional users can be created directly from the client using:

```text
REGISTRO -u username -p password
```

User credentials are intentionally stored in plain text as part of the assignment requirements.

---

# Compilation

Compile the server:

```bash
g++ servidor.cc -o servidor
```

Compile the client:

```bash
g++ cliente.cc -o cliente
```

---

# Execution

Start the server:

```bash
./servidor
```

Open another terminal and start a client:

```bash
./cliente
```

Open additional clients to simulate multiplayer sessions.

---

# Project Structure

```text
cliente.cc      Client implementation
servidor.cc     Server implementation
juego.h         Game logic and structures
usuarios.txt    Registered/test users
README.md       Documentation
```

---

# Learning Objectives

This project was developed to practice:

- TCP socket programming
- Connection-oriented communication
- Concurrent server design
- Application protocol implementation
- Multiplayer session management
- Client–server software architecture

---

# Academic Context

This repository contains the implementation of the Blackjack game developed for a university networking assignment.

The assignment provided the project requirements and server architecture guidelines; the game behaviour, protocol implementation, user management and gameplay functionality were developed as part of the project work.
