
#include <windows.h>
#include <bass.h>
#include <GL/gl.h>
#include <GL/glfw.h>
#include <math.h>
#include <string.h>
int chan,act,time,level;
int ismod;
int pos;

long millimod = 0;

long long millis() {
    LARGE_INTEGER s_frequency;
    int s_use_qpc = QueryPerformanceFrequency(&s_frequency);
   if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (millimod+((1000LL * now.QuadPart) / s_frequency.QuadPart));
   } else {
      return (millimod+GetTickCount());
   }
}
int quit = 0;
int left = -1;
int right = -1;

void handleKeypress(int theKey, int theAction)
{ 
  printf("%d\n", theKey); 
  if (theKey == 257) quit = 1; 
  if (theKey == 82) quit = 2; 
  if (theKey == 285) left = -left;
  if (theKey == 286) right = -right;
}
int main(int argc, char* argv[]) 
{
if (HIWORD(BASS_GetVersion())!=BASSVERSION) {
return 2;
}

BASS_Init(-1,44100,0,0,NULL);
char songname[100] = "                  ";
if (argc > 1)
strcpy(songname, argv[1]);
else strcpy(songname, "data/music");
printf("song: %s", songname);

BASS_SetConfig(BASS_CONFIG_BUFFER, 5000);

if ((chan=BASS_StreamCreateFile(FALSE,songname,0,0,BASS_STREAM_PRESCAN))
       || (chan=BASS_StreamCreateURL(songname,0,BASS_STREAM_PRESCAN,0,0))) {
       pos=BASS_ChannelGetLength(chan,BASS_POS_BYTE);
       if (BASS_StreamGetFilePosition(chan,BASS_FILEPOS_DOWNLOAD)!=-1) {
       } else
       ismod=FALSE;
   } else {
       // try loading the MOD (with looping, sensitive ramping, and calculate the duration)
       if (!(chan=BASS_MusicLoad(FALSE,songname,0,0,BASS_MUSIC_RAMPS|BASS_MUSIC_PRESCAN,1)))
           // not a MOD either
       { // count channels
           float dummy; 
           int a;
           for (a=0;BASS_ChannelGetAttribute(chan,BASS_ATTRIB_MUSIC_VOL_CHAN+a,&dummy);a++);
}
       pos=BASS_ChannelGetLength(chan,BASS_POS_BYTE);
       ismod=TRUE;
   }

   // display the time length
   if (pos!=-1) {
       time=(DWORD)BASS_ChannelBytes2Seconds(chan,pos);
       printf(" %u:%02u",time/60,time%60);
   } else {
       printf("");
   }

glfwInit();
if( !glfwOpenWindow( 1024, 768, 0,0,0,0,0,0, GLFW_WINDOW ) )
{
glfwTerminate();
return 3;
}
glfwSetWindowTitle("Trilobit THC ('THC hacky compiler ver 0.1')");
glfwSetKeyCallback(handleKeypress);

BASS_ChannelPlay(chan,FALSE);
printf("mainloop, press any key in the console to quit.\n");

long startmillis = millis();

while(quit == 0 && (act=BASS_ChannelIsActive(chan)))
{

  if (left == 1)
  {
    int amo = -10;
    millimod+=amo;
    BASS_ChannelStop(chan);
    BASS_ChannelSetPosition(chan, BASS_ChannelGetPosition(chan,0)+amo,BASS_MUSIC_POSRESETEX);
    BASS_ChannelPlay(chan, FALSE);
  }

  if (right == 1)
  {
    int amo = 10;
    millimod+=amo;
    BASS_ChannelStop(chan);
    BASS_ChannelSetPosition(chan, BASS_ChannelGetPosition(chan,0)+amo,BASS_MUSIC_POSRESETEX);
    BASS_ChannelPlay(chan, FALSE);
  }

  float m;
  m = (float)millis() - startmillis;

  glClearColor(1.0f*cos(m*0.001),1.0f,1.0f,1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  for (int i = 0; i < 6; i++)
  {
  glLoadIdentity();

    glTranslatef(0.0f,-0.5f-cos(m*0.1),0.0f);
    glRotatef(i*0.4*100.0f+m*0.1,1.0f,0.0f,0.0f);
  glBegin(GL_TRIANGLES);
    glColor3f(1.0f-cos(m*0.1),0.0f+i*0.02,0.0f);
    glVertex3f( 0.0f, 2.0f, 0.0f);
    glColor3f(1.0f,1.0f,0.0f-cos(m*0.1));
    glVertex3f(-2.0f,-2.0f, 0.0f);
    glColor3f(1.0f,0.0f-cos(m*0.1),1.0f);
    glVertex3f( 2.0f,-2.0f, 0.0f);
  glEnd();
}

  glfwSwapBuffers();
}

exit:
printf("termination");
glfwTerminate();
/*
BASS_ChannelSlideAttribute(chan,BASS_ATTRIB_FREQ,1000,500);
Sleep(400);
BASS_ChannelSlideAttribute(chan,BASS_ATTRIB_VOL,-1,200);
while (BASS_ChannelIsSliding(chan,0)) Sleep(1);
*/
BASS_Free();
return quit;
}