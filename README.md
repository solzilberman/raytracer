# Raytracer

**Sample Renders**
![](/configs/sample_render1.png)
![](/configs/sample_render2.png)
![](/configs/sample_render3.png)

**Build (Target: Win10 x64)**
 - Open .sln
 - ctrl + F5

**Configuration**
Scene configurations are stored in `ProjectDir/configs/`
Provided example scenes: *scene.json* and *cornell.json*
The sample scenes contain all required json key/values for rendering.
*Optional: Skybox object.*

Global Flags `#define` in top of `main.cpp` file:
`GL_FLAG`: if set to 1, will bring up live display of rendered image after tracing is complete.
`ANIMATION_FLAG, ANIMATION_FRAMES`: if `ANIMATION_FLAG` set to 1, will generate count `ANIMATION_FRAMES`  frames in frame folder.

**Output Structure**
`/renders/stills` output png location if animation is disabled.
`/renders/frames` output of each frame of animation if animation is enabled.
`ProjectRoot/utils/` contains python script for compiling frames into .mp4 and cleaning frame folder.

**Run**
executable: `/x64/Release/raytracer.exe`


**COOL FEATURES**

 - json scene configuration
 - Plane, Quad, Sphere, Triangle, Capsule geometries integrated
 - Skybox (when enabled)
 - Animation (extra credit)
 -  optimization via OpenMP compiler directives (extra credit)
 - Super-sample anti-aliasing (extra credit)

**References**
References are included in the source code for external assets and additional features I was experimenting with. 
