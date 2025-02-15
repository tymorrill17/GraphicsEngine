# Fluid Simulation Project

## Goals
I want to learn about and play with particle-based fluid simulation techniques to make neat-looking and interactive animations. I will start in 2D then work my way up to 3D. Here are a few goals I have in mind for this project:
1. Use multi-threading and/or compute shaders to perform physics updates and simulate many particles (I want to push the bounds and see how high I can get)
2. Change the billboard or sphere-mesh way of rendering the fluid to get a realistic fluid render
3. Add support for interactions with non-particle meshes, i.e. dropping objects into the fluid and having them interact

## Progress
### 01 - Particles Rendered, Interactive Windows
After a long time developing a more general-purpose rendering engine, I finally have a starting point for this project. Using Dear ImGui for the debug windows, I have an interface to interact with important parameters.
Currently, the particles are just given a random initial velocity with gravity acting on them and basic collision with the walls of the window.

![Demonstration of Interactive Window](pics/Milestone1-InteractiveParticleSystem.gif)

Next step is to apply basic collision detection between the particles.
