// Ventana de diálogo estilo Pokémon: cuadro blanco abajo, texto que se escribe letra por letra
#ifndef UI_DIALOGO_H
#define UI_DIALOGO_H

// Abre un diálogo con un nombre y una lista de frases
void dialogo_abrir(const char *nombre, const char *const *frases, int n);
int  dialogo_activo(void);
// Llamar cada cuadro. avanzar = 1 en el cuadro en que se aprieta el botón. Devuelve 1 cuando el diálogo se acaba de cerrar.
int  dialogo_actualizar(int avanzar);
void dialogo_dibujar(void);       // dentro de ui_inicio / ui_fin

#endif
