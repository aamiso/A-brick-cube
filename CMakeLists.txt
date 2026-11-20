cmake_minimum_required(VERSION 3.5)

# Le dice a CMake dónde está VitaSDK (ya viene instalado en GitHub)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(legogta C)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

# Nombre y datos del juego en la Vita
set(VITA_APP_NAME "LEGO GTA")
set(VITA_TITLEID  "LEGOGTA01")
set(VITA_VERSION  "00.10")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -O2 -std=gnu99")

add_executable(${PROJECT_NAME} src/main.c)

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
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
)
