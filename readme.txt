write code into out.c, codegen.exe will recompile it each time using TCC as a compiler

included in 0.2:

	- GL
	- GLFW
	- BASS
	- SDL
	- SDL_IMAGE
	- SDL_MIXER
	- WINAPI

To add new libraries:
	- add then to codegen.c linking stage and recompile codegen with MingW
	- use tiny_impdef to generate def files from dll
	- add library include headers to the include directory
	- run codegen on your new out.c

vp79799@gmail.com