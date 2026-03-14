# ARCHITECTURE — Hybrid MPI + OpenMP Pong

## High-Level Diagram

```
┌────────────────────────────────────────────────────────────────┐
│                          MPI WORLD                              │
├────────────────────────────────┬───────────────────────────────┤
│         RANK 0                 │         RANK 1                │
│    (Render + Authority)        │    (Physics Worker)           │
├────────────────────────────────┼───────────────────────────────┤
│                                │                               │
│  • Creates SFML window         │  • No rendering               │
│  • Renders all game objects    │  • Updates ball states        │
│  • Authoritative for:          │  • OpenMP: parallelize        │
│    - Score tracking            │    per-ball simulation        │
│    - Hit totals                │  • Sends events to Rank 0:    │
│    - Game timer                │    - Ball positions           │
│    - Game end logic            │    - Hit events               │
│    - Respawn decisions         │    - Score events             │
│  • Receives physics updates    │                               │
│    from Rank 1                 │  • Receives paddle positions  │
│  • Sends paddle positions      │    from Rank 0                │
│    to Rank 1                   │                               │
│                                │                               │
└────────────────────────────────┴───────────────────────────────┘
```

## Rank Responsibilities

### Rank 0: Render + Authority
**Purpose**: The "game server" that maintains authoritative state and renders the game.

**Responsibilities**:
- Create and manage the SFML window
- Handle input events
- Render all game objects (paddles, balls, scores, UI)
- Maintain authoritative state:
  - Player scores
  - Hit totals (p1HitsTotal, p2HitsTotal)
  - Game timer (60 seconds countdown)
  - Game end conditions
- Make respawn decisions when balls score
- Send paddle positions to Rank 1 each frame
- Receive physics updates from Rank 1:
  - Ball positions and velocities
  - Hit events (which paddle was hit)
  - Score events (which side scored)

**Communication Pattern**:
- **Send to Rank 1**: Paddle positions every frame
- **Receive from Rank 1**: Ball states + events every frame

### Rank 1: Physics Worker
**Purpose**: Dedicated physics simulation with OpenMP parallelization.

**Responsibilities**:
- Simulate all ball movements (no rendering)
- Check collisions:
  - Ball vs. wall (top/bottom)
  - Ball vs. paddle (left/right)
  - Ball vs. goal zones
- Emit events to Rank 0:
  - Hit events (ball ID + paddle ID)
  - Score events (ball ID + scoring side)
- Use OpenMP to parallelize per-ball updates

**Communication Pattern**:
- **Receive from Rank 0**: Paddle positions every frame
- **Send to Rank 0**: Ball states + events every frame

**OpenMP Usage**:
```cpp
#pragma omp parallel for
for (int i = 0; i < NUM_BALLS; ++i) {
    // Simulate ball[i] independently
    // Update position, check collisions
    // Record events (thread-safe)
}
```

## Ball Rarity System

| Ball ID | Rarity    | Color  | Points | Speed Multiplier |
|---------|-----------|--------|--------|------------------|
| 0       | Common    | Yellow | 1      | 1.0x             |
| 1       | Rare      | Red    | 3      | 1.0x             |
| 2       | Epic      | Purple | 5      | 1.0x             |
| 3       | Legendary | Cyan   | 10     | 1.5x (faster)    |

**Fixed Rarity Rules**:
- Each ball ID has a fixed rarity (no random changes)
- Rarity determines color, points, and speed
- Legendary ball is noticeably faster but same size
- Points are added when the ball scores

## Game Flow

1. **Initialization** (Rank 0 and Rank 1):
   - Rank 0: Create window, load assets, initialize game state
   - Rank 1: Initialize physics state for all 4 balls
   - Both: Sync initial state

2. **Main Game Loop** (60 FPS):
   - **Rank 0**:
     - Poll input events
     - Update paddle positions based on input
     - Send paddle positions to Rank 1
     - Receive ball states + events from Rank 1
     - Process events (update score, hit totals)
     - Render frame
     - Update timer
     - Check game end conditions
   
   - **Rank 1**:
     - Receive paddle positions from Rank 0
     - Simulate all balls (OpenMP parallel loop)
     - Detect collisions and events
     - Send ball states + events to Rank 0

3. **Game End** (60 seconds elapsed):
   - Rank 0: Calculate win rates, show leaderboard
   - Rank 1: Idle (or exit)

## Communication Protocol (per tick)

```
Rank 0 (Master)                                  Rank 1 (Worker)
----------------                                  ----------------
Poll input + move paddles
Build TickInput {
  paddle1, paddle2,
  gameWidth, gameHeight, dt,
  running,
  respawnFlags/positions
}
MPI_Send(TickInput)  --------------------------->  MPI_Recv(TickInput)
                                                   Apply respawns
                                                   OpenMP parallel-for over balls
                                                     - simulateBallTick(...)
                                                     - thread-local events
                                                   Merge events
                                                   Build TickOutput {
                                                     balls[], events[], eventCount
                                                   }
MPI_Recv(TickOutput) <--------------------------  MPI_Send(TickOutput)
Apply score/hit events
Respawn scored balls (next TickInput)
Render frame
```

### Message Types

- `TickInput`: packed paddle states, bounds, `dt`, running flag, respawn data
- `TickOutput`: packed ball states and packed event arrays
- Wire fields are explicit fixed-size numeric types (`int32`, `uint8`, `float`) for MPI safety

## Build Configuration

- **Project Type**: Console Application (Win32/x64)
- **MPI**: Microsoft MPI (MS-MPI) or Intel MPI
- **OpenMP**: Built-in compiler support (MSVC `/openmp`)
- **SFML**: Graphics, Window, System libraries

## Run Configuration

```powershell
# Standard 2-process run
mpiexec -n 2 PingPongSFML.exe

# Debug: verbose output
mpiexec -n 2 PingPongSFML.exe -verbose

# Single-process fallback (for testing without MPI)
PingPongSFML.exe
```

## OpenMP Event Aggregation Strategy

- OpenMP is used only in rank 1, only for the per-ball simulation loop.
- Each thread writes to its own local event list.
- After the parallel region, rank 1 merges all thread-local event lists into the outbound `TickOutput` buffer.
- This avoids event push data races while keeping simulation deterministic per-ball.
