#include "SDL/SDL_main.h"
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_mixer.h"

#include <windows.h>
#include <bass.h>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <math.h>
#include <string.h>




int main(int argc, char* argv[]) 
{
    return SDL_main(argc,argv);
}
int SDL_main(int argc, char *argv[])
{
    SDL_Event event;
    SDL_Surface *screen;

    int quit;
    quit = 0;

    if( SDL_Init(SDL_INIT_VIDEO) < 0 ) exit(1);

    SDL_WM_SetCaption("SDL Window", NULL);

    screen = SDL_SetVideoMode( 1024 , 768 , 32 , SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_ANYFORMAT);

    printf("init audio\n");
    // Inilialize SDL_mixer , exit if fail
    if( SDL_Init(SDL_INIT_AUDIO) < 0 ) exit(1);
    // Setup audio mode
    printf("open audio\n");
    Mix_OpenAudio(44100,AUDIO_S16SYS,2,640);
    Mix_Music *mus;
    printf("load music\n");
    mus = Mix_LoadMUS("data/music.ogg");
    if(!mus) {
        printf("Mix_LoadMUS(\"music.ogg\"): %s\n", Mix_GetError());
    }
    printf("play music\n");
    Mix_PlayMusic(mus,1);

    printf("mainloop\n");
    while(quit == 0) 
    {

        SDL_FillRect(screen , NULL , 0x221122);
        SDL_Flip(screen);

        while( SDL_PollEvent( &event ) )
        {
            switch( event.type )
            {
                case SDL_KEYDOWN:
                    switch( event.key.keysym.sym )
                    {
                        case SDLK_UP:
                            break;
                        case SDLK_DOWN:
                            break;
                        case SDLK_LEFT:
                            break;
                        case SDLK_RIGHT:
                            break;
                        case SDLK_ESCAPE:
                            quit = 1;
                        default:
                            break;
                        } // switch( event.key.keysym.sym ){ END
                    break; // case SDL_KEYDOWN: END
                case SDL_QUIT:
                    quit = 1;
                    break;
                default:
                    break;
            } // switch( event.type ){ END
        }
    };

    Mix_FreeMusic(mus);
    SDL_Quit();
    return 0;
}

