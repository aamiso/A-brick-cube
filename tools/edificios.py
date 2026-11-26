import os, sys
from lib import M
OUT = sys.argv[1] if len(sys.argv) > 1 else 'out'

def casa_roja():
    m = M('Casa_roja', 20, 18, 14); m.plate('G')
    m.box(0, 0, 14, 20, 1, 4, 'S'); m.box(8, 0, 12, 4, 1, 2, 'S')           # acera y camino
    x0, z0, x1, z1 = 3, 2, 17, 13
    m.box(x0, 1, z0, x1-x0, 5, z1-z0, 'R')                                  # paredes de ladrillo
    m.box(x0, 1, z0, x1-x0, 1, z1-z0, 'r')                                  # zócalo
    for yy in (1,): pass
    m.vent('f', x0, z0, x1, z1, 1, 6, ww=3, gap=3, wh=3, fh=6, off=2, mg=2, sill='W', frame='W')
    m.vent('l', x0, z0, x1, z1, 1, 6, ww=3, gap=3, wh=3, fh=6, off=2, mg=3, frame='W')
    m.vent('r', x0, z0, x1, z1, 1, 6, ww=3, gap=3, wh=3, fh=6, off=2, mg=3, frame='W')
    m.puerta(10, z1, 3, 4, 'N', 'W')
    m.box(8, 5, z1, 5, 1, 2, 'W'); m.box(8, 1, z1, 1, 4, 1, 'W'); m.box(12, 1, z1, 1, 4, 1, 'W')   # porche
    m.box(7, 6, z1, 7, 1, 2, 'D')
    k = m.techo_dos_aguas(x0, z0, x1, z1, 6, 'D', 'x', 1)
    m.box(13, 6, 4, 2, 5, 2, 'r'); m.box(13, 11, 4, 2, 1, 2, 'D')           # chimenea
    # triángulo en el frente
    for j in range(1, 4): m.box(x0+j+3, 6+j-1, z1-1, 14-2*(j+3), 1, 1, 'R') if False else None
    return m

def casa_azul():
    m = M('Casa_azul', 22, 22, 18); m.plate('G')
    m.box(0, 0, 18, 22, 1, 4, 'S'); m.box(9, 0, 15, 4, 1, 3, 'S')
    x0, z0, x1, z1 = 3, 2, 19, 15
    m.box(x0, 1, z0, x1-x0, 10, z1-z0, 'B'); m.box(x0, 1, z0, x1-x0, 1, z1-z0, 'b')
    m.box(x0, 6, z0, x1-x0, 1, z1-z0, 'W')                                   # cinta entre pisos
    m.vent4(x0, z0, x1, z1, 1, 6, ww=3, gap=3, wh=3, fh=5, off=2, mg=2, frame='W')
    m.vent4(x0, z0, x1, z1, 6, 11, ww=3, gap=3, wh=3, fh=5, off=2, mg=2, frame='W')
    m.puerta(11, z1, 4, 5, 'N', 'W')
    m.box(7, 1, z1, 1, 5, 2, 'W'); m.box(14, 1, z1, 1, 5, 2, 'W'); m.box(6, 6, z1, 10, 1, 3, 'W')   # porche con columnas
    m.box(6, 7, z1, 10, 1, 1, 'S')                                           # balcón
    m.techo_piramide(x0, z0, x1, z1, 11, 'b', 1)
    m.box(14, 11, 4, 2, 5, 2, 'W')                                           # chimenea
    return m

def casa_moderna():
    m = M('Casa_moderna', 26, 22, 12); m.plate('G')
    m.box(0, 0, 18, 26, 1, 4, 'S'); m.box(2, 0, 14, 8, 1, 4, 'S')
    m.box(3, 1, 3, 12, 4, 12, 'W'); m.box(3, 1, 3, 12, 1, 12, 's')            # volumen blanco bajo
    m.box(11, 1, 6, 12, 7, 9, 'W'); m.box(11, 1, 6, 12, 1, 9, 's')           # volumen alto
    m.box(2, 5, 2, 14, 1, 14, 'D'); m.box(10, 8, 5, 14, 1, 11, 'D')           # losas de techo
    m.box(4, 2, 14, 7, 3, 1, 'V'); m.box(4, 2, 14, 7, 1, 1, 'v')              # ventanal
    m.box(12, 2, 14, 10, 6, 1, 'V'); m.box(12, 7, 14, 10, 1, 1, 'v')
    for x in (12, 16, 20, 23): m.box(x, 2, 14, 1, 6, 1, 'K')
    m.box(5, 1, 14, 6, 4, 1, 'n')                                            # garaje de madera
    m.box(13, 1, 14, 3, 1, 1, 'K'); m.puerta(18, 15, 3, 4, 'N', 'K') if False else None
    m.box(17, 1, 14, 3, 5, 1, 'N')
    m.vent('l', 3, 3, 15, 15, 1, 5, ww=3, gap=3, wh=2, fh=6, off=2, mg=2)
    m.vent('r', 11, 6, 23, 15, 1, 9, ww=3, gap=3, wh=3, fh=6, off=3, mg=2)
    m.box(22, 0, 17, 4, 1, 4, 'V')                                           # alberca
    m.box(1, 1, 18, 1, 3, 1, 'g'); m.box(24, 1, 20, 1, 3, 1, 'g')
    return m

def banco():
    m = M('Banco', 30, 26, 22); m.plate('S')
    x0, z0, x1, z1 = 3, 3, 27, 21
    m.box(1, 1, 1, 28, 1, 24, 'w'); m.box(2, 2, 2, 26, 1, 22, 'S')           # escalones del basamento
    m.box(x0, 3, z0, x1-x0, 13, z1-z0, 'w')
    m.vent('l', x0, z0, x1, z1, 3, 16, ww=2, gap=4, wh=6, fh=13, off=3, mg=3, sill='S')
    m.vent('r', x0, z0, x1, z1, 3, 16, ww=2, gap=4, wh=6, fh=13, off=3, mg=3, sill='S')
    m.vent('b', x0, z0, x1, z1, 3, 16, ww=2, gap=4, wh=6, fh=13, off=3, mg=3, sill='S')
    for x in range(5, 26, 4): m.box(x, 3, z1, 2, 12, 2, 'W'); m.box(x-1, 3, z1, 4, 1, 2, 'S'); m.box(x-1, 14, z1, 4, 1, 2, 'S')   # columnas
    m.box(4, 15, z1, 22, 2, 2, 'S')                                           # arquitrabe
    for j in range(4): m.box(5+j*2, 17+j, z1, 20-j*4, 1, 2, 'W')              # frontón triangular
    m.box(11, 4, z1-1, 8, 5, 1, 'D'); m.box(12, 5, z1-1, 6, 4, 1, 'L'); m.box(14, 3, z1-1, 2, 1, 1, 'K')   # puerta dorada
    m.box(10, 12, z1-1, 10, 2, 1, 'l'); m.box(11, 12, z1-1, 8, 2, 1, 'L')    # letrero dorado
    m.box(x0, 16, z0, x1-x0, 1, z1-z0, 'S'); m.box(x0+2, 17, z0+2, x1-x0-4, 1, z1-z0-4, 'D')
    m.aire(9, 18, 8); m.aire(18, 18, 12)
    return m

def cafe():
    m = M('Cafe', 32, 28, 30); m.plate('S')
    m.box(0, 0, 0, 32, 1, 20, 'G') if False else None
    x0, z0, x1, z1 = 3, 3, 29, 19
    m.box(x0, 1, z0, x1-x0, 11, z1-z0, 'n'); m.box(x0, 1, z0, x1-x0, 2, z1-z0, 'N')
    m.box(5, 4, z1-1, 22, 6, 1, 'V'); m.box(5, 9, z1-1, 22, 1, 1, 'v')       # vitrina grande
    for x in (5, 10, 21, 26): m.box(x, 4, z1-1, 1, 6, 1, 'N')
    m.puerta(16, z1, 4, 6, 'N', 'W', y=3)
    m.vent('l', x0, z0, x1, z1, 1, 12, ww=3, gap=3, wh=4, fh=11, off=4, mg=3, sill='N', frame='N')
    m.vent('r', x0, z0, x1, z1, 1, 12, ww=3, gap=3, wh=4, fh=11, off=4, mg=3, sill='N', frame='N')
    for x in range(3, 29, 2):                                                # toldo a rayas
        c = 'R' if (x//2) % 2 == 0 else 'W'
        for k in range(4): m.p(x, 10-k//2, z1+k, c); m.p(x+1, 10-k//2, z1+k, c)
    m.techo_dos_aguas(x0, z0, x1, z1, 12, 'r', 'x', 1)
    m.box(22, 12, 5, 2, 4, 2, 'D')
    m.box(12, 12, z1-2, 8, 4, 1, 'Y'); m.box(13, 13, z1-2, 6, 2, 1, 'N')     # letrero del café
    # terraza con mesas, sillas y sombrillas
    m.box(2, 1, 22, 28, 1, 4, 'w') if False else None
    for tx in (5, 13, 22):
        m.box(tx, 1, 23, 1, 2, 1, 'K'); m.box(tx-1, 3, 22, 3, 1, 3, 'W')
        m.box(tx-2, 1, 23, 1, 1, 1, 'N'); m.box(tx+2, 1, 23, 1, 1, 1, 'N')
        m.box(tx, 4, 23, 1, 4, 1, 'K'); m.box(tx-2, 8, 22, 5, 1, 3, 'R' if tx != 13 else 'Y'); m.box(tx-1, 9, 23, 3, 1, 1, 'R' if tx != 13 else 'Y')
    for x in range(2, 30): m.p(x, 1, 26, 'N') if x % 3 else None             # barandilla baja
    for x in (2, 29): m.box(x, 1, 21, 1, 2, 6, 'N')
    return m

def hospital():
    m = M('Hospital', 88, 48, 38); m.plate('S')
    # alas laterales (8 pisos bajos) y torre central
    for (x0, x1) in ((4, 32), (56, 84)):
        m.box(x0, 1, 8, x1-x0, 20, 32, 'W'); m.box(x0, 1, 8, x1-x0, 2, 32, 's')
        m.vent4(x0, 8, x1, 40, 3, 21, ww=4, gap=2, wh=3, fh=6, off=2, mg=2, sill='S')
        m.box(x0, 21, 8, x1-x0, 1, 32, 'S'); m.parapeto(x0, 8, x1, 40, 22, 'W')
        m.aire(x0+6, 22, 14); m.aire(x0+16, 22, 24)
        for x in range(x0+2, x1-2, 6): m.box(x, 9, 40, 1, 1, 1, 'B') if False else None
    m.box(30, 1, 6, 28, 34, 34, 'W'); m.box(30, 1, 6, 28, 2, 34, 's')
    m.vent4(30, 6, 58, 40, 3, 34, ww=4, gap=2, wh=3, fh=6, off=2, mg=2, sill='S')
    m.box(30, 35, 6, 28, 1, 34, 'S'); m.parapeto(30, 6, 58, 40, 36, 'W', 1)
    m.box(40, 36, 16, 8, 2, 8, 's')                                          # cuarto de máquinas
    # cruz roja grande en la torre
    cx = 44
    m.box(cx-2, 22, 40, 5, 13, 1, 'R'); m.box(cx-6, 26, 40, 13, 5, 1, 'R')
    # helipuerto
    m.box(48, 36, 30, 8, 1, 8, 'D'); m.box(49, 37, 31, 6, 1, 6, 'D') if False else None
    m.box(50, 36, 32, 1, 1, 4, 'Y'); m.box(54, 36, 32, 1, 1, 4, 'Y'); m.box(50, 36, 34, 5, 1, 1, 'Y')
    # entrada y marquesina de urgencias
    m.box(34, 1, 40, 20, 1, 6, 'S')
    m.box(38, 1, 40, 12, 5, 1, 'V'); m.box(38, 5, 40, 12, 1, 1, 'v'); m.box(43, 1, 40, 2, 5, 1, 'K')   # puertas de vidrio
    m.box(34, 7, 40, 20, 1, 6, 'W'); m.box(34, 8, 40, 20, 1, 6, 'R') if False else None
    for x in (34, 53): m.box(x, 1, 44, 1, 6, 1, 'S')
    m.box(34, 7, 45, 20, 2, 1, 'R'); m.box(40, 7, 45, 8, 2, 1, 'W')
    # ambulancias estacionadas
    for ax in (8, 14):
        m.box(ax, 1, 42, 4, 3, 2, 'W'); m.box(ax+3, 1, 42, 1, 2, 2, 'V'); m.box(ax+1, 3, 42, 2, 1, 1, 'R')
    return m

def edificio_rojo():
    m = M('Edificio_rojo', 18, 18, 38); m.plate('S')
    x0, z0, x1, z1 = 2, 2, 16, 16
    m.box(x0, 1, z0, x1-x0, 26, z1-z0, 'R'); m.box(x0, 1, z0, x1-x0, 3, z1-z0, 'r')
    for y in (7, 13, 19, 25): m.box(x0, y, z0, x1-x0, 1, z1-z0, 'W')         # cintas blancas
    m.vent4(x0, z0, x1, z1, 4, 26, ww=2, gap=2, wh=3, fh=6, off=1, mg=2, frame=None, sill='W')
    m.puerta(9, z1, 4, 4, 'K', 'W'); m.toldo(6, z1, 6, 5, 2, 'W', 'K')
    m.box(3, 27, 3, 12, 1, 12, 'W'); m.box(4, 28, 4, 10, 6, 10, 'R'); m.box(4, 28, 4, 10, 1, 10, 'r')   # retranqueo
    m.vent4(4, 4, 14, 14, 28, 34, ww=2, gap=2, wh=3, fh=6, off=1, mg=2)
    m.box(4, 34, 4, 10, 1, 10, 'W'); m.box(5, 35, 5, 8, 1, 8, 'D')
    m.box(6, 36, 6, 4, 2, 4, 'N'); m.box(5, 38, 5, 6, 1, 6, 'D') if False else None   # tanque de agua
    m.box(11, 36, 10, 1, 1, 1, 'K'); m.box(12, 36, 10, 1, 3, 1, 'K')
    return m

def torre_dorada():
    m = M('Torre_dorada', 56, 56, 156); m.plate('S')
    m.box(4, 1, 4, 48, 1, 48, 'w')                                           # plaza
    # podio
    m.box(8, 2, 8, 40, 14, 40, 'L'); m.box(8, 2, 8, 40, 2, 40, 'l')
    m.vent4(8, 8, 48, 48, 2, 16, ww=4, gap=3, wh=7, fh=14, off=3, mg=3, glass='V', top='v', frame='l')
    m.box(8, 16, 8, 40, 1, 40, 'l'); m.box(10, 17, 10, 36, 1, 36, 'S')
    for x in range(10, 46, 5): m.box(x, 2, 48, 2, 14, 2, 'L')                # columnas del frente
    m.box(8, 15, 48, 40, 2, 3, 'l')
    m.box(24, 2, 47, 8, 9, 1, 'K'); m.box(25, 3, 47, 6, 8, 1, 'V'); m.box(28, 2, 47, 1, 9, 1, 'K')
    m.box(20, 11, 48, 16, 3, 1, 'D') if False else None
    # fuste: bandas de vidrio y oro con retranqueos
    def tramo(x0, z0, w, y0, y1, vidrio='V'):
        m.box(x0, y0, z0, w, y1-y0, w, 'L')
        for f in range(y0, y1, 6):
            for cara in 'fblr':
                m.vent(cara, x0, z0, x0+w, z0+w, f, min(y1, f+6), ww=w-6, gap=0, wh=3, fh=6, off=2, mg=3, glass=vidrio, top=None)
            m.box(x0, f, z0, w, 1, w, 'l')
    tramo(14, 14, 28, 17, 56)
    m.box(13, 56, 13, 30, 1, 30, 'l')
    tramo(16, 16, 24, 57, 90)
    m.box(15, 90, 15, 26, 1, 26, 'l')
    tramo(18, 18, 20, 91, 118)
    m.box(17, 118, 17, 22, 1, 22, 'l')
    tramo(21, 21, 14, 119, 138)
    m.box(20, 138, 20, 16, 2, 16, 'l')
    # corona y aguja
    m.box(22, 140, 22, 12, 3, 12, 'L'); m.box(24, 143, 24, 8, 3, 8, 'Y'); m.box(26, 146, 26, 4, 3, 4, 'L')
    m.box(27, 149, 27, 2, 5, 2, 'l'); m.box(27, 154, 27, 2, 2, 2, 'Y')
    return m

def farmacia():
    m = M('Farmacia', 24, 20, 20); m.plate('S')
    x0, z0, x1, z1 = 2, 2, 22, 16
    m.box(x0, 1, z0, x1-x0, 12, z1-z0, 'W'); m.box(x0, 1, z0, x1-x0, 2, z1-z0, 's')
    m.box(4, 3, z1-1, 16, 6, 1, 'V'); m.box(4, 8, z1-1, 16, 1, 1, 'v')       # vitrina
    for x in (4, 11, 12, 19): m.box(x, 3, z1-1, 1, 6, 1, 'K')
    m.box(11, 3, z1-1, 2, 5, 1, 'S')
    m.toldo(2, z1, 20, 10, 3, 'G', 'W')
    m.vent('l', x0, z0, x1, z1, 1, 13, ww=3, gap=3, wh=3, fh=12, off=4, mg=3, frame='G')
    m.vent('r', x0, z0, x1, z1, 1, 13, ww=3, gap=3, wh=3, fh=12, off=4, mg=3, frame='G')
    m.box(x0, 13, z0, x1-x0, 1, z1-z0, 'S'); m.parapeto(x0, z0, x1, z1, 14, 'W')
    # cruz verde (letrero)
    m.box(9, 11, z1, 6, 1, 1, 'K') if False else None
    m.box(10, 12, z1-1, 4, 1, 1, 'G'); m.box(11, 11, z1-1, 2, 1, 1, 'G')
    m.box(21, 6, z1, 1, 1, 1, 'K') if False else None
    m.box(9, 14, 5, 6, 1, 1, 'G') if False else None
    m.box(10, 15, z1-1, 4, 1, 1, 'K') if False else None
    m.box(9, 14, 10, 6, 1, 1, 'G'); m.box(11, 14, 8, 2, 5, 1, 'G'); m.box(11, 14, 12, 2, 1, 1, 'G')   # cruz en el techo
    m.aire(15, 15, 5)
    return m

def hotel():
    m = M('Hotel', 32, 28, 40); m.plate('S')
    x0, z0, x1, z1 = 3, 3, 29, 23
    m.box(x0, 1, z0, x1-x0, 30, z1-z0, 'n'); m.box(x0, 1, z0, x1-x0, 5, z1-z0, 'w')
    m.vent4(x0, z0, x1, z1, 6, 31, ww=2, gap=2, wh=3, fh=5, off=1, mg=2, sill='W')
    for f in range(6, 31, 5):                                                  # balcones corridos en el frente
        m.box(x0+1, f, z1, x1-x0-2, 1, 2, 'S'); m.box(x0+1, f+1, z1+1, x1-x0-2, 1, 1, 'W')
    m.box(5, 1, z1-1, 22, 4, 1, 'V'); m.box(5, 4, z1-1, 22, 1, 1, 'v')       # vestíbulo
    m.puerta(16, z1, 4, 4, 'K', 'L', y=1)
    m.box(9, 5, z1, 14, 1, 4, 'R'); m.box(9, 6, z1+3, 14, 1, 1, 'L')         # marquesina de entrada
    for x in (9, 22): m.box(x, 1, z1+3, 1, 4, 1, 'L')
    m.box(x0, 31, z0, x1-x0, 1, z1-z0, 'S'); m.parapeto(x0, z0, x1, z1, 32, 'n')
    m.box(8, 33, z1-1, 16, 4, 1, 'b'); m.box(9, 34, z1-1, 14, 2, 1, 'L')     # letrero del hotel
    m.box(10, 32, 8, 12, 1, 8, 'V'); m.box(10, 33, 8, 12, 1, 1, 'W')         # piscina en la azotea
    m.aire(20, 32, 10)
    return m

def torres_gemelas():
    m = M('Torres_gemelas', 56, 44, 52); m.plate('S')
    m.box(2, 1, 2, 52, 1, 40, 'w')
    m.box(4, 2, 12, 48, 5, 26, 's'); m.vent('f', 4, 12, 52, 38, 2, 7, ww=5, gap=2, wh=3, fh=5, off=1, mg=2)
    m.box(4, 7, 12, 48, 1, 26, 'S')
    for tx in (6, 30):
        m.box(tx, 8, 16, 20, 40, 20, 'B') if False else None
        m.box(tx, 2, 16, 20, 42, 20, 'B'); m.box(tx, 2, 16, 20, 2, 20, 'b')
        m.vent4(tx, 16, tx+20, 36, 4, 44, ww=3, gap=1, wh=4, fh=5, off=1, mg=2, glass='V', top=None)
        for y in range(6, 44, 5): m.box(tx, y, 16, 20, 1, 20, 'S')
        for cx, cz in ((tx, 16), (tx+19, 16), (tx, 35), (tx+19, 35)): m.box(cx, 2, cz, 1, 42, 1, 'b')
        m.box(tx, 44, 16, 20, 1, 20, 'S'); m.box(tx+4, 45, 20, 12, 3, 12, 'B'); m.box(tx+4, 45, 20, 12, 1, 12, 'b')
        m.box(tx+9, 48, 25, 2, 4, 2, 'K')
    m.box(26, 36, 22, 4, 3, 8, 'V'); m.box(26, 35, 22, 4, 1, 8, 's'); m.box(26, 39, 22, 4, 1, 8, 's')   # puente entre torres
    m.puerta(28, 40 if False else 38, 6, 5, 'V', 'K', y=2) if False else None
    m.box(24, 2, 37, 8, 5, 1, 'K'); m.box(25, 2, 37, 6, 4, 1, 'V')
    return m

def oficina_moderna():
    m = M('Oficina_moderna', 32, 28, 34); m.plate('S')
    x0, z0, x1, z1 = 3, 3, 29, 23
    m.box(x0, 1, z0, x1-x0, 22, z1-z0, 'v')
    m.vent4(x0, z0, x1, z1, 1, 23, ww=4, gap=0, wh=4, fh=5, off=1, mg=1, glass='V', top=None)
    for y in range(1, 24, 5): m.box(x0, y, z0, x1-x0, 1, z1-z0, 'S')
    for x in range(x0, x1, 5):
        for z in (z0, z1-1): m.box(x, 1, z, 1, 22, 1, 'S')
    m.box(x0, 1, z0, x1-x0, 3, z1-z0, 'D'); m.box(6, 1, z1-1, 20, 3, 1, 'V')   # planta baja
    m.puerta(16, z1, 4, 3, 'K', 'S', y=1)
    # remate escalonado con terraza verde
    m.box(x0+4, 23, z0+4, x1-x0-8, 7, z1-z0-8, 'v'); m.vent4(x0+4, z0+4, x1-4, z1-4, 23, 30, ww=4, gap=0, wh=4, fh=5, off=1, mg=1, glass='V', top=None)
    m.box(x0, 23, z0, x1-x0, 1, z1-z0, 'S'); m.box(x0+2, 24, z0+2, 4, 1, z1-z0-4, 'G'); m.box(x1-6, 24, z0+2, 4, 1, z1-z0-4, 'G')
    m.box(x0+4, 30, z0+4, x1-x0-8, 1, z1-z0-8, 'S'); m.aire(12, 31, 10); m.aire(17, 31, 13)
    m.box(15, 31, 8, 1, 3, 1, 'K')
    return m

def edificio_colorido():
    m = M('Edificio_colorido', 24, 24, 36); m.plate('S')
    x0, z0, x1, z1 = 3, 3, 21, 21
    cols = ['O', 'Y', 'T', 'P', 'p', 'B']
    m.box(x0, 1, z0, x1-x0, 4, z1-z0, 'W'); m.box(5, 1, z1-1, 14, 3, 1, 'V'); m.puerta(12, z1, 4, 3, 'K', 'W')
    m.toldo(7, z1, 10, 5, 2, 'R', 'W')
    for i, c in enumerate(cols):
        y = 5 + i*5
        m.box(x0, y, z0, x1-x0, 5, z1-z0, c)
        m.vent4(x0, z0, x1, z1, y, y+5, ww=3, gap=2, wh=3, fh=5, off=1, mg=2, glass='V', top=None, frame='W')
        m.box(x0-1 if x0 > 0 else x0, y+4, z0-1, x1-x0+2, 1, z1-z0+2, 'W') if i % 2 == 0 else None
    m.box(x0, 35, z0, x1-x0, 1, z1-z0, 'W')
    m.box(5, 36, 12, 1, 2, 6, 'K') if False else None
    return m

def centro_comercial():
    m = M('Centro_comercial', 28, 24, 24); m.plate('S')
    x0, z0, x1, z1 = 2, 2, 26, 20
    m.box(x0, 1, z0, x1-x0, 12, z1-z0, 'w'); m.box(x0, 1, z0, x1-x0, 2, z1-z0, 's')
    m.box(4, 3, z1-1, 20, 7, 1, 'V'); m.box(4, 9, z1-1, 20, 1, 1, 'v')       # gran vitrina
    for x in (4, 9, 14, 19, 23): m.box(x, 3, z1-1, 1, 7, 1, 'K')
    m.box(12, 3, z1-1, 4, 5, 1, 'K'); m.box(12, 3, z1-1, 2, 5, 1, 'S') if False else None
    # fachada con tiendas de colores
    for i, c in enumerate(['R', 'Y', 'G', 'B', 'P', 'O']):
        m.box(x0+1+i*4, 10, z1, 4, 2, 1, c)
    m.box(x0, 12, z0, x1-x0, 1, z1-z0, 'S')
    m.vent('l', x0, z0, x1, z1, 1, 12, ww=3, gap=3, wh=3, fh=12, off=5, mg=3)
    m.vent('r', x0, z0, x1, z1, 1, 12, ww=3, gap=3, wh=3, fh=12, off=5, mg=3)
    m.box(8, 13, z1-1, 12, 4, 1, 'D'); m.box(9, 14, z1-1, 10, 2, 1, 'Y')     # letrero
    m.box(10, 13, 8, 8, 2, 6, 'V')                                           # tragaluz
    m.box(2, 13, 2, 24, 1, 18, 'S') if False else None
    m.parapeto(x0, z0, x1, z1, 13, 'W'); m.aire(5, 14, 6); m.aire(20, 14, 12)
    return m

def comisaria():
    m = M('Comisaria', 56, 36, 31); m.plate('S')
    x0, z0, x1, z1 = 3, 3, 53, 31
    m.box(x0, 1, z0, x1-x0, 18, z1-z0, 'b'); m.box(x0, 1, z0, x1-x0, 3, z1-z0, 'D')
    m.vent4(x0, z0, x1, z1, 4, 19, ww=4, gap=3, wh=3, fh=6, off=1, mg=3, glass='V', top='v', sill='W', frame='W')
    for y in (7, 13): m.box(x0, y, z0, x1-x0, 1, z1-z0, 'W') if False else None
    m.box(x0, 12, z0, x1-x0, 1, z1-z0, 'W')
    # cuerpo central más alto con escudo
    m.box(18, 19, 8, 20, 6, 20, 'b'); m.vent4(18, 8, 38, 28, 19, 25, ww=4, gap=3, wh=3, fh=6, off=1, mg=2, sill='W')
    m.box(18, 25, 8, 20, 1, 20, 'W'); m.parapeto(18, 8, 38, 28, 26, 'D')
    m.box(26, 14, z1-1, 4, 4, 1, 'Y'); m.box(27, 15, z1-1, 2, 2, 1, 'b')    # escudo
    m.box(24, 17, z1-1, 8, 1, 1, 'Y') if False else None
    # entrada
    m.box(22, 1, z1-1, 12, 6, 1, 'W'); m.box(23, 1, z1-1, 10, 5, 1, 'V'); m.box(27, 1, z1-1, 2, 5, 1, 'K')
    m.box(20, 7, z1, 16, 1, 3, 'W'); m.box(20, 1, z1+1, 1, 6, 1, 'W'); m.box(35, 1, z1+1, 1, 6, 1, 'W')
    m.box(18, 1, 31, 20, 1, 3, 'S')
    # cocheras a los lados
    for gx in (7, 41):
        m.box(gx, 1, z1-1, 8, 6, 1, 'S'); 
        for y in (2, 4): m.box(gx, y, z1-1, 8, 1, 1, 's')
    # sirenas azul y roja en el techo
    m.box(x0+2, 13, z0+2, 4, 2, 4, 'R'); m.box(x0+6, 13, z0+2, 4, 2, 4, 'B')
    m.box(x1-10, 13, z0+2, 4, 2, 4, 'R'); m.box(x1-6, 13, z0+2, 4, 2, 4, 'B')
    m.box(44, 26, 18, 1, 6, 1, 'K'); m.box(45, 29, 18, 4, 2, 1, 'R')          # mástil con bandera
    # patrullas estacionadas
    for px in (6, 40):
        m.box(px, 1, 33, 8, 2, 3, 'W'); m.box(px+2, 3, 33, 4, 1, 3, 'K'); m.box(px+3, 4, 34, 2, 1, 1, 'R')
    return m

LISTA = [casa_roja, casa_azul, casa_moderna, banco, cafe, hospital, edificio_rojo, torre_dorada, farmacia, hotel,
         torres_gemelas, oficina_moderna, edificio_colorido, centro_comercial, comisaria]
NOMBRES = ['casa_roja','casa_azul','casa_moderna','banco','cafe','hospital','edificio_rojo','torre_dorada','farmacia','hotel',
           'torres_gemelas','oficina_moderna','edificio_colorido','centro_comercial','comisaria']
os.makedirs(OUT, exist_ok=True)
for f, n in zip(LISTA, NOMBRES):
    m = f(); open(os.path.join(OUT, n + '.txt'), 'w').write(m.texto()); print(n, m.W, m.D)
