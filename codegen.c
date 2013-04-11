#include <tcclib.h>
#include <stdarg.h>

#include "libtcc.h"

size_t strlen(const char *str)
{
    const char *s;
    int jore = 0;
    for (s = str; *s; ++s)
        jore++;
    return jore;
}

void addcode(char* src, char* fmt, ...)
{
  va_list va;

  va_start(va, fmt);
  sprintf(src+strlen(src), fmt, va);
  va_end(va);
}

char* src;
TCCState *s;

void reload_source_file()
{
    printf("source - (re)loading source from file out.c\n");

    FILE *pFile = fopen("out.c", "rb");
    long lSize = 0;

    printf("source - reading: filesize = ");

    fseek(pFile, 0, 2);
    lSize = ftell(pFile);
    rewind(pFile);

    printf("%d\n",lSize);

    printf("source - allocating source\n");

    realloc(src, sizeof(char)*lSize);

    printf("source - allocating source done\n");

    printf("reading out.c");

    int result = fread(src,1,lSize,pFile);

    if (result == lSize) printf("...reading complete!\n");
    else printf("...error reading source file.\n");

//    printf("----------------------------------------------\n%s\n--------------------------------------------------------\n", src);

    fclose(pFile);
}

void compile_and_link()
{
    tcc_add_library_path(s, ".");
    tcc_add_library_path(s, ".\\tcc\\lib");
    tcc_add_include_path(s, ".\\tcc\\include");
    tcc_add_include_path(s, ".\\tcc\\include\\winapi");

    tcc_add_library(s, "bass");

    tcc_add_library(s, "opengl32");
    tcc_add_library(s, "glfw");

    tcc_add_library(s, "SDL");
    tcc_add_library(s, "SDL_image");
    tcc_add_library(s, "SDL_mixer");

    tcc_add_library(s, "libjpeg-8");
    tcc_add_library(s, "libpng-15-15");
    tcc_add_library(s, "libtiff-5");
    tcc_add_library(s, "libwebp-2");

    printf("linker set\n");

    printf("type!\n");
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
    printf("compiling!\n");
    tcc_compile_string(s, src);
    printf("compilation complete!\n");
}

int execute(int argc, char* argv[])
{
    printf("executing compiler program, argc: %d\n", argc);
    printf("---------------------------------------------------------\n");

    int ret = tcc_run(s,argc,argv);
    char errorcode[256];
    printf("---------------------------------------------------------\n");
    printf("execution terminated\n");
    sprintf(errorcode, "error in c script! code: %d", ret);
    printf("script sez: %s\n", ret >= 0 ? "no error!" : errorcode);
    return ret;
}

int main(int argc, char* argv[])
{
    printf("THC - Trilobit Hacky Compiler 0.2\n");
    src = (char*)malloc(128000);

        printf("entry - tcc init\n");
        s = tcc_new();
        printf("entry - tcc init done\n");

        if (!s) {
            printf("ERROR: Could not create a new tcc instance.\n");
            exit(1);
        }

        reload_source_file();
        compile_and_link();
        int ret = execute(argc, argv);

    tcc_delete(s);

    exit(0);
    return 0;
}