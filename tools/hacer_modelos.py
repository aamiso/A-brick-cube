#!/usr/bin/env python3
# Crea los modelos voxel hechos a mano (uno por lugar). Uso: python3 hacer_modelos.py  (escribe en ../data/modelos/)
import os, random
SAL = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'data', 'modelos')

FONT = {'H':["X.X","X.X","XXX","X.X","X.X"],'O':["XXX","X.X","X.X","X.X","XXX"],'S':["XXX","X..","XXX","..X","XXX"],
 'P':["XXX","X.X","XXX","X..","X.."],'I':["X","X","X","X","X"],'T':["XXX",".X.",".X.",".X.",".X."],
 'A':[".X.","X.X","XXX","X.X","X.X"],'L':["X..","X..","X..","X..","XXX"],'E':["XXX","X..","XXX","X..","XXX"],
 'R':["XX.","X.X","XX.","X.X","X.X"],'F':["XXX","X..","XX.","X..","X.."],'M':["X.X","XXX","XXX","X.X","X.X"],
 'C':["XXX","X..","X..","X..","XXX"],'B':["XX.","X.X","XX.","X.X","XX."],'U':["X.X","X.X","X.X","X.X","XXX"],'N':["XX.X","X.XX","X..X","X..X","X..X"]}

class V:
    def __init__(s, nombre, W, D, H, colores):
        s.n, s.W, s.D, s.H, s.col = nombre, W, D, H, colores
        s.g = [[['.'] * W for _ in range(D)] for _ in range(H)]
    def put(s, x, y, z, c):
        if 0 <= x < s.W and 0 <= y < s.H and 0 <= z < s.D: s.g[y][z][x] = c
    def get(s, x, y, z):
        return s.g[y][z][x] if 0 <= x < s.W and 0 <= y < s.H and 0 <= z < s.D else '.'
    def box(s, x, y, z, w, h, d, c):
        for yy in range(y, y + h):
            for zz in range(z, z + d):
                for xx in range(x, x + w): s.put(xx, yy, zz, c)
    def texto(s, x, y, z, t, c, lado='f'):      # letras en la pared del frente (z fijo); y = fila de abajo
        cx = x
        for ch in t:
            f = FONT[ch]
            for r in range(5):
                for k, p in enumerate(f[r]):
                    if p == 'X': s.put(cx + k, y + 4 - r, z, c)
            cx += len(f[0]) + 1
    def ancho_texto(s, t): return sum(len(FONT[c][0]) + 1 for c in t) - 1
    def guardar(s):
        with open(os.path.join(SAL, s.n + '.txt'), 'w') as f:
            f.write('modelo %s\n// Hecho con tools/hacer_modelos.py\n' % s.n.capitalize())
            for k, v in s.col.items(): f.write('color %s #%s\n' % (k, v))
            for y in range(s.H):
                f.write('capa %d\n' % (y + 1))
                for z in range(s.D): f.write(''.join(s.g[y][z]) + '\n')

def suelo(v, borde=2):
    v.box(0, 0, 0, v.W, 1, v.D, 'G')
    v.box(borde, 0, borde, v.W - 2 * borde, 1, v.D - 2 * borde, 'P')

# ---------------- HOSPITAL (22 x 16 celdas) ----------------
def hospital():
    v = V('hospital', 44, 32, 30, dict(P='e3d4b4', G='6bbf4a', C='efe6cf', W='ffffff', V='6f9fd8', v='4f7fc0', S='b8b2a4', k='25272c', Y='ffd23a',
        B='2f62c9', R='d9382c', D='6b5a3c', T='8a6a42', L='3f9a3a', g='7f7f86'))
    suelo(v)
    def ala(x0, w, z0, d):
        for f in range(3):
            y0 = 1 + f * 7
            v.box(x0, y0, z0, w, 2, d, 'C')                  # franja crema
            v.box(x0, y0 + 2, z0, w, 4, d, 'V')              # ventanal
            v.box(x0, y0 + 6, z0, w, 1, d, 'W')              # cornisa blanca
        # divisiones de vidrio (cada 3 cubitos una línea más oscura)
        for f in range(3):
            for xx in range(x0 + 1, x0 + w, 3):
                for zz in (z0, z0 + d - 1):
                    for yy in range(1 + f * 7 + 2, 1 + f * 7 + 6): v.put(xx, yy, zz, 'v')
        for f in range(3):
            for zz in range(z0 + 1, z0 + d, 3):
                for xx in (x0, x0 + w - 1):
                    for yy in range(1 + f * 7 + 2, 1 + f * 7 + 6): v.put(xx, yy, zz, 'v')
    ala(2, 5, 4, 20); ala(37, 5, 4, 20)       # alas (salen al frente)
    ala(7, 30, 4, 16)                         # cuerpo central (más atrás)
    # planta baja oscura en el centro + puerta
    v.box(7, 1, 19, 30, 6, 1, 'W')
    v.box(18, 1, 19, 8, 5, 1, 'k'); v.box(19, 1, 19, 6, 4, 1, 'V')   # puertas de vidrio
    # techo
    v.box(2, 22, 4, 5, 1, 20, 'W'); v.box(37, 22, 4, 5, 1, 20, 'W'); v.box(7, 22, 4, 30, 1, 16, 'W')
    for (x0, w, z0, d) in ((2, 5, 4, 20), (37, 5, 4, 20), (7, 30, 4, 16)):
        v.box(x0, 23, z0, w, 1, d, 'W')
        v.box(x0 + 1, 23, z0 + 1, w - 2, 1, d - 2, '.')
    v.box(8, 22, 5, 28, 1, 14, 'D')
    # helipuerto
    v.box(18, 23, 6, 12, 1, 12, 'k')
    v.box(22, 23, 8, 1, 1, 8, 'W'); v.box(26, 23, 8, 1, 1, 8, 'W'); v.box(22, 23, 11, 5, 1, 1, 'W')
    # cuarto de techo azul (como la referencia)
    v.box(9, 23, 6, 6, 3, 6, 'k'); v.box(9, 26, 6, 6, 2, 6, 'B'); v.box(9, 24, 12, 1, 2, 1, 'R')
    # cartel HOSPITAL
    v.box(7, 8, 19, 30, 6, 1, 'k')
    v.texto(7 + (30 - v.ancho_texto('HOSPITAL')) // 2, 9, 19, 'HOSPITAL', 'Y')
    # techo de entrada con 4 columnas
    v.box(12, 7, 20, 20, 1, 8, 'W')
    for x in (13, 30):
        v.box(x, 1, 26, 2, 6, 2, 'W')
    # escalones
    v.box(15, 1, 28, 14, 1, 2, 'S'); v.box(17, 1, 30, 10, 1, 1, 'S')
    # cruz roja junto a la puerta
    v.box(9, 4, 19, 3, 1, 1, 'R'); v.box(10, 3, 19, 1, 3, 1, 'R')
    # árboles del frente
    for x in (4, 40):
        v.box(x, 1, 28, 1, 2, 1, 'T'); v.box(x - 1, 3, 27, 3, 2, 3, 'L')
    v.guardar()

# ---------------- EDIFICIO ROJO con escalera de incendios (9 x 9 celdas) ----------------
def ladrillo():
    random.seed(5)
    v = V('edificio_rojo', 18, 18, 38, dict(P='cfc9bb', G='6bbf4a', r='8b2a24', q='7a231e', s='a23a30', V='25324f', v='4a6a9c', k='1c1c20', m='8c9096',
        n='a9aeb4', W='e6e0cc', c='d9d2b8', w='ffffff', O='e8a02c'))
    v.box(0, 0, 0, 18, 1, 18, 'P')
    v.box(1, 1, 2, 16, 35, 14, 'r')
    for y in range(1, 36):                                   # ladrillos con manchas
        for z in range(2, 16):
            for x in range(1, 17):
                if v.get(x, y, z) == 'r' and (x in (1, 16) or z in (2, 15)):
                    q = random.random()
                    if q < 0.12: v.put(x, y, z, 'q')
                    elif q < 0.18: v.put(x, y, z, 's')
    # frente z=15: ventanas por piso
    for f in range(4):
        y0 = 9 + f * 7
        for x0 in (3, 8, 12):
            v.box(x0, y0, 15, 2, 4, 1, 'V'); v.put(x0, y0 + 3, 15, 'v'); v.put(x0 + 1, y0 + 3, 15, 'v')
            v.box(x0 - 1, y0 - 1, 15, 4, 1, 1, 'c')           # alféizar
    # persiana metálica de la planta baja
    v.box(2, 1, 15, 10, 7, 1, 'm')
    for y in range(2, 8, 2): v.box(2, y, 15, 10, 1, 1, 'n')
    v.box(13, 1, 15, 3, 6, 1, 'k'); v.box(14, 2, 15, 1, 3, 1, 'v')   # puerta
    # escalera de incendios (negra) saliendo del frente
    for f in range(4):
        y0 = 8 + f * 7
        v.box(1, y0, 16, 16, 1, 2, 'k')                        # plataforma
        v.box(1, y0 + 1, 17, 16, 1, 1, 'k')                    # baranda
        for x in (1, 16): v.box(x, y0 + 1, 16, 1, 2, 2, 'k')
        # escalera diagonal al siguiente piso
        for k in range(6):
            v.put(10 + (k % 5), y0 + 1 + k, 16 + 1, 'k') if False else None
        for k in range(6):
            v.put(5 + k, y0 - 6 + k, 17, 'k')
    v.box(2, 8, 17, 1, 1, 1, 'k')
    # cajitas de aire acondicionado y cajas
    v.box(9, 12, 16, 2, 2, 1, 'w'); v.box(5, 19, 16, 2, 2, 1, 'w'); v.box(11, 26, 16, 2, 2, 1, 'w')
    # techo con borde negro, aires y chimenea
    v.box(1, 36, 2, 16, 1, 14, 'k'); v.box(2, 36, 3, 14, 1, 12, 'r')
    v.box(2, 37, 3, 14, 1, 1, 'k'); v.box(2, 37, 14, 14, 1, 1, 'k'); v.box(2, 37, 3, 1, 1, 12, 'k'); v.box(15, 37, 3, 1, 1, 12, 'k')
    v.box(4, 37, 5, 5, 2, 4, 'W'); v.box(10, 37, 6, 4, 2, 5, 'W'); v.box(5, 39, 6, 3, 1, 2, 'c')
    v.box(14, 37, 10, 1, 4, 1, 'k')
    # tubería lateral
    v.box(0, 1, 15, 1, 35, 1, 'k')
    v.guardar()

# ---------------- TORRE DORADA (14 x 14 celdas) ----------------
def torre_dorada():
    v = V('torre_dorada', 28, 28, 78, dict(P='d9b36a', G='6bbf4a', o='e89a1c', O='f2b01e', Y='ffd92e', y='fff27a', d='b5651d', D='8c4a14', k='3a2410', g='a8761f'))
    v.box(0, 0, 0, 28, 1, 28, 'P'); v.box(0, 0, 0, 28, 1, 2, 'G')
    # núcleo central
    v.box(7, 1, 7, 14, 62, 14, 'O')
    v.box(9, 62, 9, 10, 8, 10, 'Y'); v.box(8, 70, 8, 12, 2, 12, 'o'); v.box(10, 72, 10, 8, 1, 8, 'y')
    # contrafuertes: cuatro por lado, de distinta altura
    alt = [26, 40, 52, 34]
    for i, x in enumerate((3, 8, 14, 20)):
        v.box(x, 1, 3, 4, alt[i], 4, 'd'); v.box(x, 1, 21, 4, alt[(i + 2) % 4], 4, 'd')
    for i, z in enumerate((8, 14)):
        v.box(3, 1, z, 4, alt[i + 1] - 6, 4, 'd'); v.box(21, 1, z, 4, alt[i] - 4, 4, 'd')
    # franjas oscuras horizontales y ventanas encendidas en el núcleo
    for y in range(6, 62, 8):
        v.box(7, y, 7, 14, 1, 14, 'g')
    for y in range(10, 60, 8):
        for x in (10, 14, 18):
            v.box(x, y, 6, 2, 4, 1, 'k'); v.put(x, y + 3, 6, 'y')
    # rascacielos: luz grande al frente arriba
    v.box(9, 55, 6, 10, 5, 1, 'Y'); v.box(11, 56, 5, 6, 3, 1, 'y')
    # puntitos luminosos en los contrafuertes
    for (x, y, z) in ((4, 20, 2), (9, 34, 2), (15, 46, 2), (21, 28, 2), (3, 16, 9), (24, 22, 9), (4, 30, 15)):
        v.put(x, y, z, 'y')
    # entrada con marquesina
    v.box(9, 1, 6, 10, 6, 1, 'k'); v.box(8, 7, 3, 12, 1, 4, 'D'); v.box(11, 1, 6, 6, 5, 1, 'y')
    v.box(12, 1, 2, 4, 1, 4, 'g')
    v.guardar()

# ---------------- FARMACIA (12 x 10 celdas) ----------------
def farmacia():
    v = V('farmacia', 24, 20, 20, dict(P='e3d4b4', G='6bbf4a', W='ffffff', w='e8f0ee', V='a9e0f0', v='7cc6dc', g='2fa86a', h='1f7a4d', k='25272c', R='d9382c', Y='fff6b0', L='3f9a3a', T='6b4a2f'))
    suelo(v)
    v.box(2, 1, 3, 20, 10, 14, 'w')                             # cuerpo
    v.box(2, 1, 16, 20, 7, 1, 'V')                              # vitrina
    for x in range(3, 22, 4): v.box(x, 1, 16, 1, 7, 1, 'W')
    v.box(9, 1, 16, 6, 6, 1, 'k'); v.box(10, 1, 16, 4, 5, 1, 'V') # puerta
    v.box(2, 8, 16, 20, 2, 1, 'g')                              # franja verde
    v.box(1, 7, 16, 22, 1, 3, 'h'); v.box(1, 8, 17, 22, 1, 2, 'g')   # toldo verde
    for x in range(1, 23, 2): v.box(x, 7, 18, 1, 1, 1, 'W')
    v.box(1, 11, 2, 22, 1, 16, 'W'); v.box(2, 11, 3, 20, 1, 14, '.'); v.box(2, 11, 3, 20, 1, 14, 'w')  # techo
    # cruz verde grande sobre el techo (cartel)
    v.box(10, 12, 15, 4, 1, 1, 'W')
    v.box(9, 12, 16, 6, 6, 1, 'W'); v.box(9, 14, 16, 6, 2, 1, 'g'); v.box(11, 12, 16, 2, 6, 1, 'g')
    v.box(10, 12, 16, 1, 1, 1, 'W') if False else None
    v.texto(3, 3 + 7, 3, 'RX', 'g') if False else None
    v.box(3, 12, 8, 4, 2, 3, 'h')                               # aire
    v.box(2, 1, 17, 1, 1, 2, 'W')
    # arbolitos
    for x in (1, 22):
        v.box(x, 1, 18, 1, 2, 1, 'T'); v.box(x - 1, 3, 17, 3, 2, 3, 'L')
    v.guardar()

# ---------------- HOTEL (16 x 14 celdas) ----------------
def hotel():
    v = V('hotel', 32, 28, 40, dict(P='e3d4b4', G='6bbf4a', B='d8b98a', b='b9976a', W='ffffff', V='7fb6d8', v='5a93bb', k='25272c', R='b22f3a', r='8a1f2a', Y='ffd23a', L='3f9a3a', T='6b4a2f', g='7a7a80', M='8a5a3c'))
    suelo(v)
    v.box(3, 1, 4, 26, 28, 18, 'B')
    for f in range(4):
        y0 = 7 + f * 6
        for x0 in range(5, 27, 5):
            v.box(x0, y0, 22, 3, 3, 1, 'V'); v.put(x0 + 1, y0 + 1, 22, 'v')       # ventanas
            v.box(x0 - 1, y0 - 1, 22, 5, 1, 1, 'W')                             # balcón
            v.box(x0 - 1, y0 - 1, 23, 5, 1, 1, 'W'); v.box(x0 - 1, y0, 23, 5, 1, 1, 'g') if False else None
            v.box(x0 - 1, y0, 23, 1, 1, 1, 'W'); v.box(x0 + 3, y0, 23, 1, 1, 1, 'W')
        v.box(3, y0 + 3, 22, 26, 1, 1, 'b')
    # planta baja oscura y entrada
    v.box(3, 1, 22, 26, 5, 1, 'b')
    v.box(12, 1, 22, 8, 5, 1, 'k'); v.box(13, 1, 22, 6, 4, 1, 'V')
    v.box(9, 6, 22, 14, 1, 7, 'R'); v.box(9, 7, 28, 14, 1, 1, 'r')           # marquesina roja
    v.box(10, 1, 28, 1, 5, 1, 'W'); v.box(21, 1, 28, 1, 5, 1, 'W')
    v.box(11, 1, 24, 10, 1, 5, 'M')                                          # alfombra
    # techo con cartel HOTEL
    v.box(3, 29, 4, 26, 1, 18, 'W'); v.box(4, 29, 5, 24, 1, 16, 'g')
    v.box(3, 30, 4, 26, 1, 1, 'W'); v.box(3, 30, 21, 26, 1, 1, 'W'); v.box(3, 30, 4, 1, 1, 18, 'W'); v.box(28, 30, 4, 1, 1, 18, 'W')
    v.box(6, 30, 14, 20, 7, 1, 'k'); v.texto(6 + (20 - v.ancho_texto('HOTEL')) // 2 if False else 8, 31, 14, 'HOTEL', 'Y') if False else None
    v.box(8, 30, 14, 16, 7, 1, 'k')
    v.texto(8 + (16 - v.ancho_texto('HOTEL')) // 2, 31, 14, 'HOTEL', 'Y')
    for x in (9, 22): v.box(x, 30, 13, 1, 1, 1, 'g')
    for x in (3, 28):
        v.box(x, 1, 25, 1, 2, 1, 'T'); v.box(x - 1, 3, 24, 3, 2, 3, 'L')
    v.guardar()


# ---------------- TORRES GEMELAS con puente (20 x 15 celdas) ----------------
def gemelas():
    v = V('torres_gemelas', 40, 30, 40, dict(P='d8d4c8', G='3fae3a', w='e9e9f2', W='ffffff', g='b9b9c6', V='2f58c8', v='4a78e0', k='25272c', m='8c8c94', Y='f4e04a', h='9aa0aa'))
    v.box(0, 0, 0, 40, 1, 30, 'G'); v.box(2, 0, 4, 36, 1, 24, 'P')
    def ala(x0):
        v.box(x0, 1, 4, 14, 31, 22, 'w')
        for f in range(4):                                   # 4 pisos de ventanas azules
            y0 = 4 + f * 7
            for z0 in (7, 12, 17):
                v.box(x0 + 3, y0, 26, 3, 4, 1, 'V') if False else None
            for xx in range(x0 + 3, x0 + 12, 4):
                v.box(xx, y0, 26 - 1 + 1, 3, 4, 1, 'V') if False else None
        for f in range(4):
            y0 = 3 + f * 7
            for xx in (x0 + 4, x0 + 9):
                v.box(xx, y0, 26, 3, 4, 1, 'V'); v.box(xx - 1, y0 - 1, 26, 5, 1, 1, 'W'); 
            for zz in (7, 12, 17, 22):
                v.box(x0 + (14 if x0 == 0 else -1), y0, zz, 1, 4, 3, 'V')
                v.box(x0 + (-1 if x0 == 0 else 14), y0, zz, 1, 4, 3, 'V')
            v.box(x0 - 1 if x0 else x0, y0 + 4, 4, 15, 1, 23, 'W') if False else None
            v.box(x0, y0 + 4, 4, 14, 1, 22, 'g')
        for z in range(5, 26, 2): v.box(x0 + 13 if x0 == 0 else x0, 1, z, 1, 30, 1, 'g')   # aletas laterales
        v.box(x0, 32, 4, 14, 1, 22, 'W'); v.box(x0 + 1, 32, 5, 12, 1, 20, 'k')
        v.box(x0 + 3, 32, 9, 8, 1, 8, 'Y'); v.box(x0 + 4, 32, 10, 6, 1, 6, 'k'); v.box(x0 + 5, 32, 12, 4, 1, 1, 'Y')   # helipuerto
        v.box(x0 + 3, 33, 21, 5, 3, 4, 'm'); v.box(x0 + 4, 33, 25, 3, 2, 1, 'k')
        v.box(x0 + 5, 1, 26, 3, 3, 1, 'k')                    # puerta
    ala(2); ala(24)
    # puente central con losas abiertas
    for f in range(4):
        y0 = 4 + f * 7
        v.box(16, y0 + 3, 9, 8, 1, 12, 'w'); v.box(16, y0, 9, 8, 1, 12, 'g') if f else None
        v.box(16, y0 + 1, 9, 8, 1, 1, 'h'); v.box(16, y0 + 1, 20, 8, 1, 1, 'h')
        for z in (10, 14, 18): v.box(16, y0 + 1, z, 8, 2, 1, 'm') if False else None
    v.box(16, 1, 9, 8, 1, 12, 'P')
    v.guardar()

# ---------------- OFICINA MODERNA blanca con vidrio (16 x 14 celdas) ----------------
def oficina():
    v = V('oficina_moderna', 32, 28, 34, dict(P='dcd8cc', p='c0bcb0', G='4aa83a', L='3f9a3a', T='6b4a2f', l='8ccf3a', W='f7f7f5', w='e1e1de', V='9fd0c4', v='6aa59a', k='25272c', O='d8a24a', g='3a3f46'))
    v.box(0, 0, 0, 32, 1, 28, 'P')
    for x in range(0, 32, 2):
        for z in range(0, 28, 2):
            if (x + z) % 4 == 0: v.box(x, 0, z, 2, 1, 2, 'p')
    v.box(3, 1, 4, 26, 28, 18, 'W')                             # bloque blanco
    v.box(15, 1, 4, 14, 28, 18, 'V')                            # fachada de vidrio (derecha)
    for yy in range(4, 28, 6): v.box(15, yy, 22, 14, 1, 1, 'v')
    for xx in range(15, 29, 3): v.box(xx, 1, 22, 1, 28, 1, 'v')
    v.box(3, 1, 4, 12, 28, 18, 'W')
    v.box(3, 1, 22, 12, 28, 1, 'w')
    for xx in range(4, 14, 3): v.box(xx, 24, 22, 2, 1, 1, 'g')   # ventanitas altas
    # portal de entrada (marco hundido)
    v.box(5, 1, 22, 8, 12, 1, 'V'); v.box(4, 1, 22, 1, 13, 2, 'W'); v.box(13, 1, 22, 1, 13, 2, 'W'); v.box(4, 13, 22, 10, 2, 2, 'W')
    v.box(7, 1, 22, 4, 7, 1, 'g'); v.box(8, 1, 22, 2, 6, 1, 'O'); v.box(7, 8, 23, 4, 1, 1, 'O')
    # techo con huecos
    v.box(3, 29, 4, 26, 1, 18, 'W'); v.box(6, 29, 7, 8, 1, 6, 'w'); v.box(18, 29, 10, 8, 1, 8, 'w')
    v.box(3, 30, 4, 26, 1, 2, 'W'); v.box(3, 30, 20, 26, 1, 2, 'W'); v.box(3, 30, 4, 2, 1, 18, 'W'); v.box(27, 30, 4, 2, 1, 18, 'W')
    v.box(14, 30, 8, 6, 1, 2, 'W'); v.box(8, 31, 6, 10, 1, 1, 'W') if False else None
    # árboles a los lados y arbustos
    def arbol(x, z):
        v.box(x, 1, z, 2, 4, 2, 'T'); v.box(x - 2, 5, z - 2, 6, 5, 6, 'L'); v.box(x - 1, 10, z - 1, 4, 2, 4, 'l'); v.box(x - 3, 6, z, 2, 3, 3, 'l')
    arbol(0, 24); arbol(29, 25)
    for x in (6, 24):
        v.box(x, 1, 25, 4, 1, 2, 'L')
    v.guardar()

# ---------------- EDIFICIO COLORIDO de 5 pisos (12 x 12 celdas) ----------------
def colorido():
    v = V('edificio_colorido', 24, 24, 36, dict(P='b8b8bc', p='d6d6da', B='f2cfb8', Y='e8e033', C='5fd9c8', M='c47ab8', m='e8a0d0', k='1f2024', O='c98a3a', W='ffffff', g='8c8c92', G='6a6a70'))
    v.box(0, 0, 0, 24, 1, 24, 'P'); v.box(1, 0, 1, 22, 1, 22, 'p')
    v.box(3, 1, 3, 18, 28, 18, 'B')
    for f in range(5):
        y0 = 1 + f * 6
        v.box(2, y0 + 5, 2, 20, 1, 20, 'Y')                   # losas amarillas
        if f < 4:
            for xx in range(4, 20, 4):
                v.box(xx, y0 + 1, 21, 2, 4, 1, 'k'); v.box(xx - 1, y0 + 1, 21, 1, 4, 1, 'M'); v.box(xx + 2, y0 + 1, 21, 1, 4, 1, 'M')
                v.box(xx - 1, y0 + 5, 22, 4, 1, 1, 'C')       # toldos turquesa
            for zz in (6, 11, 16):
                v.box(21, y0 + 1, zz, 1, 4, 3, 'k'); v.box(21, y0 + 1, zz - 1, 1, 4, 1, 'M')
    # planta baja con puerta
    v.box(9, 1, 21, 6, 5, 1, 'O'); v.box(10, 1, 21, 4, 4, 1, 'k')
    for x in (8, 15): v.box(x, 1, 22, 1, 5, 1, 'Y')
    # piso 5 retranqueado con terraza rosa
    v.box(3, 25, 3, 18, 1, 18, 'g')
    v.box(3, 26, 3, 18, 1, 1, 'm'); v.box(3, 26, 20, 18, 1, 1, 'm'); v.box(3, 26, 3, 1, 1, 18, 'm'); v.box(20, 26, 3, 1, 1, 18, 'm')
    v.box(6, 26, 3, 12, 6, 10, 'B')
    for xx in (7, 10, 13, 16):
        v.box(xx, 27, 13, 2, 4, 1, 'k'); v.box(xx, 31, 13, 2, 1, 1, 'C')
    v.box(5, 32, 2, 14, 1, 13, 'Y'); v.box(6, 33, 3, 12, 1, 11, 'W')
    # techo: cajas grises
    v.box(7, 34, 4, 3, 1, 3, 'G'); v.box(12, 34, 5, 4, 2, 3, 'g'); v.box(16, 34, 9, 3, 1, 3, 'G'); v.box(10, 34, 9, 3, 2, 2, 'k')
    v.guardar()

# ---------------- CENTRO COMERCIAL azul (14 x 12 celdas) ----------------
def comercial():
    v = V('centro_comercial', 28, 24, 24, dict(P='cfcfd2', p='9a9aa0', B='7d9fd8', b='5d7fba', k='2a2c33', W='ffffff', V='bfe8c8', v='8fcf9f', Y='ffe9a0', O='8a6a3c', N='2a60d0', g='d5dae2', t='aeb4bd'))
    v.box(0, 0, 0, 28, 1, 24, 'P')
    v.box(2, 1, 3, 24, 8, 17, 'B')                            # planta baja
    v.box(2, 1, 3, 24, 2, 17, 'k')
    v.box(2, 9, 3, 24, 2, 18, 'k')                            # faja negra
    v.box(8, 11, 3, 18, 8, 12, 'B')                           # piso alto retranqueado
    # cartel SHOPPING (blanco con letras negras)
    v.box(4, 6, 20, 20, 3, 1, 'W') if False else None
    v.box(3, 5, 20, 22, 5, 1, 'W')
    v.texto(3 + (22 - v.ancho_texto('SHOP')) // 2, 5, 20, 'SHOP', 'k')
    # ventanas planta baja y puerta con luz
    for x0 in (3, 20):
        v.box(x0, 2, 20, 5, 3, 1, 'V'); v.box(x0 - 1, 1, 20, 7, 1, 1, 'W') if False else None
        v.box(x0 - 1, 2, 20, 1, 4, 1, 'W'); v.box(x0 + 5, 2, 20, 1, 4, 1, 'W')
    v.box(11, 1, 20, 6, 5, 1, 'Y'); v.box(12, 1, 20, 4, 4, 1, 'v'); v.box(13, 1, 20, 2, 4, 1, 'k') if False else None
    # terraza con baranda de vidrio
    v.box(2, 11, 15, 24, 1, 6, 'g')
    for x in range(2, 26, 4): v.box(x, 12, 20, 1, 2, 1, 'W')
    v.box(2, 12, 20, 24, 1, 1, 'V'); v.box(2, 12, 15, 1, 1, 6, 'V'); v.box(25, 12, 15, 1, 1, 6, 'V')
    # piso alto: ventanas y cartel
    v.box(8, 14, 15, 5, 4, 1, 'V'); v.box(14, 14, 15, 3, 4, 1, 'V'); v.box(18, 14, 15, 5, 4, 1, 'v')
    v.box(9, 18, 15, 14, 1, 1, 'W') if False else None
    v.box(8, 19, 3, 18, 1, 12, 'b')                           # techo
    v.box(8, 20, 3, 18, 1, 12, 'B'); v.box(9, 20, 4, 16, 1, 10, '.')
    v.box(10, 19, 14, 14, 1, 1, 'W') if False else None
    v.box(14, 20, 12, 8, 2, 1, 'W'); v.box(15, 20, 12, 6, 1, 1, 'k') if False else None
    v.texto(11, 15, 15, 'SHOP', 'W') if False else None
    # tanques azules y cuarto marrón en el techo
    v.box(20, 20, 5, 3, 3, 3, 'N'); v.box(24, 20, 5, 1, 1, 1, 'N') if False else None
    v.box(11, 20, 5, 4, 4, 4, 'O'); v.box(12, 20, 9, 2, 2, 1, 'k')
    v.box(2, 0, 21, 24, 1, 3, 't')
    v.guardar()

if __name__ == '__main__':
    os.makedirs(SAL, exist_ok=True)
    hospital(); ladrillo(); torre_dorada(); farmacia(); hotel(); gemelas(); oficina(); colorido(); comercial()
    print('modelos listos')
