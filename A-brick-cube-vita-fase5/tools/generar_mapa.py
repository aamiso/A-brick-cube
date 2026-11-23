#!/usr/bin/env python3
# Genera un mapa de 300 x 300 para A Brick Cube Vita.
# Uso:  python3 generar_mapa.py [semilla] > ciudad.txt
import random, sys
SEED = int(sys.argv[1]) if len(sys.argv) > 1 else 11
random.seed(SEED)
N = 300
m = [['.'] * N for _ in range(N)]

def rect(x0, z0, w, d, ch):
    for z in range(z0, z0 + d):
        for x in range(x0, x0 + w):
            if 0 <= x < N and 0 <= z < N: m[z][x] = ch

# ---- 1) cuadrícula de calles ----
def cortes():
    pos, v = [], 0
    pos.append(v); v += 3   # borde de pasto
    out = []
    x = 3
    while True:
        bloque = random.choice([30, 36, 42])
        if x + bloque + 6 > N - 3: break
        out.append((x, bloque)); x += bloque + 6
    return out, x
# calles verticales y horizontales: [inicio_calle]
def lineas():
    xs, x = [], 3
    while x + 6 + 30 <= N - 3:
        xs.append(x)
        x += 6 + random.choice([30, 36, 42])
    xs.append(x)  # última calle
    return xs
xs, zs = lineas(), lineas()
# ajustar para que la última calle quepa
xs = [x for x in xs if x + 6 <= N - 3]; zs = [z for z in zs if z + 6 <= N - 3]
for z in zs: rect(0, z, N, 6, 'S'); rect(0, z + 1, N, 4, '=') ; 
for x in xs: rect(x, 0, 6, N, 'S'); rect(x + 1, 0, 4, N, '=')
# las aceras de los cruces quedan como calle
for z in zs:
    for x in xs:
        rect(x, z, 6, 6, 'S'); rect(x + 1, z + 1, 4, 4, '=')
        rect(x, z + 1, 6, 4, '='); rect(x + 1, z, 4, 6, '=')
# las calles llegan hasta el borde: deja un borde de 3 de pasto
rect(0, 0, N, 3, '.'); rect(0, N - 3, N, 3, '.'); rect(0, 0, 3, N, '.'); rect(N - 3, 0, 3, N, '.')

# ---- 2) manzanas ----
def manzanas():
    bx = [(xs[i] + 6, xs[i + 1] - 1) for i in range(len(xs) - 1)]
    bz = [(zs[i] + 6, zs[i + 1] - 1) for i in range(len(zs) - 1)]
    return [(a0, b0, a1 - a0 + 1, b1 - b0 + 1) for (a0, a1) in bx for (b0, b1) in bz]
bl = manzanas()
cx0 = cz0 = N / 2

def libre(x0, z0, w, d):
    return all(m[z][x] == '.' for z in range(z0 - 1, z0 + d + 1) for x in range(x0 - 1, x0 + w + 1)
               if 0 <= x < N and 0 <= z < N)

def arbol_en(x, z):
    if m[z][x] == '.': m[z][x] = 'T'

def lote(x0, z0, w, d, letras, dmin, dmax):
    # coloca un edificio centrado en el lote con margen
    ew = random.randint(dmin, min(dmax, w - 2)); ed = random.randint(dmin, min(dmax, d - 2))
    ex = x0 + random.randint(1, max(1, w - ew - 1)); ez = z0 + random.randint(1, max(1, d - ed - 1))
    rect(ex, ez, ew, ed, random.choice(letras))

def distrito(x0, z0, w, d):
    cx, cz = x0 + w / 2, z0 + d / 2
    dist = max(abs(cx - cx0), abs(cz - cz0))
    return dist

def subdividir(x0, z0, w, d, minimo):
    # parte la manzana en lotes
    if w >= 2 * minimo + 2 and (w >= d or d < 2 * minimo + 2):
        c = random.randint(minimo + 1, w - minimo - 1)
        return subdividir(x0, z0, c, d, minimo) + subdividir(x0 + c, z0, w - c, d, minimo)
    if d >= 2 * minimo + 2:
        c = random.randint(minimo + 1, d - minimo - 1)
        return subdividir(x0, z0, w, c, minimo) + subdividir(x0, z0 + c, w, d - c, minimo)
    return [(x0, z0, w, d)]

TAM = {'H': (10, 9), 'J': (11, 11), 'K': (13, 11), 'X': (15, 13), 'Q': (16, 14)}
cuenta = {'X': 0, 'Q': 0}
LIMITE = {'X': 6, 'Q': 12}
def lugar_modelo(a, b, c, e, letras, prob):
    """Pone un lugar con modelo (rectángulo exacto) dentro del lote si cabe."""
    if random.random() > prob: return False
    opciones = [l for l in letras if TAM[l][0] + 2 <= c and TAM[l][1] + 2 <= e and cuenta.get(l, 0) < LIMITE.get(l, 999)]
    if not opciones: return False
    if any(o in cuenta for o in opciones):
        opciones.sort(key=lambda o: (cuenta.get(o, 0) / LIMITE.get(o, 1), random.random()))
        l = opciones[0]
    else:
        l = random.choice(opciones)
    fw, fd = TAM[l]
    x = a + random.randint(1, c - fw - 1); z = b + random.randint(1, e - fd - 1)
    rect(x, z, fw, fd, l)
    if l in cuenta: cuenta[l] += 1
    return True

parques, centro = [], None
idx = list(range(len(bl)))
# manzana del centro = plaza donde empieza el jugador
bl_orden = sorted(idx, key=lambda i: distrito(*bl[i]))
plaza = bl_orden[0]
lagos = random.sample(bl_orden[8:], 2)
parque_g = random.sample([i for i in bl_orden[4:] if i not in lagos], 3)
industria = [i for i in idx if bl[i][0] > N * 0.62 and bl[i][1] > N * 0.62][:4]
for i in idx:
    x0, z0, w, d = bl[i]
    dist = distrito(x0, z0, w, d)
    if i == plaza:
        rect(x0 + 2, z0 + 2, w - 4, d - 4, '.')
        rect(x0 + w // 2 - 3, z0 + d // 2 - 3, 7, 7, 'S')
        for _ in range(26): arbol_en(random.randint(x0, x0 + w - 1), random.randint(z0, z0 + d - 1))
        m[z0 + d // 2][x0 + w // 2] = 'P'
        continue
    if i in lagos:
        rect(x0 + 5, z0 + 5, w - 10, d - 10, 'W')
        for _ in range(w * d // 25): arbol_en(random.randint(x0, x0 + w - 1), random.randint(z0, z0 + d - 1))
        continue
    if i in parque_g:
        for _ in range(w * d // 12): arbol_en(random.randint(x0, x0 + w - 1), random.randint(z0, z0 + d - 1))
        rect(x0 + 3, z0 + d // 2, w - 6, 1, 'S'); rect(x0 + w // 2, z0 + 3, 1, d - 6, 'S')
        continue
    if i in industria:
        for (a, b, c, e) in subdividir(x0, z0, w, d, 14):
            rect(a + 1, b + 1, c - 2, e - 2, random.choice('GBG'))
        continue
    if dist < 55:      # centro: rascacielos
        for (a, b, c, e) in subdividir(x0, z0, w, d, 12):
            if not lugar_modelo(a, b, c, e, 'XQ', 0.40): lote(a, b, c, e, 'EEDDFNA', 10, 18)
    elif dist < 105:   # anillo medio: oficinas, departamentos, tiendas, bancos y cafés
        for (a, b, c, e) in subdividir(x0, z0, w, d, 9):
            if not lugar_modelo(a, b, c, e, 'XQQ', 0.60): lote(a, b, c, e, 'ABFLLCNB', 8, 13)
    else:              # afueras: casas con modelo y jardín
        for (a, b, c, e) in subdividir(x0, z0, w, d, 12):
            if random.random() < 0.10:
                for _ in range(c * e // 8): arbol_en(random.randint(a, a + c - 1), random.randint(b, b + e - 1))
                continue
            if not lugar_modelo(a, b, c, e, 'HJK', 1.0):
                for _ in range(c * e // 10): arbol_en(random.randint(a, a + c - 1), random.randint(b, b + e - 1))
                continue
            for _ in range(random.randint(2, 6)):
                arbol_en(a + random.randint(0, c - 1), b + random.randint(0, e - 1))
# árboles en aceras largas: pasto en las esquinas de afuera
for z in range(N):
    for x in range(N):
        if m[z][x] == '.' and random.random() < 0.004: m[z][x] = 'T'
# P: asegurar una sola
ps = [(x, z) for z in range(N) for x in range(N) if m[z][x] == 'P']
if not ps:
    for z in range(N // 2, N):
        if m[z][N // 2] in '.=S': m[z][N // 2] = 'P'; break
# la P se pone sobre acera/calle, nunca encima de un árbol
print("# A Brick Cube - ciudad %dx%d (semilla %d)" % (N, N, SEED))
for row in m: print(''.join(row))
