#!/usr/bin/env python3
# Crea src/ui/fuente_datos.h (letras de 8x14 puntos) a partir de una fuente del sistema.
import os
from PIL import Image, ImageDraw, ImageFont
RUTAS = ['/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf', '/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf']
fuente = ImageFont.truetype(next(r for r in RUTAS if os.path.exists(r)), 13)
chars = [chr(c) for c in range(32, 127)] + list('áéíóúüñÁÉÍÓÚÜÑ¿¡°·○×')
out = ['// Generado por tools/hacer_fuente.py: no editar a mano', '#define FUENTE_W 8', '#define FUENTE_H 14', '#define FUENTE_N %d' % len(chars),
       'static const unsigned short FUENTE_COD[FUENTE_N] = {' + ','.join(str(ord(c)) for c in chars) + '};',
       'static const unsigned char FUENTE_BITS[FUENTE_N][FUENTE_H] = {']
for c in chars:
    im = Image.new('L', (8, 14), 0)
    ImageDraw.Draw(im).text((0, 0), c, font=fuente, fill=255)
    filas = []
    for y in range(14):
        b = 0
        for x in range(8):
            if im.getpixel((x, y)) > 110: b |= 0x80 >> x
        filas.append('0x%02x' % b)
    out.append('  {' + ','.join(filas) + '},')
out.append('};')
open(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'src', 'ui', 'fuente_datos.h'), 'w').write('\n'.join(out) + '\n')
print('fuente lista:', len(chars), 'letras')
