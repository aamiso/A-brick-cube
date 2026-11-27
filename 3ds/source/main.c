#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>
#include <stdlib.h>

static void probar_romfs(void) {
    Result rc = romfsInit();
    if (R_FAILED(rc)) {
        printf("romfsInit fallo: 0x%08lX\n", rc);
        return;
    }

    FILE* f = fopen("romfs:/mapas/ciudad.txt", "rb");
    if (!f) {
        printf("No se abrio romfs:/mapas/ciudad.txt\n");
        romfsExit();
        return;
    }

    char linea[128] = {0};
    if (fgets(linea, sizeof(linea), f) != NULL) {
        printf("Mapa leido: %s\n", linea);
    } else {
        printf("Archivo vacio o sin salto de linea\n");
    }

    fclose(f);
    romfsExit();
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("A Brick Cube 3DS - Base port\n");
    printf("Presiona START para salir.\n\n");

    probar_romfs();

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) {
            break;
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    gfxExit();
    return 0;
}
