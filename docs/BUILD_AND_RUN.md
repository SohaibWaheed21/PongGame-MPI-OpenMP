# BUILD AND RUN — Hybrid MPI + OpenMP Pong

This guide explains how to build and run the Hybrid MPI + OpenMP Pong game in Visual Studio 2026 (or 2022).

---

## Prerequisites

### Required Software
1. **Visual Studio 2022** (or 2019)
   - C++ Desktop Development workload
   - Windows SDK

2. **SFML 2.5.1** (or later)
   - Download from: https://www.sfml-dev.org/download.php
   - Choose Visual C++ version matching your VS version
   - Extract to a known location (e.g., `C:\SFML-2.5.1`)

3. **Microsoft MPI (MS-MPI)**
   - Download from: https://docs.microsoft.com/en-us/message-passing-interface/microsoft-mpi
   - Install both:
     - `msmpisdk.msi` (development files)
     - `msmpisetup.exe` (runtime)

4. **Arial Font** (`arial.ttf`)
   - Usually found in `C:\Windows\Fonts\arial.ttf`
   - Must be copied to the output directory (next to the `.exe`)

---

## Project Setup in Visual Studio

### 1. Open the Solution
1. Open Visual Studio
2. File → Open → Project/Solution
3. Navigate to `D:\PingPongSFML\` and open `PingPongSFML.sln`

### 2. Configure SFML Include Directories
1. Right-click the project → Properties
2. Configuration: **All Configurations**, Platform: **x64**
3. Navigate to: **C/C++** → **General** → **Additional Include Directories**
4. Add: `C:\SFML-2.5.1\include` (adjust path to your SFML location)

### 3. Configure SFML Library Directories
1. Navigate to: **Linker** → **General** → **Additional Library Directories**
2. Add: `C:\SFML-2.5.1\lib` (adjust path)

### 4. Link SFML Libraries
1. Navigate to: **Linker** → **Input** → **Additional Dependencies**
2. For **Debug** configuration, add:
   ```
   sfml-graphics-d.lib
   sfml-window-d.lib
   sfml-system-d.lib
   ```
3. For **Release** configuration, add:
   ```
   sfml-graphics.lib
   sfml-window.lib
   sfml-system.lib
   ```

### 5. Enable OpenMP (for Phase 3+)
1. Navigate to: **C/C++** → **Language** → **OpenMP Support**
2. Set to: **Yes (/openmp)**

### 6. Configure MPI (for Phase 2+)
1. Navigate to: **C/C++** → **General** → **Additional Include Directories**
2. Add: `$(MSMPI_INC)` (environment variable set by MS-MPI SDK)

3. Navigate to: **Linker** → **General** → **Additional Library Directories**
4. Add: `$(MSMPI_LIB64)` (for x64 builds)

5. Navigate to: **Linker** → **Input** → **Additional Dependencies**
6. Add: `msmpi.lib`

### 7. Set Working Directory
1. Navigate to: **Debugging** → **Working Directory**
2. Set to: `$(OutDir)`
   - This ensures the program runs in the output folder where DLLs and assets are located

---

## Copy Runtime Assets

### SFML DLLs
Copy the following DLLs from `C:\SFML-2.5.1\bin\` to your output directory:
- For **Debug**: Copy from Debug build folder (if available)
  - `sfml-graphics-d-2.dll`
  - `sfml-window-d-2.dll`
  - `sfml-system-d-2.dll`
- For **Release**: 
  - `sfml-graphics-2.dll`
  - `sfml-window-2.dll`
  - `sfml-system-2.dll`

**Output directory locations**:
- Debug: `D:\PingPongSFML\x64\Debug\`
- Release: `D:\PingPongSFML\x64\Release\`

### Arial Font
1. Copy `C:\Windows\Fonts\arial.ttf` to the output directory
2. Must be named exactly `arial.ttf`

**Quick PowerShell command** (run from project root):
```powershell
# Copy arial.ttf to both Debug and Release
Copy-Item "C:\Windows\Fonts\arial.ttf" -Destination "x64\Debug\arial.ttf"
Copy-Item "C:\Windows\Fonts\arial.ttf" -Destination "x64\Release\arial.ttf"

# Copy SFML DLLs (adjust SFML path as needed)
$sfmlPath = "C:\SFML-2.5.1\bin"
Copy-Item "$sfmlPath\sfml-graphics-2.dll" -Destination "x64\Release\"
Copy-Item "$sfmlPath\sfml-window-2.dll" -Destination "x64\Release\"
Copy-Item "$sfmlPath\sfml-system-2.dll" -Destination "x64\Release\"

Copy-Item "$sfmlPath\sfml-graphics-d-2.dll" -Destination "x64\Debug\" -ErrorAction SilentlyContinue
Copy-Item "$sfmlPath\sfml-window-d-2.dll" -Destination "x64\Debug\" -ErrorAction SilentlyContinue
Copy-Item "$sfmlPath\sfml-system-d-2.dll" -Destination "x64\Debug\" -ErrorAction SilentlyContinue
```

---

## Building the Project

### In Visual Studio
1. Select configuration: **Debug** or **Release**
2. Select platform: **x64**
3. Build → Build Solution (Ctrl+Shift+B)
4. Verify no errors in the Output window

### Expected Output
```
1>------ Build started: Project: PingPongSFML, Configuration: Debug x64 ------
1>Ball.cpp
1>Game.cpp
1>main.cpp
1>Paddle.cpp
1>Score.cpp
1>PingPongSFML.vcxproj -> D:\PingPongSFML\x64\Debug\PingPongSFML.exe
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

---

## Running the Game

### Phase 0-1: Single Process (No MPI)
**From Visual Studio**:
1. Press **F5** (Start Debugging) or **Ctrl+F5** (Start Without Debugging)
2. The game window should appear

**From Command Line**:
```powershell
cd D:\PingPongSFML\x64\Debug
.\PingPongSFML.exe
```

### Phase 2+: Multi-Process with MPI
**From Command Line**:
```powershell
cd D:\PingPongSFML\x64\Debug

# Run with 2 MPI processes (Rank 0 and Rank 1)
mpiexec -n 2 PingPongSFML.exe
```

**From Visual Studio** (configure for MPI debugging):
1. Right-click project → Properties → Debugging
2. **Command**: `C:\Program Files\Microsoft MPI\Bin\mpiexec.exe`
3. **Command Arguments**: `-n 2 $(TargetPath)`
4. **Working Directory**: `$(OutDir)`
5. Press **F5** to debug

---

## Visual Studio Configuration Checklist (submission/demo)

### Required MPI debug launch settings
- Project Properties → **Debugging**
  - **Command**: `C:\Program Files\Microsoft MPI\Bin\mpiexec.exe`
  - **Command Arguments**: `-n 2 "$(TargetPath)"`
  - **Working Directory**: `$(OutDir)`

### Required OpenMP compiler setting
- Project Properties → **C/C++ → Language → OpenMP Support**
  - Set to **Yes (`/openmp`)**

### Required runtime assets
- SFML DLLs must be in `$(OutDir)`
- `arial.ttf` must be in `$(OutDir)`

### Required runtime command
- Terminal run command:
  - `mpiexec -n 2 PingPongSFML.exe`

---

## Benchmark Mode (optional)

- In `src/common/Constants.h`:
  - `BENCHMARK_MODE = false` (default gameplay: 4 balls)
  - Set `BENCHMARK_MODE = true` to build a higher-ball-count benchmark run
- This is useful to demonstrate OpenMP benefit in rank 1.

---

## Troubleshooting

### "Failed to load font: arial.ttf"
- Ensure `arial.ttf` is in the same directory as the `.exe`
- Check the "Working Directory" in project properties is set to `$(OutDir)`

### "SFML DLL not found" errors
- Copy SFML DLLs to the output directory (see "Copy Runtime Assets" above)
- For Debug builds, use `-d` DLL versions (e.g., `sfml-graphics-d-2.dll`)

### "Cannot open include file: 'SFML/Graphics.hpp'"
- Verify SFML include path in project properties (see "Configure SFML Include Directories")
- Ensure SFML is extracted and path is correct

### "Unresolved external symbol" linker errors
- Verify SFML library path in project properties (see "Configure SFML Library Directories")
- Ensure correct `.lib` files are linked for Debug/Release configurations

### MPI errors (Phase 2+)
- **"mpiexec not found"**: Add `C:\Program Files\Microsoft MPI\Bin` to your PATH
- **"MPI_Init failed"**: Ensure MS-MPI runtime is installed (`msmpisetup.exe`)
- **"Only one process spawned"**: Check `mpiexec -n 2` command

---

## Game Controls

### Player 1 (Left Paddle)
- **W**: Move up
- **S**: Move down

### Player 2 (Right Paddle)
- **Up Arrow**: Move up
- **Down Arrow**: Move down

### General
- **Close Window**: Exit game

---

## Performance Notes

- **Target Frame Rate**: 60 FPS (enforced by SFML)
- **Game Duration**: 60 seconds
- **Resolution**: 1000×555 pixels (fixed)

---

## Next Steps

After successful build and run:
1. ✅ Phase 0: Documentation and build setup (current)
2. ✅ Phase 1: Convert to 4-ball gameplay (sequential)
3. ✅ Phase 2: Integrate MPI (rank 0 and rank 1 split)
4. ✅ Phase 3: Add OpenMP parallelization (physics worker)
5. ✅ Phase 4: Quality and robustness polish

---

## Additional Resources

- **SFML Documentation**: https://www.sfml-dev.org/documentation/
- **MS-MPI Documentation**: https://docs.microsoft.com/en-us/message-passing-interface/microsoft-mpi
- **OpenMP Tutorial**: https://www.openmp.org/resources/tutorials-articles/
