# Hybrid MPI + OpenMP Multi‑Ball Pong (SFML, C++ / Visual Studio)

A classic 2‑player Pong game built with **SFML** and extended into a **multi‑ball** version with **rarity-based scoring**. The project demonstrates a **hybrid parallel design**:

- **MPI** is used to split the program into two processes:
  - **Rank 0 (Master):** SFML window, input handling, authoritative score/timer, rendering
  - **Rank 1 (Worker):** headless physics simulation + collision detection
- **OpenMP** is used inside the **worker process** to parallelize the per‑tick loop that simulates all balls.

## Features
- **4 balls on screen at once**, fixed rarities:
  - **Common:** Yellow, +1 point
  - **Rare:** Red, +3 points
  - **Epic:** Purple, +5 points
  - **Legendary:** Cyan, +10 points (**faster**)
- **Independent scoring and respawn** per ball
- **Total paddle hit counters** (accumulated across all balls)
- **Match timer** and end-of-game display
- Clean separation between **rendering** and **simulation state** (MPI-friendly)

## Requirements
- Windows + Visual Studio (x64)
- SFML (matching your compiler/runtime)
- MPI implementation (recommended on Windows: **MS‑MPI**)
- OpenMP enabled in the project

## Build (Visual Studio)
1. Open the solution in Visual Studio.
2. Select **x64** configuration.
3. Make sure SFML include/lib directories are configured.
4. Enable OpenMP:
   - `Project Properties → C/C++ → Language → OpenMP Support = Yes (/openmp)`
5. Build the project.

## Run (Terminal)
From a terminal in the built output folder (where the `.exe` is located):

```bash
mpiexec -n 2 PongGame.exe
```

- `-n 2` starts:
  - rank 0 = renderer/authority
  - rank 1 = physics worker

## Run (Visual Studio Debugging)
Set these in:
`Project Properties → Configuration Properties → Debugging`

- **Command:**
  - `C:\Program Files\Microsoft MPI\Bin\mpiexec.exe`
- **Command Arguments:**
  - `-n 2 "$(TargetPath)"`
- **Working Directory:**
  - `$(OutDir)`

## Notes (SFML runtime assets)
Make sure these are present in the output directory (`$(OutDir)`):
- SFML DLLs (e.g., `sfml-graphics-2.dll`, `sfml-window-2.dll`, `sfml-system-2.dll`)
- `arial.ttf` (or update the code to use a different font path)

## How the parallelism is used
- **MPI:** Rank 0 sends paddle state to Rank 1 each tick; Rank 1 returns updated ball states and hit/score events.
- **OpenMP:** Rank 1 uses an OpenMP-parallel loop to update multiple balls concurrently per tick.

## License
Add a license if needed for your submission.
