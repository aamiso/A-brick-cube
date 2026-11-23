// Lectura de los controles
#ifndef CORE_ENTRADA_H
#define CORE_ENTRADA_H

// Convierte el valor de un stick (0 a 255) en un número de -1 a 1,
// con una "zona muerta" para que no se mueva solo.
float leer_stick(unsigned char valor);

#endif
