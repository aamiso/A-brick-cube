<p align="center">
  <img src="https://raw.githubusercontent.com/aamiso/A-brick-cube/main/sce_sys/livearea/contents/bg0.png" alt="A Brick Cube" width="820">
</p>

<h1 align="center">A Brick Cube</h1>

<p align="center">
  A voxel open-world game for PlayStation Vita and Nintendo 3DS.<br>
  Juego de mundo abierto voxel para PlayStation Vita y Nintendo 3DS.
</p>

<p align="center">
  <a href="https://github.com/aamiso/A-brick-cube/actions/workflows/build.yml"><img src="https://img.shields.io/github/actions/workflow/status/aamiso/A-brick-cube/build.yml?label=Vita%20build" alt="Vita build"></a>
  <a href="https://github.com/aamiso/A-brick-cube/actions/workflows/build-3ds.yml"><img src="https://img.shields.io/github/actions/workflow/status/aamiso/A-brick-cube/build-3ds.yml?label=3DS%20build" alt="3DS build"></a>
  <img src="https://img.shields.io/badge/status-alpha-orange" alt="Alpha status">
</p>

<p align="center">
  <a href="#the-game">The game</a> ·
  <a href="#features">Features</a> ·
  <a href="#3ds-controls">Controls</a> ·
  <a href="#downloads">Downloads</a> ·
  <a href="#build-from-source">Build</a> ·
  <a href="#español">Español</a>
</p>

---

## The game

A Brick Cube is a voxel open-world game for PlayStation Vita and Nintendo 3DS. Explore a handmade city, drive through its streets, meet its characters, and discover the interiors.

**Short description:** A voxel open-world game for PS Vita and Nintendo 3DS. Explore the city, drive, and take on missions. Story and online play are planned.

The project is in alpha and under active development. Story mode and online play are future plans; they are not available in the current build.

## Features

- A handmade city with neighborhoods, parks, plazas, and voxel buildings.
- Cars, AI traffic, driving, and collisions.
- Characters, dialogue, and delivery, race, and destruction missions.
- Connected interiors with 51 rooms to explore.
- Day and night cycle, clock, minimap, and money.
- A Nintendo 3DS port with a second-screen interface and save games.

## 3DS controls

| Button | On foot | In a car |
|---|---|---|
| Circle Pad | Move | Steer and drive |
| D-pad | Move the camera | — |
| A | Jump | — |
| B | Talk / advance dialogue | — |
| X | Enter a building or car | Get out |
| Y | — | Handbrake |
| R | Run | Accelerate |
| L | Change camera | Brake / reverse |
| SELECT | Save outdoors | — |
| START | Quit | Quit |

The bottom screen shows the minimap, time, money, mission details, and prompts.

## Future plans

A Brick Cube is planned to grow with a story that expands its world and characters, as well as an online mode. These features are planned for the future and are not part of the current build.

## Downloads

- **PlayStation Vita:** [Download the published alpha release](https://github.com/aamiso/A-brick-cube/releases/tag/game). The current release contains a Vita VPK.
- **Nintendo 3DS:** [View 3DS builds on GitHub Actions](https://github.com/aamiso/A-brick-cube/actions/workflows/build-3ds.yml). Successful runs provide a `.3dsx` artifact. Check the Releases page for an official 3DS package.

## Build from source

### PlayStation Vita

Requires VitaSDK and the vitaGL dependencies installed by the project workflow. With VitaSDK set up:

```sh
cmake -S . -B build
cmake --build build
```

See the [Vita build workflow](.github/workflows/build.yml).

### Nintendo 3DS

Requires devkitPro, devkitARM, and libctru:

```sh
cd 3ds
make
```

See the [3DS build workflow](.github/workflows/build-3ds.yml).

## Project status

- **PlayStation Vita:** alpha VPK available.
- **Nintendo 3DS:** port in development, with separate Circle Pad and D-pad controls, a rear-follow camera on foot, and Old 3DS-specific performance adjustments.
- New builds and releases will be shared as they are prepared.

---

## Español

### El juego

A Brick Cube es un juego de mundo abierto voxel para PlayStation Vita y Nintendo 3DS. Explora una ciudad hecha a mano, conduce por sus calles, conoce a sus personajes y descubre los interiores.

**Descripción corta:** Juego de mundo abierto voxel para PS Vita y Nintendo 3DS. Explora la ciudad, conduce y completa misiones. La historia y el modo online están planeados.

El proyecto está en fase alpha y sigue en desarrollo. La historia y el modo online son planes futuros; todavía no están disponibles en la versión actual.

### Características

- Ciudad hecha a mano con barrios, parques, plazas y edificios voxel.
- Autos, tráfico con IA, conducción y choques.
- Personajes, diálogos y misiones de entrega, carreras y destrucción.
- Interiores conectados con 51 habitaciones para explorar.
- Ciclo de día y noche, reloj, minimapa y dinero.
- Port para Nintendo 3DS con interfaz en la segunda pantalla y guardado.

### Controles de 3DS

| Botón | A pie | En un auto |
|---|---|---|
| Circle Pad | Moverse | Girar y conducir |
| Cruceta | Mover la cámara | — |
| A | Saltar | — |
| B | Hablar / avanzar diálogo | — |
| X | Entrar a un edificio o auto | Bajar |
| Y | — | Freno de mano |
| R | Correr | Acelerar |
| L | Cambiar cámara | Frenar / reversa |
| SELECT | Guardar afuera | — |
| START | Salir | Salir |

La pantalla inferior muestra el minimapa, la hora, el dinero, la misión y los avisos.

### Planes futuros

A Brick Cube planea crecer con una historia que amplíe su mundo y sus personajes, además de un modo online. Estas funciones están planeadas para el futuro y todavía no forman parte de la versión actual.

### Descargas

- **PlayStation Vita:** [Descargar la versión alpha publicada](https://github.com/aamiso/A-brick-cube/releases/tag/game). La release actual contiene un archivo VPK para Vita.
- **Nintendo 3DS:** [Ver las compilaciones de 3DS en GitHub Actions](https://github.com/aamiso/A-brick-cube/actions/workflows/build-3ds.yml). Las compilaciones exitosas generan un artefacto `.3dsx`. Revisa Releases para encontrar una versión oficial para 3DS.

### Compilar desde el código fuente

#### PlayStation Vita

Requiere VitaSDK y las dependencias de vitaGL instaladas por el flujo del proyecto. Con VitaSDK configurado:

```sh
cmake -S . -B build
cmake --build build
```

Consulta el [flujo de compilación de Vita](.github/workflows/build.yml).

#### Nintendo 3DS

Requiere devkitPro, devkitARM y libctru:

```sh
cd 3ds
make
```

Consulta el [flujo de compilación de 3DS](.github/workflows/build-3ds.yml).

### Estado del proyecto

- **PlayStation Vita:** VPK alpha disponible.
- **Nintendo 3DS:** port en desarrollo, con controles separados para Circle Pad y cruceta, cámara trasera al caminar y ajustes de rendimiento específicos para Old 3DS.
- Se compartirán nuevas compilaciones y releases cuando estén listas.

---

<p align="center">
  Made with care and lots of cubes · Hecho con cariño y muchos cubitos.
</p>
