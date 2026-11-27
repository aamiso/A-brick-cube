#include <3ds.h>
#include <stdio.h>

int main(void) {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    romfsInit();

    printf("A Brick Cube 3DS - Prueba inicial\n\n");

    FILE *f = fopen("romfs:/mapas/ciudad.txt", "r");
    if (f) {
        printf("[OK] ciudad.txt cargado desde romfs!\n");
        fclose(f);
    } else {
        printf("[ERROR] No se encontro ciudad.txt\n");
    }

    printf("\nPresiona START para salir.\n");

    while (aptMainLoop()) {
        gspWaitForVBlank();
        gfxSwapBuffers();
        hidScanInput();

        if (hidKeysDown() & KEY_START) break;
    }

    romfsExit();
    gfxExit();
    return 0;
}
