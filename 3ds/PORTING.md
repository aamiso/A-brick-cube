# Port nativo de A Brick Cube para Nintendo 3DS

## Regla principal

El port reutiliza la lógica del juego original en `src/`. No se reimplementan en `3ds/source/main.c` el movimiento, las colisiones, la cámara, las animaciones, la ciudad ni los vehículos. La capa que se sustituye es la de plataforma y renderizado.

## Módulos originales que se conservan

- `src/player/jugador.c` y `jugador.h`: movimiento, salto, aceleración y colisiones del jugador.
- `src/player/personaje.c` y `personaje.h`: modelo voxel articulado y animaciones.
- `src/engine/camara.c` y `camara.h`: seguimiento, modo hombro, límites y prevención de obstáculos.
- `src/world/ciudad.c` y `ciudad.h`: lectura del mapa original, terreno urbano y colisiones.
- `src/world/lugares.c` y `lugares.h`: modelos voxel originales de edificios y sus colisiones.
- Los demás módulos de `src/world/` y `src/ui/` se integran conservando su lógica, en lugar de sustituirlos por aproximaciones.

## Capa específica de 3DS

La plataforma 3DS debe proporcionar:
1. Inicialización y cierre de Citro3D/ctru.
2. Una implementación de renderizado Citro3D para las operaciones gráficas que usan los módulos originales.
3. Lectura de controles con Circle Pad y botones físicos, traducida al formato de entrada del juego.
4. Acceso a los datos originales mediante RomFS, manteniendo las rutas de mapa y modelos.
5. Un bucle de presentación adaptado a las pantallas de 3DS.

El código de plataforma no debe contener una segunda implementación de la física, cámara o mundo.

## Cámara y controles

- La cámara se actualiza mediante `camara_actualizar()` y se aplica mediante `camara_aplicar()`.
- El modo de hombro original usa una distancia objetivo de 2.7 unidades y desplazamiento lateral suave.
- El Circle Pad sustituye al stick izquierdo.
- Como Old 3DS no tiene stick derecho, la cámara se controla con una asignación de botones; esto solo cambia la entrada, no la lógica de cámara.

## Datos originales

El mapa debe ser `data/mapas/ciudad.txt` (252 x 252, con su posición inicial marcada por `P`) y los modelos deben proceder de `data/modelos/`. No se debe reconstruir el mapa con colores genéricos ni reemplazar los modelos por cajas aproximadas.

## Criterios para considerar integrado el port

- El build 3DS compila los módulos originales que contienen la lógica del juego.
- El mapa completo y los modelos originales se leen desde RomFS.
- El jugador se actualiza con `jugador_actualizar()` y se dibuja con `jugador_dibujar()`.
- La cámara usa `camara_actualizar()` y `camara_aplicar()`.
- No quedan fórmulas duplicadas de movimiento/cámara en el main de 3DS.
- El resultado se instala como CIA mediante el flujo de empaquetado del proyecto.
- Un build exitoso solo confirma compilación; el funcionamiento real debe verificarse en hardware o emulador.

## Estado de integración

La base del port ya está integrada en `main`:

- `3ds/source/main.c` usa el bucle principal original de `src/main.c`, adaptando la lectura de controles y el inicio/cierre de Citro3D.
- El Makefile compila los módulos originales de `src/core`, `src/engine`, `src/player`, `src/world` y `src/ui`; excluye solamente el `src/main.c` de Vita.
- `3ds/source/vitaGL_compat.c` traduce las operaciones de matriz, buffers y dibujo fijo usadas por esos módulos a Citro3D.
- `src/world/lugares.h` selecciona `romfs:/modelos/` cuando se compila para 3DS.
- La cámara inicia en el modo de hombro original, que utiliza la distancia objetivo de 2.7 unidades.
- La compilación de GitHub Actions produjo `A-brick-cube-3ds.3dsx` correctamente en el commit `a1dbad6063219de31ebd1cf63e2c43c71be9f834`.

**Pendiente de validar:** ejecución real en Old 3DS o emulador. La compilación exitosa confirma que el programa compila y enlaza, pero no demuestra todavía el rendimiento ni el comportamiento gráfico en el dispositivo. El artefacto generado en esta etapa es `.3dsx`, no un paquete CIA para FBI.
