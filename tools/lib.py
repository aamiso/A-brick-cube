# Biblioteca del pack: paleta FIJA + herramientas para armar edificios de cubitos
PAL = {
 'W':'#f4f4f2','w':'#e3dccd','S':'#c9ccd1','s':'#8e949e','D':'#4a4f58','K':'#1f2126',
 'R':'#d9382c','r':'#8f2a24','O':'#e8742a','Y':'#f2c230','y':'#f6e7a1',
 'G':'#3aa655','g':'#24703c','T':'#2bb5a8','B':'#2f62c9','b':'#1f3f8a',
 'V':'#9fdcf0','v':'#4a8fb0','P':'#8a3fc0','p':'#e88fb5','N':'#6b4a2f','n':'#c9a47a',
 'L':'#e8b923','l':'#a87d12'}
class M:
    def __init__(s, nombre, W, D, H):
        s.nombre, s.W, s.D, s.H = nombre, W, D, H
        s.g = [[['.']*W for _ in range(D)] for _ in range(H)]
    def p(s, x, y, z, c):
        if 0 <= x < s.W and 0 <= y < s.H and 0 <= z < s.D: s.g[y][z][x] = c
    def box(s, x, y, z, w, h, d, c):
        for yy in range(y, y+h):
            for zz in range(z, z+d):
                for xx in range(x, x+w): s.p(xx, yy, zz, c)
    def ring(s, x, y, z, w, h, d, c):          # solo las paredes (sin interior)
        for yy in range(y, y+h):
            for zz in range(z, z+d):
                for xx in range(x, x+w):
                    if xx in (x, x+w-1) or zz in (z, z+d-1): s.p(xx, yy, zz, c)
    def plate(s, c='S', borde=None, bc=None):   # piso de la parcela (capa 1)
        s.box(0, 0, 0, s.W, 1, s.D, c)
        if borde: 
            s.ring(0, 0, 0, s.W, 1, s.D, bc)
    # ventanas en una cara. cara: 'f' frente(+z) 'b' atrás 'l' izq 'r' der
    def vent(s, cara, x0, z0, x1, z1, ya, yb, ww=3, gap=2, wh=3, fh=6, off=2, mg=2, glass='V', top='v', sill=None, frame=None):
        horiz = cara in 'fb'
        a0, a1 = (x0, x1) if horiz else (z0, z1)
        fixed = {'f': z1-1, 'b': z0, 'l': x0, 'r': x1-1}[cara]
        y = ya + off
        while y + wh <= yb:
            pos = a0 + mg
            while pos + ww <= a1 - mg:
                for i in range(ww):
                    for j in range(wh):
                        c = top if (j == wh-1 and top) else glass
                        if frame and (i in (0, ww-1) or j == 0) and ww > 2 and wh > 2: c = frame
                        s._cara(horiz, fixed, pos+i, y+j, c)
                    if sill: s._cara(horiz, fixed, pos+i, y-1, sill)
                pos += ww + gap
            y += fh
    def _cara(s, horiz, fixed, a, y, c):
        if horiz: s.p(a, y, fixed, c)
        else: s.p(fixed, y, a, c)
    def vent4(s, x0, z0, x1, z1, ya, yb, **k):
        for c in 'fblr': s.vent(c, x0, z0, x1, z1, ya, yb, **k)
    def puerta(s, cx, z1, w=4, h=5, c='N', marco='K', y=1):   # puerta en el frente (+z): z1 = z de la cara
        for i in range(w):
            for j in range(h): s.p(cx-w//2+i, y+j, z1-1, c)
        for i in range(-1, w+1): s.p(cx-w//2+i, y+h, z1-1, marco)
        for j in range(h): s.p(cx-w//2-1, y+j, z1-1, marco); s.p(cx-w//2+w, y+j, z1-1, marco)
    def toldo(s, x, z1, w, y, prof=3, c1='R', c2='W', grosor=1):   # toldo a rayas que sale hacia el frente
        for i in range(w):
            c = c1 if (i//2) % 2 == 0 else c2
            for k in range(prof):
                s.p(x+i, y+(prof-1-k)//2 if grosor else y, z1+k, c)
    def techo_dos_aguas(s, x0, z0, x1, z1, y, c, eje='x', vuelo=1, c2=None):
        # el caballete corre a lo largo de x (eje='x') o de z
        if eje == 'x':
            k = 0
            while z0-vuelo+k <= z1+vuelo-1-k:
                s.box(x0-vuelo, y+k, z0-vuelo+k, (x1-x0)+2*vuelo, 1, (z1-z0)+2*vuelo-2*k, c); k += 1
            return k
        k = 0
        while x0-vuelo+k <= x1+vuelo-1-k:
            s.box(x0-vuelo+k, y+k, z0-vuelo, (x1-x0)+2*vuelo-2*k, 1, (z1-z0)+2*vuelo, c); k += 1
        return k
    def techo_piramide(s, x0, z0, x1, z1, y, c, vuelo=1):
        k = 0
        while x0-vuelo+k < x1+vuelo-k and z0-vuelo+k < z1+vuelo-k:
            s.box(x0-vuelo+k, y+k, z0-vuelo+k, (x1-x0)+2*vuelo-2*k, 1, (z1-z0)+2*vuelo-2*k, c); k += 1
        return k
    def parapeto(s, x0, z0, x1, z1, y, c, h=1):
        s.ring(x0, y, z0, x1-x0, h, z1-z0, c)
    def aire(s, x, y, z, w=3, d=3):                       # equipo de aire acondicionado en el techo
        s.box(x, y, z, w, 2, d, 's'); s.box(x+1, y+2, z+1, max(1, w-2), 1, max(1, d-2), 'D')
    def texto(s):
        usados = set(c for y in s.g for z in y for c in z) - {'.'}
        top = max(i for i, y in enumerate(s.g) if any(c != '.' for z in y for c in z))
        o = ['modelo ' + s.nombre, '// Pack de edificios v1 - paleta fija']
        for k, v in PAL.items():
            if k in usados: o.append('color %s %s' % (k, v))
        for y in range(top+1):
            o.append('capa %d' % (y+1))
            for z in range(s.D): o.append(''.join(s.g[y][z]))
        return '\n'.join(o) + '\n'
