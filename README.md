# NRaster

My implementation of a software rasterizer. 

## How to build

NRaster uses premake5 to generate the project. The project comes with GenerateSolution.bat that will generate a VS2017 solution.

## Features

* Multi thread triangle rasterization using bins.
* Perspective correct attribute interpolation
* Supports OBJs
* Programable vertex and pixel shaders.

## Dependencies

* SDL2, used for window creation and displaying the results to the screen
* tinyOBJ
* glm
* Premake5
* tinythreds

![Teapot](Data/Pics/RasterProgress.png?raw=true "Utah Teapot")
![Scene](Data/Pics/AnotherScene.png?raw=true "Scene")