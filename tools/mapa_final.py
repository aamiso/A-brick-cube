#!/usr/bin/env python3
# MAPA FINAL hecho a mano (232 x 232). Uso: python3 mapa_final.py > ../data/mapas/ciudad.txt
# Zonas: residencial (NO), centro/rascacielos (N), comercial (NE), hospital (O), plaza con la torre dorada (centro),
# comisaría (E), zona natural con río (SO), parque con lago (S) y hoteles/centro comercial (SE).
import random, math
random.seed(7)
N = 232
m = [['.'] * N for _ in range(N)]
TAM = {'H': (10, 9), 'J': (11, 11), 'K': (13, 11), 'X': (15, 13), 'Q': (16, 14), 'Y': (44, 24), 'R': (9, 9), 'V': (28, 28),
       'M': (12, 10), 'U': (16, 14), 'O': (28, 22), 'I': (16, 14), 'Z': (12, 12), 's': (14, 12), 'c': (28, 18)}

def rect(x0, z0, w, d, ch):
    for z in range(z0, z0 + d):
        for x in range(x0, x0 + w):
            if 0 <= x < N and 0 <= z < N: m[z][x] = ch
def lugar(ch, x, z):
    w, d = TAM[ch]; rect(x, z, w, d, ch)
def arbol(x, z):
    if 0 <= x < N and 0 <= z < N and m[z][x] == '.': m[z][x] = 'T'
def arboles(x0, z0, w, d, dens):
    for z in range(z0, z0 + d):
        for x in range(x0, x0 + w):
            if random.random() < dens: arbol(x, z)

# ---- calles estrechas: 1 acera + 6 asfalto + 1 acera ----
LIN = [4, 76, 148, 220]
for p in LIN:
    rect(p, 4, 8, N - 8, 'S'); rect(p + 1, 4, 6, N - 8, '=')
    rect(4, p, N - 8, 8, 'S'); rect(4, p + 1, N - 8, 6, '=')
for a in LIN:
    for b in LIN:
        rect(a, b, 8, 8, 'S'); rect(a + 1, b, 6, 8, '='); rect(a, b + 1, 8, 6, '=')
C1, C2, C3 = 12, 84, 156      # bloques de 64 x 64

# ---- NO: zona residencial ----
for i, x in enumerate((13, 28, 43, 58)):
    for j, z in enumerate((14, 29, 44, 59)):
        lugar('HJK'[(i + j) % 3], x + random.randint(0, 1), z + random.randint(0, 1))
arboles(C1, 12, 64, 64, 0.05)

# ---- N: centro con rascacielos ----
lugar('O', 85, 14); lugar('I', 114, 14); lugar('U', 131, 14)
lugar('R', 86, 40); lugar('Z', 97, 40); lugar('s', 111, 40); lugar('Q', 127, 40)
lugar('R', 86, 58); lugar('Z', 97, 58); lugar('X', 127, 58)
arboles(C2, 12, 64, 64, 0.03)

# ---- NE: zona comercial ----
lugar('X', 158, 14); lugar('Q', 175, 14); lugar('Z', 193, 14)
lugar('I', 158, 38); lugar('s', 176, 38); lugar('R', 192, 38)
lugar('M', 158, 58); lugar('Z', 172, 58); lugar('Q', 186, 58); lugar('M', 204, 58)
arboles(C3, 12, 64, 64, 0.03)

# ---- O: hospital grande con estacionamiento ----
lugar('Y', 14, 88); lugar('M', 61, 90)
rect(14, 118, 58, 20, '='); rect(14, 116, 58, 2, 'S')
arboles(C1, 84, 64, 64, 0.025)

# ---- centro: parque con la torre dorada (28 x 28) en una plaza circular ----
CX, CZ, RR = 116, 110, 25
for z in range(CZ - RR - 1, CZ + RR + 2):
    for x in range(CX - RR - 1, CX + RR + 2):
        if (x - CX + 0.5) ** 2 + (z - CZ + 0.5) ** 2 <= RR * RR and m[z][x] == '.': m[z][x] = 'S'     # el borde queda escalonado (cortes de acera)
rect(113, CZ + RR - 2, 6, 147 - (CZ + RR - 2) + 1, 'S')                                          # avenida sur
for (x0, x1) in ((90, 109), (122, 141)):                                                          # prados con borde de acera
    rect(x0, 140, x1 - x0 + 1, 7, 'S'); rect(x0 + 1, 141, x1 - x0 - 1, 5, '.')
    for xx in range(x0 + 3, x1 - 1, 5): arbol(xx, 143)
lugar('V', 102, 96)
m[141][115] = 'P'                                     # aquí empieza el jugador
for (x0, z0) in ((86, 86), (133, 86), (86, 127), (133, 127)):                                    # esquinas de pasto con árboles
    for _ in range(5): arbol(x0 + random.randint(0, 12), z0 + random.randint(0, 12))
for i in range(86, 146, 3):                           # hilera de árboles en el borde del parque
    arbol(87, i); arbol(144, i); arbol(i, 87)
# ---- E: comisaría ----
lugar('c', 174, 88); lugar('Z', 158, 90); lugar('Z', 206, 90)
rect(160, 118, 56, 20, '='); rect(160, 116, 56, 2, 'S')
arboles(C3, 84, 64, 64, 0.025)

# ---- SO: zona natural con río ----
arboles(C1, 156, 64, 64, 0.30)
for z in range(156, 220):
    cx = 16 + int((z - 156) * 0.55) + int(3 * math.sin((z - 156) / 6.0))
    for x in range(cx, cx + 5): m[z][x] = 'W'
for z in range(156, 220):
    for x in range(C1, C1 + 64):
        if m[z][x] == 'T' and any(m[zz][xx] == 'W' for zz in range(z - 1, z + 2) for xx in range(x - 2, x + 3) if 0 <= xx < N): m[z][x] = '.'

# ---- S: parque con lago ----
for z in range(156, 220):
    for x in range(84, 148):
        if ((x - 103) / 16.0) ** 2 + ((z - 190) / 9.0) ** 2 < 1.0: m[z][x] = 'W'
arboles(C2, 156, 64, 64, 0.10)
rect(86, 178, 60, 2, 'S'); rect(115, 158, 2, 58, 'S')
for z in range(156, 220):
    for x in range(84, 148):
        if m[z][x] == 'T' and (abs(x - 116) < 2 or abs(z - 179) < 2): m[z][x] = '.'

# ---- SE: hotel, centro comercial y más ----
lugar('s', 160, 160); lugar('U', 180, 160); lugar('Z', 202, 160)
lugar('I', 160, 186); lugar('Q', 180, 186); lugar('X', 200, 186)
arboles(C3, 156, 64, 64, 0.04)

# ---- bordes: árboles en el pasto de afuera ----
for z in range(N):
    for x in range(N):
        if m[z][x] == '.' and (x < 5 or z < 5 or x > N - 6 or z > N - 6) and random.random() < 0.25: m[z][x] = 'T'
# ---- bosque alrededor: la ciudad es un gran cuadrado y el límite del mapa es un bosque cerrado ----
M = 10
N2 = N + 2 * M
big = [['.'] * N2 for _ in range(N2)]
for z in range(N2):
    for x in range(N2):
        d = min(x, z, N2 - 1 - x, N2 - 1 - z)           # distancia al borde del mapa
        if d < 3 or (d < M and random.random() < 0.5): big[z][x] = 'T'     # pared de árboles al final y bosque más suelto hacia la ciudad
for z in range(N):
    for x in range(N):
        ch = m[z][x]
        if ch == '.' and (x < 4 or z < 4 or x > N - 5 or z > N - 5): ch = 'T' if random.random() < 0.5 else '.'
        big[z + M][x + M] = ch
# ---- menos árboles: pared de 2 al borde, bosque suelto hacia la ciudad y pocos árboles dentro ----
rr = random.Random(99)
for z in range(N2):
    for x in range(N2):
        if big[z][x] != 'T': continue
        d = min(x, z, N2 - 1 - x, N2 - 1 - z)
        if d < 2: continue
        keep = 0.30 if d < M else 0.30
        if rr.random() > keep: big[z][x] = '.'
print("# A Brick Cube - MAPA FINAL %dx%d (ciudad cuadrada + bosque)" % (N2, N2))
for row in big: print(''.join(row))
