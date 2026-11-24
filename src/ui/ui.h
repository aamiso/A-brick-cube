// Dibujo 2D sencillo: rectángulos y texto con una fuente de puntos (en pantalla de 960 x 544)
#ifndef UI_UI_H
#define UI_UI_H

void ui_inicio(void);                                  // empezar a dibujar la interfaz
void ui_fin(void);                                     // terminar (vuelve al modo 3D)
void ui_rect(float x, float y, float w, float h, unsigned rgba);   // 0xRRGGBBAA
// Escribe texto (UTF-8). esc = tamaño de cada punto. Devuelve el ancho dibujado.
float ui_texto(float x, float y, int esc, const char *txt, unsigned rgba);
float ui_ancho(const char *txt, int esc);
// Parte el texto en líneas que caben en "ancho" píxeles. Devuelve cuántas líneas (máx. max_lin).
int  ui_partir(const char *txt, int esc, float ancho, char lin[][96], int max_lin);

#endif
