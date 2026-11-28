// Pantalla de abajo de la 3DS (320 x 240): hora, dinero, minimapa, misión y textos
#ifndef BRICK3DS_ABAJO_H
#define BRICK3DS_ABAJO_H

#define ABAJO_W 320
#define ABAJO_H 240

// Botón "Guardar" (para detectar el toque)
#define BOTON_X 222
#define BOTON_Y 208
#define BOTON_W 94
#define BOTON_H 28

typedef struct {
    float px, pz, ang;          // jugador (o su carro)
    int en_carro;
    const char *lugar;          // título del interior (NULL = en la calle)
    const char *aviso;          // "B: Hablar con ..." / "X: Entrar ..." (NULL = nada)
    const char *oferta;         // texto de oferta de misión (NULL = nada)
    const char *guardado_msg;   // "Partida guardada" etc. (NULL = nada)
    int guardado_ok;
    int boton_apretado;
} EstadoAbajo;

void abajo_dibujar(const EstadoAbajo *e);   // entre ui_inicio_area / ui_fin

#endif
