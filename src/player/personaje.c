#include "player/personaje.h"
#include "engine/voxel.h"
#include "core/util.h"
#include <vitaGL.h>
#include <math.h>

#define LADO 0.1f   // cada cubito del personaje mide 0.1 (el personaje mide 1.8)

static void pintar(Voxeles *v, unsigned camisa, unsigned pantalon, unsigned piel, unsigned pelo) {
    voxel_color(v, 'c', camisa);
    voxel_color(v, 'n', pantalon);
    voxel_color(v, 'p', piel);
    voxel_color(v, 'h', pelo);
    voxel_color(v, 'e', 0x1b1b22);
    voxel_color(v, 'm', 0xb8484a);
    voxel_color(v, 'k', 0x24242b);
    voxel_color(v, 'b', 0x3a2c22);
}

void personaje_crear(Personaje *p, unsigned camisa, unsigned pantalon, unsigned piel, unsigned pelo) {
    Voxeles v;

    // Cabeza 6 x 6 x 6 (el frente es +z)
    voxel_crear(&v, 6, 6, 6); pintar(&v, camisa, pantalon, piel, pelo);
    voxel_caja(&v, 0, 0, 0, 6, 6, 6, 'p');
    voxel_caja(&v, 0, 5, 0, 6, 1, 6, 'h');           // pelo arriba
    voxel_caja(&v, 0, 4, 0, 6, 1, 2, 'h');           // pelo de atrás
    voxel_caja(&v, 0, 4, 0, 1, 1, 5, 'h');           // patillas
    voxel_caja(&v, 5, 4, 0, 1, 1, 5, 'h');
    voxel_caja(&v, 0, 3, 0, 6, 1, 1, 'h');           // nuca
    voxel_caja(&v, 0, 4, 5, 6, 1, 1, 'h');           // fleco
    voxel_poner(&v, 1, 3, 5, 'e'); voxel_poner(&v, 4, 3, 5, 'e');   // ojos
    voxel_poner(&v, 2, 1, 5, 'm'); voxel_poner(&v, 3, 1, 5, 'm');   // boca
    malla_iniciar(&p->cabeza);
    voxel_malla(&v, LADO, 3.0f, 0.0f, 3.0f, &p->cabeza);
    malla_subir(&p->cabeza);
    voxel_liberar(&v);

    // Torso 8 x 6 x 4
    voxel_crear(&v, 8, 6, 4); pintar(&v, camisa, pantalon, piel, pelo);
    voxel_caja(&v, 0, 0, 0, 8, 6, 4, 'c');
    voxel_caja(&v, 0, 0, 0, 8, 1, 4, 'b');           // cinturón
    voxel_caja(&v, 3, 5, 3, 2, 1, 1, 'p');           // cuello en V
    malla_iniciar(&p->torso);
    voxel_malla(&v, LADO, 4.0f, 0.0f, 2.0f, &p->torso);
    malla_subir(&p->torso);
    voxel_liberar(&v);

    // Brazo 2 x 6 x 2 (el pivote es el hombro, arriba)
    voxel_crear(&v, 2, 6, 2); pintar(&v, camisa, pantalon, piel, pelo);
    voxel_caja(&v, 0, 2, 0, 2, 4, 2, 'c');
    voxel_caja(&v, 0, 0, 0, 2, 2, 2, 'p');
    malla_iniciar(&p->brazo);
    voxel_malla(&v, LADO, 1.0f, 6.0f, 1.0f, &p->brazo);
    malla_subir(&p->brazo);
    voxel_liberar(&v);

    // Pierna 3 x 6 x 3 (el pivote es la cadera, arriba)
    voxel_crear(&v, 3, 6, 3); pintar(&v, camisa, pantalon, piel, pelo);
    voxel_caja(&v, 0, 1, 0, 3, 5, 3, 'n');
    voxel_caja(&v, 0, 0, 0, 3, 1, 3, 'k');
    malla_iniciar(&p->pierna);
    voxel_malla(&v, LADO, 1.5f, 6.0f, 1.5f, &p->pierna);
    malla_subir(&p->pierna);
    voxel_liberar(&v);

    p->listo = 1;
}

void personaje_liberar(Personaje *p) {
    malla_liberar(&p->cabeza); malla_liberar(&p->torso);
    malla_liberar(&p->brazo);  malla_liberar(&p->pierna);
    p->listo = 0;
}

void personaje_dibujar(const Personaje *p, float x, float z, float angulo,
                       float vel, float fase, float tiempo, float elev) {
    if (!p->listo) return;
    float correr = vel > 1.0f ? (vel - 1.0f) / 0.7f : 0.0f;
    if (correr > 1.0f) correr = 1.0f;
    if (vel > 1.0f) vel = 1.0f;
    float aire = elev > 0.03f ? 1.0f : 0.0f;
    float quieto = (1.0f - vel) * (1.0f - aire);
    float paso = sinf(fase);
    float balanceo = paso * (48.0f + 24.0f * correr) * vel * (1.0f - aire * 0.7f);                         // piernas y brazos al caminar
    float rebote = fabsf(paso) * (0.07f + 0.06f * correr) * vel * (1.0f - aire) + elev;                    // sube y baja al caminar
    float respira = sinf(tiempo * 2.2f) * 0.012f * quieto;       // respiración
    float inclina = (5.0f + 11.0f * correr) * vel + aire * 8.0f;                                  // se inclina hacia adelante
    float giro_torso = paso * 7.0f * vel;                        // caderas
    float mira = sinf(tiempo * 0.6f) * 14.0f * quieto;           // mira alrededor en reposo
    float abre = 4.0f + sinf(tiempo * 2.2f) * 1.5f * quieto;     // brazos un poco separados

    float brazos_arriba = aire * 150.0f;
    float piernas_aire = aire * 28.0f;
    glPushMatrix();
    glTranslatef(x, rebote + respira, z);
    glRotatef(angulo, 0.0f, 1.0f, 0.0f);

    // Piernas
    glPushMatrix(); glTranslatef(-0.17f, 0.6f, 0.0f); glRotatef(balanceo + piernas_aire, 1.0f, 0.0f, 0.0f);
    malla_dibujar(&p->pierna); glPopMatrix();
    glPushMatrix(); glTranslatef(0.17f, 0.6f, 0.0f); glRotatef(-balanceo - piernas_aire * 0.5f, 1.0f, 0.0f, 0.0f);
    malla_dibujar(&p->pierna); glPopMatrix();

    // Torso con cabeza y brazos
    glPushMatrix();
    glTranslatef(0.0f, 0.6f, 0.0f);
    glRotatef(inclina, 1.0f, 0.0f, 0.0f);
    glRotatef(giro_torso, 0.0f, 1.0f, 0.0f);
    malla_dibujar(&p->torso);

    glPushMatrix(); glTranslatef(-0.50f, 0.6f, 0.0f);
    glRotatef(-balanceo * 0.9f - brazos_arriba * 0.8f, 1.0f, 0.0f, 0.0f); glRotatef(abre, 0.0f, 0.0f, 1.0f);
    malla_dibujar(&p->brazo); glPopMatrix();
    glPushMatrix(); glTranslatef(0.50f, 0.6f, 0.0f);
    glRotatef(balanceo * 0.9f - brazos_arriba, 1.0f, 0.0f, 0.0f); glRotatef(-abre, 0.0f, 0.0f, 1.0f);
    malla_dibujar(&p->brazo); glPopMatrix();

    glPushMatrix(); glTranslatef(0.0f, 0.6f, 0.0f);
    glRotatef(mira - giro_torso, 0.0f, 1.0f, 0.0f);
    malla_dibujar(&p->cabeza); glPopMatrix();

    glPopMatrix();
    glPopMatrix();
    (void)PI;
}
