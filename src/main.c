// LEGO GTA - Etapa 2: suelo + personajito LEGO + cámara que lo sigue
//
// Controles:
//   Stick izquierdo = caminar
//   Stick derecho   = girar la cámara (izq/der) y subirla o bajarla (arriba/abajo)
//   START           = salir

#include <vitaGL.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <math.h>

#define PI 3.14159265f

// ---------------------------------------------------------------
// 1) El cubo: la "pieza de LEGO" básica con la que se construye todo
// ---------------------------------------------------------------

// Las 8 esquinas de un cubo de tamaño 1 (centrado en el origen)
static const float esquinas[8][3] = {
    {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
    {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f, 0.5f,  0.5f}, {-0.5f, 0.5f,  0.5f}};

// Las 6 caras: frente, atrás, izquierda, derecha, arriba, abajo
static const int caras[6][4] = {
    {4, 5, 6, 7}, {1, 0, 3, 2}, {0, 4, 7, 3},
    {5, 1, 2, 6}, {3, 7, 6, 2}, {0, 1, 5, 4}};

// Qué tan clara se ve cada cara (así se nota el volumen sin luces)
static const float brillo[6] = {0.80f, 0.60f, 0.70f, 0.70f, 1.00f, 0.40f};

#define CARA_ARRIBA 4

static float vertices[36 * 3];

// Convierte las 6 caras en 12 triángulos (36 puntos)
static void construir_cubo(void) {
    const int orden[6] = {0, 1, 2, 0, 2, 3};
    int n = 0;
    for (int c = 0; c < 6; c++) {
        for (int k = 0; k < 6; k++) {
            const float *p = esquinas[caras[c][orden[k]]];
            for (int j = 0; j < 3; j++) vertices[n * 3 + j] = p[j];
            n++;
        }
    }
}

// Dibuja una caja: posición (x,y,z), tamaño (ancho,alto,largo) y color (r,g,b)
static void caja(float x, float y, float z, float sx, float sy, float sz,
                 float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(sx, sy, sz);
    for (int c = 0; c < 6; c++) {
        glColor3f(r * brillo[c], g * brillo[c], b * brillo[c]);
        glDrawArrays(GL_TRIANGLES, c * 6, 6);
    }
    glPopMatrix();
}

// Dibuja solo la tapa de arriba de una caja (para el suelo, es más rápido)
static void baldosa(float x, float y, float z, float sx, float sz,
                    float r, float g, float b) {
    glPushMatrix();
    // La tapa del cubo está 0.5 arriba de su centro; se baja 0.5 para que
    // la superficie del suelo quede justo a la altura "y" (los pies del personaje)
    glTranslatef(x, y - 0.5f, z);
    glScalef(sx, 1.0f, sz);
    glColor3f(r, g, b);
    glDrawArrays(GL_TRIANGLES, CARA_ARRIBA * 6, 6);
    glPopMatrix();
}

// ---------------------------------------------------------------
// 2) El personajito LEGO (hecho con cajas)
// ---------------------------------------------------------------
static void dibujar_personaje(float x, float z, float angulo, float paso) {
    // "paso" mueve las piernas y brazos cuando camina
    float swing = sinf(paso) * 25.0f;

    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef(angulo, 0.0f, 1.0f, 0.0f);

    // Piernas (azules)
    glPushMatrix();
    glTranslatef(-0.18f, 0.55f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    caja(0.0f, -0.275f, 0.0f, 0.3f, 0.55f, 0.3f, 0.10f, 0.25f, 0.75f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.18f, 0.55f, 0.0f);
    glRotatef(-swing, 1.0f, 0.0f, 0.0f);
    caja(0.0f, -0.275f, 0.0f, 0.3f, 0.55f, 0.3f, 0.10f, 0.25f, 0.75f);
    glPopMatrix();

    // Cuerpo (rojo)
    caja(0.0f, 0.90f, 0.0f, 0.75f, 0.70f, 0.40f, 0.85f, 0.10f, 0.10f);

    // Brazos (rojos)
    glPushMatrix();
    glTranslatef(-0.50f, 1.20f, 0.0f);
    glRotatef(-swing, 1.0f, 0.0f, 0.0f);
    caja(0.0f, -0.30f, 0.0f, 0.25f, 0.60f, 0.25f, 0.85f, 0.10f, 0.10f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.50f, 1.20f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    caja(0.0f, -0.30f, 0.0f, 0.25f, 0.60f, 0.25f, 0.85f, 0.10f, 0.10f);
    glPopMatrix();

    // Cabeza (amarilla) y el botoncito de arriba, como en las figuras LEGO
    caja(0.0f, 1.50f, 0.0f, 0.50f, 0.50f, 0.50f, 1.00f, 0.80f, 0.00f);
    caja(0.0f, 1.82f, 0.0f, 0.22f, 0.14f, 0.22f, 1.00f, 0.80f, 0.00f);

    // Ojitos (negros) en la cara de adelante
    caja(-0.12f, 1.55f, 0.26f, 0.08f, 0.08f, 0.02f, 0.05f, 0.05f, 0.05f);
    caja( 0.12f, 1.55f, 0.26f, 0.08f, 0.08f, 0.02f, 0.05f, 0.05f, 0.05f);

    glPopMatrix();
}

// ---------------------------------------------------------------
// 3) El suelo: baldosas verdes en cuadrícula
// ---------------------------------------------------------------
#define MITAD 10   // 10 baldosas hacia cada lado = 20x20 en total
#define TAM   3.0f // tamaño de cada baldosa

static void dibujar_suelo(void) {
    for (int i = -MITAD; i < MITAD; i++) {
        for (int j = -MITAD; j < MITAD; j++) {
            int par = ((i + j) & 1) == 0;
            float g = par ? 0.65f : 0.50f;
            baldosa((i + 0.5f) * TAM, 0.0f, (j + 0.5f) * TAM, TAM, TAM,
                    0.20f, g, 0.20f);
        }
    }
}

// ---------------------------------------------------------------
// 4) Leer un stick: devuelve un número de -1 a 1 (con zona muerta)
// ---------------------------------------------------------------
static float stick(unsigned char valor) {
    float v = ((float)valor - 128.0f) / 128.0f;
    if (fabsf(v) < 0.15f) return 0.0f; // los sticks nunca quedan en 0 exacto
    return v;
}

// ---------------------------------------------------------------
// 5) El programa principal
// ---------------------------------------------------------------
int main(void) {
    vglInit(0x800000);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    construir_cubo();

    float px = 0.0f, pz = 0.0f;       // posición del personaje
    float angulo = 0.0f;              // hacia dónde mira el personaje
    float paso = 0.0f;                // animación de caminar
    float giro_cam = 0.0f;            // giro de la cámara (grados)
    float alto_cam = 25.0f;           // inclinación de la cámara (grados)
    const float distancia_cam = 9.0f; // qué tan lejos está la cámara

    while (1) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_START) break;

        float lx = stick(pad.lx), ly = stick(pad.ly);
        float rx = stick(pad.rx), ry = stick(pad.ry);

        // Mover la cámara con el stick derecho
        giro_cam += rx * 2.5f;
        alto_cam += ry * 1.5f;
        if (alto_cam < 5.0f)  alto_cam = 5.0f;
        if (alto_cam > 70.0f) alto_cam = 70.0f;

        // Caminar con el stick izquierdo (relativo a hacia dónde mira la cámara)
        float rad = giro_cam * PI / 180.0f;
        float adelante_x = sinf(rad), adelante_z = -cosf(rad);
        float derecha_x = cosf(rad),  derecha_z = sinf(rad);

        float dx = derecha_x * lx + adelante_x * (-ly);
        float dz = derecha_z * lx + adelante_z * (-ly);
        float fuerza = sqrtf(dx * dx + dz * dz);

        if (fuerza > 0.01f) {
            if (fuerza > 1.0f) { dx /= fuerza; dz /= fuerza; fuerza = 1.0f; }
            px += dx * 0.15f;
            pz += dz * 0.15f;
            angulo = atan2f(dx, dz) * 180.0f / PI;
            paso += 0.25f * fuerza;
        } else {
            paso = 0.0f;
        }

        // No dejar que se salga del suelo
        float limite = MITAD * TAM - 0.5f;
        if (px >  limite) px =  limite;
        if (px < -limite) px = -limite;
        if (pz >  limite) pz =  limite;
        if (pz < -limite) pz = -limite;

        // --- Dibujar ---
        glViewport(0, 0, 960, 544);
        glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-0.96f, 0.96f, -0.544f, 0.544f, 1.0f, 200.0f);

        // La cámara: se aleja, se inclina, gira y mira al personaje
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -distancia_cam);
        glRotatef(alto_cam, 1.0f, 0.0f, 0.0f);
        glRotatef(giro_cam, 0.0f, 1.0f, 0.0f);
        glTranslatef(-px, -1.0f, -pz);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices);

        dibujar_suelo();
        dibujar_personaje(px, pz, angulo, paso);

        glDisableClientState(GL_VERTEX_ARRAY);

        vglSwapBuffers(GL_FALSE);
    }

    sceKernelExitProcess(0);
    return 0;
}
