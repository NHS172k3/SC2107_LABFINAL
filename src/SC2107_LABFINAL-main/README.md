# SC2107_LABFINAL
Collection of MSP432 example projects, labs and documentation used for embedded systems teaching and experiments (TI MSP-EXP432P401R / Code Composer Studio).

## Overview
This repository contains multiple lab exercises, example projects, and board documentation for the MSP432 LaunchPad platform. Projects are organized by lab / topic and include Code Composer Studio project files and linker/startup scripts.

## Quick links
- License: [src/license.txt](src/license.txt)  
- Project readme (original): [src/ReadMe.txt](src/ReadMe.txt)  
- Documentation (API / headers): [src/documentation/](src/documentation/)  
- Hardware datasheets & board files: [src/datasheets/](src/datasheets/)

## Prerequisites
- Texas Instruments Code Composer Studio (recommended) or compatible toolchain for MSP432.
- MSP-EXP432P401R LaunchPad (or equivalent MSP432 hardware).

## Quick start — open & build in Code Composer Studio
1. Open Code Composer Studio.
2. Import the project folder (Example: `File → Import → CCS Project` and select the project folder such as [src/BUMP_AVOIDANCE/](src/BUMP_AVOIDANCE/)).
3. Ensure the active build configuration targets the MSP432P401R and that the linker command file (e.g. [msp432p401r.cmd](src/BUMP_AVOIDANCE/msp432p401r.cmd)) is present.
4. Build the project via `Project → Build Project`.
5. Connect your MSP432 LaunchPad, then use the `Debug` perspective to load and run.

## File / Folder layout (high level)
- src/ — main repository content and labs  
  - [BUMP_AVOIDANCE/](src/BUMP_AVOIDANCE/) — bump avoidance example, startup & cmd files  
  - [BUMP_NAVIGATION/](src/BUMP_NAVIGATION/)  
  - [Control/](src/Control/)  
  - [Lab1_C_Programming/](src/Lab1_C_Programming/)  
  - [Lab2_ReflectanceGPIO/](src/Lab2_ReflectanceGPIO/)  
  - [Lab3_TimerCompare_Motor/](src/Lab3_TimerCompare_Motor/)  
  - [Lab4_ADC_IRSensors/](src/Lab4_ADC_IRSensors/)  
  - [LINE_FOLLOWER/](src/LINE_FOLLOWER/)  
  - [documentation/](src/documentation/) — generated API/html docs  
  - [datasheets/](src/datasheets/) — board/hardware files

## Building from command line
These projects are tailored for CCS; command-line makefiles may exist under project `Debug/` folders. For reproducible builds, use CCS or ensure your toolchain mirrors the project settings (compiler flags, linker script).

## Troubleshooting
- If projects import with warnings or missing preferences, check `.metadata/` and project `.cproject` / `.project` files in each project folder.
- Advice and ULP markers in `.metadata` indicate static-analysis advice (low-power recommendations). See `src/.metadata/.log` for import/build diagnostics.

## Contributing
- Add new labs under `src/` following the existing folder structure.
- Include a short README in each project folder describing purpose, board, and how to run.

## License
See [src/license.txt](src/license.txt) for licensing details.

---

For full file list and to browse examples, open the `src/` folder in your repo or IDE:
- [src/](src/)
