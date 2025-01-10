# Emerald Isle [(Click here to se the demonstration)](https://youtu.be/GFyuOIc1MCU)


## Overview
Emerald Isle is a 3D graphics project developed using OpenGL, GLFW, and GLM. The project simulates a futuristic Dublin environment with various entities such as buildings, trees, aircraft, and more. The project demonstrates advanced graphics programming techniques, including shader programming, texture mapping, and light managing.
Please read the report for further understanding. 
## Features
- **3D Rendering**: Utilizes OpenGL for rendering 3D objects and environments.
- **Shader Programming**: Implements vertex and fragment shaders for various effects.
- **Texture Mapping**: Applies textures to 3D models for realistic appearances.
- **Camera Control**: Provides interactive camera controls using keyboard and mouse inputs.
- **Entity Management**: Manages various entities like buildings, trees, and aircraft within the city environment.

## Skills Demonstrated
- **OpenGL Programming**: Proficiency in using OpenGL for 3D graphics rendering.
- **Shader Development**: Ability to write and debug GLSL shaders for various graphical effects.
- **Texture Handling**: Experience in loading and applying textures to 3D models.
- **Interactive Controls**: Implementation of interactive camera controls using GLFW.
- **Entity Management**: Efficient management and rendering of multiple entities in a 3D environment.
- **Mathematical Skills**: Application of mathematical concepts for transformations, projections, and lighting calculations.
- **Robustness Analysis:** Emphasis on ensuring stability and error handling in various aspects of the project.


## Installation
1. **Clone the repository**:
    ```sh
    git clone https://github.com/DavidEsteso/Emerald-isle.git
    cd Emerald-isle
    ```
2. **Install dependencies**: Ensure you have the following libraries installed:
    - GLFW
    - GLM
    - GLAD
    - TinyOBJLoader
3. **Build the project**: Use your preferred build system (e.g., CMake) to compile the project.
4. **Run the application**: Execute the compiled binary to run the Emerald Isle simulation.

## Usage
### Camera Controls
- `W`, `A`, `S`, `D`: Move the camera forward, left, backward, and right.
- Mouse Movement: Rotate the camera view.
- `ESC`: Exit the application.

### Interaction
- `ENTER`: Interact with aircraft within the city environment.

## File Structure
- `src/`: Contains the source code for the project.
- `include/`: Contains header files.
- `shaders/`: Contains GLSL shader files.
- `models/`: Contains 3D model files.
- `textures/`: Contains texture files.

## Note
This project is currently not executable as the following folders are missing:
- `lab2/models`
- `lab2/textures/bricks`
- `lab2/textures/panel`

However, in the `report.pdf` file, there are links at the end where you can download these missing files.

## Contributing
Contributions are welcome! Please fork the repository and submit a pull request with your changes.

## License
This project is licensed under the MIT License.