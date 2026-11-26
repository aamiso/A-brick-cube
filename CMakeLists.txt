cmake_minimum_required(VERSION 3.5)

# Le dice a CMake dónde está VitaSDK (ya viene instalado en GitHub)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(a_brick_cube C)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

# Nombre y datos del juego en la Vita
set(VITA_APP_NAME "A Brick Cube")
set(VITA_TITLEID  "ABCV00001")
set(VITA_VERSION  "00.80")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -O2 -std=gnu99")

# Busca solo todos los archivos .c dentro de src/ (y sus subcarpetas).
# Así, cuando agreguemos archivos nuevos, no hay que tocar este archivo.
file(GLOB_RECURSE JUEGO_SRC ${CMAKE_SOURCE_DIR}/src/*.c)

add_executable(${PROJECT_NAME} ${JUEGO_SRC})

# Permite escribir #include "engine/cubo.h" desde cualquier archivo
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Librerías que necesita vitaGL para dibujar en 3D
target_link_libraries(${PROJECT_NAME}
  vitaGL
  vitashark
  mathneon
  SceShaccCgExt
  taihen_stub
  SceShaccCg_stub
  SceGxm_stub
  SceDisplay_stub
  SceCtrl_stub
  SceSysmodule_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceCommonDialog_stub
  SceKernelDmacMgr_stub
  stdc++
  m
)
# Crea el .vpk (el archivo que se instala en la Vita o se abre en el emulador)
vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})

# Archivos que se meten dentro del .vpk (el mapa de la ciudad)
set(VPK_ARCHIVOS "")
if(EXISTS ${CMAKE_SOURCE_DIR}/data/mapas/ciudad.txt)
  list(APPEND VPK_ARCHIVOS FILE ${CMAKE_SOURCE_DIR}/data/mapas/ciudad.txt data/mapas/ciudad.txt)
endif()

# Los modelos voxel (casas, banco, café)
file(GLOB MODELOS_VOXEL ${CMAKE_SOURCE_DIR}/data/modelos/*.txt)
foreach(modelo ${MODELOS_VOXEL})
  get_filename_component(nombre_modelo ${modelo} NAME)
  list(APPEND VPK_ARCHIVOS FILE ${modelo} data/modelos/${nombre_modelo})
endforeach()

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  ${VPK_ARCHIVOS}
)
