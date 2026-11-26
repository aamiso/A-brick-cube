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
    def rellenar(s, saltar=None):
        # Solo tapa huecos INTERNOS: espacio vacío con algo debajo en la misma columna.
        # (Entradas, marquesinas y soportales se quedan como están.)
        for z in range(s.D):
            for x in range(s.W):
                if saltar and saltar(x, z): continue
                top = -1
                for y in range(1, s.H):
                    if s.g[y][z][x] != '.': top = y
                abajo = None
                for y in range(1, top + 1):
                    c = s.g[y][z][x]
                    if c != '.': abajo = c
                    elif abajo: s.g[y][z][x] = abajo
    def guardar(s):
        s.rellenar(getattr(s, 'saltar', None))
        with open(os.path.join(SAL, s.n + '.txt'), 'w') as f:
            f.write('modelo %s\n// Hecho con tools/hacer_modelos.py\n' % s.n.capitalize())
            for k, v in s.col.items(): f.write('color %s #%s\n' % (k, v))
            for y in range(s.H):
                f.write('capa %d\n' % (y + 1))
                for z in range(s.D): f.write(''.join(s.g[y][z]) + '\n')

def suelo(v, borde=2):
    v.box(0, 0, 0, v.W, 1, v.D, 'G')
    v.box(borde, 0, borde, v.W - 2 * borde, 1, v.D - 2 * borde, 'P')

# ---------------- HOSPITAL GRANDE (44 x 24 celdas): ala izquierda, torre central con cruz, ala con helipuerto ----------------
def hospital():
    v = V('hospital', 88, 48, 38, dict(P='cfc9bb', G='6bbf4a', C='efe6cf', W='ffffff', V='6f9fd8', v='4f7fc0', S='b8b2a4', k='25272c', Y='ffd23a',
        B='2f62c9', R='d9382c', D='6b5a3c', T='8a6a42', L='3f9a3a', g='7f7f86', e='e3e8ee'))
    v.box(0, 0, 0, 88, 1, 48, 'G'); v.box(2, 0, 2, 84, 1, 44, 'P')
    def cuerpo(x0, w, z0, d, pisos, techo=True):
        for f in range(pisos):
            y0 = 1 + f * 7
            v.box(x0, y0, z0, w, 2, d, 'C'); v.box(x0, y0 + 2, z0, w, 4, d, 'V'); v.box(x0, y0 + 6, z0, w, 1, d, 'W')
            for xx in range(x0 + 1, x0 + w, 3):
                for zz in (z0, z0 + d - 1):
                    for yy in range(y0 + 2, y0 + 6): v.put(xx, yy, zz, 'v')
            for zz in range(z0 + 1, z0 + d, 3):
                for xx in (x0, x0 + w - 1):
                    for yy in range(y0 + 2, y0 + 6): v.put(xx, yy, zz, 'v')
        if techo:
            yt = 1 + pisos * 7
            v.box(x0, yt, z0, w, 1, d, 'W')
            v.box(x0 + 1, yt, z0 + 1, w - 2, 1, d - 2, 'e')
            v.box(x0, yt + 1, z0, w, 1, d, 'W'); v.box(x0 + 1, yt + 1, z0 + 1, w - 2, 1, d - 2, '.')   # muretes
    cuerpo(4, 28, 8, 24, 3)                  # ala izquierda
    cuerpo(56, 28, 8, 24, 3)                 # ala derecha
    cuerpo(32, 24, 5, 28, 4)                 # torre central
    # helipuerto en el techo del ala derecha
    for z in range(0, 48):
        for x in range(56, 84):
            if (x - 70) ** 2 + (z - 20) ** 2 <= 81: v.put(x, 22, z, 'k')
            if 64 <= (x - 70) ** 2 + (z - 20) ** 2 <= 81 and (x - 70) ** 2 + (z - 20) ** 2 <= 81: v.put(x, 22, z, 'Y')
    v.box(66, 22, 16, 1, 1, 9, 'W'); v.box(74, 22, 16, 1, 1, 9, 'W'); v.box(66, 22, 20, 9, 1, 1, 'W')
    # cuartos en los techos (azul) y tanques
    v.box(8, 23, 12, 8, 3, 6, 'k'); v.box(8, 26, 12, 8, 2, 6, 'B'); v.box(20, 23, 12, 4, 2, 4, 'g')
    v.box(36, 30, 9, 8, 3, 8, 'k'); v.box(36, 33, 9, 8, 2, 8, 'B'); v.box(50, 30, 9, 3, 2, 3, 'g')
    # entrada: vestíbulo de vidrio de 2 pisos al frente de la torre
    v.box(34, 1, 33, 20, 13, 6, 'W'); v.box(35, 2, 38, 18, 6, 1, 'V'); v.box(35, 2, 33, 18, 8, 1, 'V')
    v.box(33, 14, 32, 22, 1, 9, 'W'); v.box(33, 15, 32, 22, 1, 9, 'e')
    v.box(40, 1, 38, 8, 6, 1, '.')                                         # hueco de la puerta
    v.box(40, 1, 38, 8, 1, 1, 'k'); v.box(40, 6, 38, 8, 1, 1, 'k')
    v.box(35, 9, 38, 18, 4, 1, 'k')
    v.texto(35 + (18 - v.ancho_texto('HOSPITAL')) // 2 + 0, 9, 39, 'HOSPITAL', 'Y') if v.ancho_texto('HOSPITAL') <= 18 else v.texto(34, 9, 39, 'HOSPITAL', 'Y')
    # canopia frontal con columnas
    v.box(36, 8, 39, 16, 1, 4, 'W')
    for x in (37, 50): v.box(x, 1, 42, 2, 7, 2, 'W')
    v.box(38, 1, 39, 12, 1, 6, 'S')
    # cruz roja grande en la torre (frente, arriba)
    v.box(41, 17, 32, 6, 1, 1, 'W')
    v.box(41, 21, 32, 6, 8, 1, 'R'); v.box(38, 24, 32, 12, 2, 1, 'R'); v.box(40, 22, 32, 8, 1, 1, 'R')
    # estacionamiento: líneas blancas y arbolitos
    for x in range(8, 80, 5):
        if not 32 <= x <= 56: v.box(x, 0, 38, 1, 1, 6, 'W')
    for x in (4, 82): v.box(x, 1, 40, 1, 2, 1, 'T'); v.box(x - 1, 3, 39, 3, 2, 3, 'L')
    # ambulancia: bloque blanco con cruz
    v.box(12, 1, 40, 8, 3, 4, 'W'); v.box(12, 4, 40, 4, 1, 4, 'V'); v.box(15, 2, 40, 2, 2, 1, 'R'); v.box(12, 1, 40, 8, 1, 1, 'k')
    v.saltar = lambda x, z: z >= 33
    v.guardar()

# ---------------- COMISARIA (28 x 18 celdas) ----------------
def comisaria():
    # COMISARÍA (28 x 18 celdas): bloque central de 4 pisos con escudo y entrada de vidrio, dos alas de 3 pisos con garaje
    v = V('comisaria', 56, 36, 31, dict(P='cfc9bb', G='6bbf4a', W='f2f4f8', e='d6dde8', B='2f62c9', b='1f3f8a', V='7fb4e6', v='4f86c0',
        k='25272c', Y='ffd23a', R='d9382c', g='7f7f86', S='b8b2a4', L='3f9a3a'))
    v.box(0, 0, 0, 56, 1, 36, 'G'); v.box(2, 0, 2, 52, 1, 32, 'P')
    def ala(x0, w, z0, d, pisos):
        for f in range(pisos):
            y0 = 1 + f * 6
            v.box(x0, y0, z0, w, 6, d, 'W'); v.box(x0, y0 + 5, z0, w, 1, d, 'e')
            for xx in range(x0 + 2, x0 + w - 2, 4):
                for zz in (z0, z0 + d - 1):
                    v.box(xx, y0 + 2, zz, 2, 3, 1, 'V')
            for zz in range(z0 + 3, z0 + d - 2, 5):
                for xx in (x0, x0 + w - 1):
                    v.box(xx, y0 + 2, zz, 1, 3, 2, 'V')
        top = 1 + pisos * 6
        v.box(x0, top, z0, w, 1, d, 'b'); v.box(x0 + 1, top + 1, z0 + 1, w - 2, 1, d - 2, 'g') if False else None
        return top
    t1 = ala(3, 17, 6, 24, 3); t2 = ala(36, 17, 6, 24, 3); t3 = ala(19, 18, 4, 28, 4)
    # franja azul en el último piso de las alas y del centro
    for x0, w, z0, d, top in ((3, 17, 6, 24, t1), (36, 17, 6, 24, t2)):
        v.box(x0, top - 3, z0 + d - 1, w, 2, 1, 'B')
    v.box(19, 19, 31, 18, 5, 1, 'B')
    # escudo dorado sobre la entrada
    for i, fila in enumerate(["YYYYYY", "YYYYYY", "YYYYYY", ".YYYY.", "..YY.."]):
        for k, ch in enumerate(fila):
            if ch == 'Y': v.put(25 + k, 23 - i, 32, 'Y')
    # entrada de vidrio con marquesina y escalones
    v.box(24, 1, 31, 8, 9, 1, 'V'); v.box(24, 1, 31, 1, 9, 1, 'W'); v.box(31, 1, 31, 1, 9, 1, 'W'); v.box(27, 1, 31, 2, 4, 1, 'k')
    v.box(22, 10, 31, 12, 1, 4, 'b'); v.box(22, 1, 34, 1, 9, 1, 'W'); v.box(33, 1, 34, 1, 9, 1, 'W')
    v.box(22, 1, 32, 12, 1, 3, 'S')
    # garaje en el ala izquierda y puerta en la derecha
    v.box(5, 1, 29, 5, 4, 1, 'g'); v.box(11, 1, 29, 5, 4, 1, 'g'); v.box(5, 5, 29, 11, 1, 1, 'Y')
    v.box(45, 1, 29, 3, 5, 1, 'V'); v.box(44, 1, 29, 1, 5, 1, 'W'); v.box(48, 1, 29, 1, 5, 1, 'W')
    # sirenas, antena y bandera en el techo
    v.box(24, t3 + 1, 14, 3, 2, 3, 'R'); v.box(30, t3 + 1, 14, 3, 2, 3, 'B'); v.box(26, t3 + 1, 8, 4, 3, 4, 'g')
    v.box(48, t1 + 1, 10, 1, 9, 1, 'g'); v.box(44, t1 + 6, 10, 4, 3, 1, 'B'); v.box(44, t1 + 8, 10, 4, 1, 1, 'R')
    # arbustos y acera
    for bx in (4, 14, 40, 50): v.box(bx, 1, 32, 3, 2, 2, 'L')
    v.guardar()

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
    v.saltar = lambda x, z: z >= 16
    v.guardar()

# ---------------- TORRE DORADA (28 x 28 celdas, ~78 de alto): el diseño original a doble escala ----------------
def torre_dorada():
    K = 2
    v = V('torre_dorada', 28 * K, 28 * K, 78 * K, dict(P='d9b36a', G='6bbf4a', o='e89a1c', O='f2b01e', Y='ffd92e', y='fff27a', d='b5651d', D='8c4a14', k='3a2410', g='a8761f'))
    def B(x, y, z, w, h, d, c): v.box(x * K, y * K, z * K, w * K, h * K, d * K, c)
    B(0, 0, 0, 28, 1, 28, 'P'); B(0, 0, 0, 28, 1, 2, 'G')
    B(7, 1, 7, 14, 62, 14, 'O')                                           # núcleo
    B(9, 62, 9, 10, 8, 10, 'Y'); B(8, 70, 8, 12, 2, 12, 'o'); B(10, 72, 10, 8, 1, 8, 'y')
    B(12, 73, 12, 4, 5, 4, 'Y')                                           # aguja
    alt = [26, 40, 52, 34]
    for i, x in enumerate((3, 8, 14, 20)):                                # contrafuertes
        B(x, 1, 3, 4, alt[i], 4, 'd'); B(x, 1, 21, 4, alt[(i + 2) % 4], 4, 'd')
    for i, z in enumerate((8, 14)):
        B(3, 1, z, 4, alt[i + 1] - 6, 4, 'd'); B(21, 1, z, 4, alt[i] - 4, 4, 'd')
    for y in range(6, 62, 8): B(7, y, 7, 14, 1, 14, 'g')                  # franjas
    for y in range(10, 60, 8):
        for x in (10, 14, 18):
            B(x, y, 6, 2, 4, 1, 'k'); B(x, y + 3, 6, 2, 1, 1, 'y')       # ventanas encendidas al frente
            B(x, y, 21, 2, 4, 1, 'k')
    B(9, 55, 6, 10, 5, 1, 'Y'); B(11, 56, 5, 6, 3, 1, 'y')               # luz grande
    for (x, y, z) in ((4, 20, 2), (9, 34, 2), (15, 46, 2), (21, 28, 2), (3, 16, 9), (24, 22, 9), (4, 30, 15)):
        B(x, y, z, 1, 1, 1, 'y')
    B(9, 1, 6, 10, 6, 1, 'k'); B(8, 7, 3, 12, 1, 4, 'D'); B(11, 1, 6, 6, 5, 1, 'y'); B(12, 1, 2, 4, 1, 4, 'g')   # entrada
    v.guardar()

# ---------------- CAFÉ (16 x 14 celdas): edificio de dos pisos con toldo, terraza, mesas, bancas y sombrillas ----------------
def cafe():
    v = V('cafe', 32, 28, 30, dict(G='6bbf4a', P='d8d2c4', p='eae4d4', C='f4e9d0', B='a8553a', b='8a3f2a', F='5a3a22', D='7a4a26', V='9fd6ee', A='2e7d4f', w='ffffff',
        S='3a2410', Y='f2c230', T='b9824a', H='8a5a2a', U='d9382c', L='3a8f3a', o='c86a2a', y='ffe680', k='25272c', K='7a4a26', r='c9b99a'))
    v.box(0, 0, 0, 32, 1, 28, 'G')
    for z in range(2, 27):
        for x in range(2, 30): v.put(x, 0, z, 'P' if ((x // 2 + z // 2) % 2 == 0) else 'p')
    # ---- edificio: x 2..29, z 2..17 ----
    X0, W0, Z0, D0 = 2, 28, 2, 16
    v.box(X0, 1, Z0, W0, 8, D0, 'C')                                   # planta baja
    v.box(X0, 1, Z0 + D0 - 1, W0, 2, 1, 'B')                           # zócalo de ladrillo al frente
    v.box(X0, 1, Z0, 1, 2, D0, 'B'); v.box(X0 + W0 - 1, 1, Z0, 1, 2, D0, 'B')
    zf = Z0 + D0 - 1                                                   # cara del frente (z = 17)
    for x in range(X0 + 1, X0 + W0 - 1):                               # ventanales corridos
        v.box(x, 3, zf, 1, 4, 1, 'V')
    for x in (X0, X0 + 7, X0 + 13, X0 + 18, X0 + 21, X0 + W0 - 1):     # pilares
        v.box(x, 1, zf, 1, 7, 1, 'F')
    v.box(X0, 7, zf, W0, 1, 1, 'F')                                    # dintel
    v.box(14, 1, zf, 4, 6, 1, 'D'); v.box(15, 3, zf, 2, 3, 1, 'V')     # puerta con vidrio
    # toldo a rayas verde y blanco
    for i, x in enumerate(range(X0, X0 + W0, 2)):
        c = 'A' if i % 2 == 0 else 'w'
        v.box(x, 8, zf + 1, 2, 1, 2, c); v.box(x, 7, zf + 3, 2, 1, 2, c); v.box(x, 6, zf + 5, 2, 1, 1, c)
    # ---- piso de arriba ----
    v.box(X0, 9, Z0, W0, 6, D0, 'C')
    for cx in (6, 14, 22):
        v.box(cx, 10, zf, 4, 4, 1, 'V')
        v.box(cx - 1, 10, zf, 1, 4, 1, 'A'); v.box(cx + 4, 10, zf, 1, 4, 1, 'A')       # postigos verdes
        v.box(cx - 1, 9, zf + 1, 6, 1, 1, 'F')                                           # alféizar
        v.box(cx, 14, zf, 4, 1, 1, 'F')
    for z in range(Z0 + 2, zf - 1, 4):                                                   # ventanas laterales
        for xx in (X0, X0 + W0 - 1): v.box(xx, 10, z, 1, 4, 2, 'V')
    v.box(12, 9, zf + 1, 8, 1, 2, 'F')                                                   # balcón
    v.box(12, 10, zf + 2, 8, 1, 1, 'F')
    for x in (12, 15, 19): v.box(x, 10, zf + 2, 1, 2, 1, 'F')
    v.box(13, 10, zf + 1, 2, 2, 1, 'L'); v.box(17, 10, zf + 1, 2, 2, 1, 'L')             # macetas del balcón
    # cornisa, techo y letrero
    v.box(X0 - 1, 15, Z0 - 1, W0 + 2, 1, D0 + 2, 'F'); v.box(X0, 16, Z0, W0, 1, D0, 'b')
    v.box(X0, 17, zf, W0, 1, 1, 'C'); v.box(X0, 17, Z0, W0, 1, 1, 'C'); v.box(X0, 17, Z0, 1, 1, D0, 'C'); v.box(X0 + W0 - 1, 17, Z0, 1, 1, D0, 'C')
    v.box(7, 17, zf, 18, 7, 1, 'S'); v.box(7, 24, zf, 18, 1, 1, 'F'); v.box(7, 17, zf, 18, 1, 1, 'F')       # letrero en el techo
    v.box(6, 17, zf, 1, 8, 1, 'F'); v.box(25, 17, zf, 1, 8, 1, 'F')
    v.texto(8, 19, zf + 1, 'CAFE', 'Y')
    v.box(26, 17, 4, 3, 5, 3, 'B'); v.box(25, 22, 3, 5, 1, 5, 'b')                       # chimenea
    # taza gigante en el techo
    v.box(4, 17, 5, 5, 3, 5, 'w'); v.box(5, 20, 6, 3, 1, 3, 'T'); v.box(9, 18, 6, 1, 2, 3, 'w'); v.box(3, 17, 4, 7, 1, 7, 'r')
    for sx in (5, 7): v.box(sx, 21, 7, 1, 2, 1, 'w')
    # ---- terraza: barandilla, bancas, mesas y sombrillas (z 19..26) ----
    for x in range(2, 30):                                                               # barandilla del frente (con entrada)
        if 13 <= x <= 18: continue
        v.box(x, 1, 26, 1, 1, 1, 'F') if x % 4 == 0 else v.box(x, 2, 26, 1, 1, 1, 'F')
    for x in range(2, 30, 4):
        if not 13 <= x <= 18: v.box(x, 1, 26, 1, 3, 1, 'F')
    for z in range(19, 26):
        for xx in (2, 29): v.box(xx, 2, z, 1, 1, 1, 'F')
    for z in range(20, 26, 4):
        for xx in (2, 29): v.box(xx, 1, z, 1, 3, 1, 'F')
    def banca(bx, bz, atras):                                  # 4 de ancho: asiento, patas y respaldo (atras = +1 / -1 hacia donde queda el respaldo)
        v.box(bx, 2, bz, 4, 1, 1, 'H'); v.put(bx, 1, bz, 'H'); v.put(bx + 3, 1, bz, 'H')
        v.box(bx, 3, bz + atras, 4, 2, 1, 'H')
    def mesa(tx, tz, sombrilla):
        v.box(tx + 1, 1, tz + 1, 1, 2, 1, 'T'); v.box(tx, 3, tz, 3, 1, 3, 'T')                 # mesa de 3x3 con pata central
        v.put(tx + 1, 4, tz + 1, 'y')                                                           # farolito
        banca(tx - 0, tz - 2, -1); banca(tx - 0, tz + 4, 1)                                     # bancas a los lados (respaldo hacia afuera)
        if sombrilla:
            v.box(tx + 1, 4, tz + 1, 1, 6, 1, 'F')
            for i in range(7):
                for j in range(7):
                    cx, cz = i - 3, j - 3
                    d = abs(cx) + abs(cz)
                    if d <= 4: v.put(tx + 1 + cx, 10 if d > 1 else 11, tz + 1 + cz, 'U' if (i + j) % 2 == 0 else 'w')
    mesa(4, 21, True); mesa(25, 21, True); mesa(11, 22, False); mesa(18, 22, False)
    # bancas pegadas a la pared y macetas
    for mx in (9, 22):
        v.box(mx, 1, 18, 2, 2, 1, 'o'); v.box(mx, 3, 18, 2, 3, 1, 'L')
    # faroles en las esquinas de la terraza
    for (lx, lz) in ((3, 25), (28, 25)):
        v.box(lx, 1, lz, 1, 6, 1, 'k'); v.box(lx - 1, 7, lz - 1, 3, 1, 3, 'y')
    v.box(14, 0, 26, 4, 1, 3, 'p'); v.box(15, 0, 27, 2, 1, 1, 'P')                         # camino de entrada
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


# ---------------- TORRES GEMELAS grandes (28 x 22 celdas) ----------------
def gemelas():
    v = V('torres_gemelas', 56, 44, 52, dict(P='d8d4c8', G='3fae3a', w='e9e9f2', W='ffffff', g='b9b9c6', V='2f58c8', v='4a78e0', k='25272c', m='8c8c94', Y='f4e04a', h='9aa0aa', d='c9c9d6'))
    v.box(0, 0, 0, 56, 1, 44, 'G'); v.box(2, 0, 3, 52, 1, 38, 'P')
    PISOS = 6
    top = 1 + PISOS * 7
    def ala(x0, interior):
        v.box(x0, 1, 4, 20, top - 1, 32, 'w')                      # cuerpo
        for f in range(PISOS):
            y0 = 3 + f * 7
            for xx in (x0 + 2, x0 + 11):                           # ventanas del frente
                v.box(xx, y0, 35, 7, 4, 1, 'V'); v.box(xx + 3, y0, 35, 1, 4, 1, 'v')
            for zz in (7, 15, 23):                                 # ventanas de los costados
                v.box(x0 - 0, y0, zz, 1, 4, 5, 'V'); v.box(x0 + 19, y0, zz, 1, 4, 5, 'V')
            v.box(x0, y0 + 5, 4, 20, 1, 32, 'W')                   # franja blanca entre pisos
            v.box(x0, y0 - 2, 36, 20, 1, 1, 'W') if f else None
        # costillas (columnas estriadas) en la esquina del frente junto al puente
        for k in range(6):
            xx = x0 + (14 + k if interior else k)
            v.box(xx, 1, 36 if k % 2 == 0 else 35, 1, top - 1, 1, 'W')
        v.box(x0 + 7, 1, 35, 6, 4, 2, 'k') if False else None
        v.box(x0 + 8, 1, 36, 5, 4, 1, 'k')                         # puerta
        v.box(x0, top, 4, 20, 1, 32, 'W'); v.box(x0 + 1, top, 5, 18, 1, 30, 'k')   # techo oscuro
        v.box(x0 + 4, top, 18, 12, 1, 12, 'Y'); v.box(x0 + 5, top, 19, 10, 1, 10, 'k'); v.box(x0 + 7, top, 22, 6, 1, 1, 'Y')   # helipuerto
        v.box(x0 + 3, top + 1, 6, 10, 5, 8, 'm'); v.box(x0 + 5, top + 1, 14, 5, 4, 1, 'k') # cuarto de techo con puerta
    ala(2, 1); ala(34, 0)
    # puente central macizo con pisos marcados
    v.box(22, 1, 10, 12, top - 1, 20, 'd')
    for f in range(PISOS):
        y0 = 3 + f * 7
        v.box(22, y0, 30, 12, 4, 1, 'g'); v.box(22, y0 + 4, 30, 12, 1, 1, 'W'); v.box(22, y0 + 5, 30, 12, 2, 1, 'h')
        for xx in range(23, 33, 3): v.box(xx, y0, 30, 1, 4, 1, 'm')
    v.box(22, top, 10, 12, 1, 20, 'W')
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
    hospital(); cafe(); comisaria(); ladrillo(); torre_dorada(); farmacia(); hotel(); gemelas(); oficina(); colorido(); comercial()
    print('modelos listos')
