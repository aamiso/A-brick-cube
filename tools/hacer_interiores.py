# Genera src/world/interior_datos.h: cuartos extra y enlaces (puertas con números 1-4) de todos los lugares.
# Cada cuarto: 'N' = persona · C mostrador · B cama · T mesa · S banco/sofá · P planta · W estante · M máquina · F nevera · R alfombra · 1-4 = puertas
import sys
TEMAS = {
 'casa':   (0xd9b98a, 0xcba878, 0xf2e6d0, 0x8a5a3a, 0x8a5a3a, 0xc87a4a, 0xd9382c),
 'casab':  (0xb8d4e8, 0xa6c4da, 0xe8f0f6, 0x2f62c9, 0xf2f2f2, 0x5a8fd0, 0x2f62c9),
 'casam':  (0xe0e0e4, 0xcfcfd4, 0xf6f6f8, 0x303338, 0x8a5a3a, 0x9aa0a8, 0x3aa655),
 'banco':  (0xd9d4c4, 0xc9c3b0, 0x6a7a8a, 0x2a3440, 0xf2c230, 0x8a8f96, 0xf2c230),
 'cafe':   (0xc99a62, 0xb88650, 0x8c3b2b, 0x4a2a1f, 0x6b4426, 0xa9783f, 0xf2c230),
 'hosp':   (0xe8f1f2, 0xcfe3e6, 0xf2f2f2, 0x2aa7b8, 0xffffff, 0xd8e0e4, 0x3a8fd0),
 'farm':   (0xe6f3e6, 0xd2e8d2, 0xf4f4f4, 0x3aa655, 0xffffff, 0xe0ece0, 0x3aa655),
 'poli':   (0xcfd6e0, 0xbcc6d4, 0xdfe6f0, 0x2a4fb0, 0xffffff, 0x4a5a78, 0x2a4fb0),
 'hotel':  (0xd8c8a8, 0xc8b690, 0xf0e4cc, 0x7a2a2a, 0x6b4426, 0xb8862a, 0xd9382c),
 'mall':   (0xe8e4ee, 0xd6d0e0, 0xf4f2f8, 0x8a3fc0, 0xffffff, 0xc8a8e0, 0xe8742a),
 'ofic':   (0xd4d8de, 0xc2c8d0, 0xe6eaee, 0x2f62c9, 0xf2f2f2, 0x8a9098, 0x2f62c9),
 'oro':    (0xe8d49a, 0xd8be72, 0xf6ecc8, 0xa87d12, 0xe8b923, 0xd4a82a, 0xe8b923),
 'rojo':   (0xd8cfc8, 0xc8beb6, 0xefe6e0, 0x8f2a24, 0xf2f2f2, 0xb86a60, 0xd9382c),
 'color':  (0xf6e7a1, 0xf2d870, 0xffffff, 0xe8742a, 0xffffff, 0xe88fb5, 0x2bb5a8),
}
class R:
    def __init__(s, w, h): s.w, s.h = w, h; s.g = [['.'] * w for _ in range(h)]; s.gente = []
    def s(s, ch, x, z): s.g[z][x] = ch; return s
    def r(s, ch, x, z, w, h):
        for j in range(z, z + h):
            for i in range(x, x + w): s.g[j][i] = ch
        return s
    def n(s, x, z, rol): s.g[z][x] = 'N'; s.gente.append(rol); return s
    def filas(s): return [''.join(f) for f in s.g]
ROLES = {  # rol: (nombres, [3 tandas de frases])
 'recep':  (['Recepcionista Ana', 'Recepcionista Beto'], [["Bienvenido. ¿En qué te puedo ayudar?"], ["Todo en orden por aquí."], ["¿Buscas una sala? Sigue por la puerta."]]),
 'guardia':(['Guardia Pérez', 'Guardia Núñez'], [["Circule, por favor.", "Aquí no hay nada sospechoso."], ["Vigilo este lugar desde hace años."], ["No toques nada, ¿eh?"]]),
 'cliente':(['Carla', 'Don Mario', 'Lupita'], [["Estoy esperando mi turno."], ["Qué lugar tan bonito, ¿verdad?"], ["Vengo todas las semanas."]]),
 'vecino': (['Doña Elsa', 'Tío Beto', 'Abuela Inés'], [["¡Qué gusto verte por aquí!"], ["Pasa, pasa, estás en tu casa."], ["Cuidado con los cubitos del piso."]]),
 'nino':   (['Nico', 'Sofi', 'Mateo'], [["¡Mira, sé saltar en un pie!"], ["Estoy jugando a las escondidas."], ["¿Tienes un cubito de sobra?"]]),
 'chef':   (['Chef Gonzalo', 'Chef Marina'], [["Hoy cocino con cubitos frescos."], ["No me interrumpas, ¡se quema!"], ["El secreto es mucho cariño."]]),
 'doctor': (['Dr. Salas', 'Dra. Paredes', 'Dr. Ortiz'], [["Respira hondo... muy bien."], ["Estás sano como un cubo perfecto."], ["Mucha agua y buen descanso."]]),
 'enfermo':(['Paciente Rufino', 'Paciente Luz'], [["Ya casi me dan de alta."], ["Aquí cuidan muy bien."], ["Me caí de un carro, ¡qué susto!"]]),
 'policia':(['Oficial Vega', 'Oficial Cruz'], [["Todo bajo control, ciudadano."], ["Si ves algo raro, avísame."], ["La ley es la ley."]]),
 'preso':  (['Preso Tito', 'Preso Gus'], [["Yo solo choqué un poquito..."], ["¿Tienes un cubito para la fianza?"], ["Nadie me cree."]]),
 'gerente':(['Sr. Andrade', 'Sra. Lagos', 'Lic. Mora'], [["Estoy en una reunión importante."], ["Los números van muy bien."], ["Pase, pero sin hacer ruido."]]),
 'oficin': (['Pablo', 'Marisol', 'Joaquín', 'Rita'], [["Tengo mil correos por leer."], ["Necesito más café."], ["Hoy es viernes, ¿verdad?"]]),
 'vend':   (['Vendedora Rosa', 'Vendedor Pepe', 'Vendedora Tina'], [["¡Hoy hay ofertas de cubitos!"], ["Llévelo, se lo dejo barato."], ["¿Busca algo en especial?"]]),
 'huesped':(['Turista Karl', 'Turista Mei', 'Turista Dave'], [["Esta ciudad de cubos es increíble."], ["Vine a tomar fotos de la torre."], ["El hotel es muy cómodo."]]),
 'artista':(['Artista Vera', 'Músico Leo'], [["Estoy creando mi obra maestra."], ["El arte también va en cubitos."], ["Shh, estoy inspirándome."]]),
 'bot':    (['Cajero Luis', 'Cajera Pili'], [["¿Quieres abrir una cuenta?"], ["Aquí todo se cuenta dos veces."], ["Su turno, por favor."]]),
}
JOB = {'recep':'Recepción','guardia':'Guardia','cliente':'Cliente','vecino':'Vecino','nino':'Niño','chef':'Chef','doctor':'Médico','enfermo':'Paciente','policia':'Policía','preso':'Preso','gerente':'Gerente','oficin':'Oficinista','vend':'Vendedor','huesped':'Turista','artista':'Artista','bot':'Cajero'}
cuartos = []     # (lugar, nombre, tema, R)
enlaces = []     # (lugar, room, dig, to_room, to_dig)
cnt = {}         # lugar -> número de cuartos extra ya creados (empiezan después de los existentes)
BASE = {'Y': 1, 'Q': 1, 'M': 1, 'X': 1, 'c': 1}
def idx(l): return BASE.get(l, 0) + cnt.get(l, 0)
def room(l, nombre, tema, w, h):
    r = R(w, h); i = idx(l); cnt[l] = cnt.get(l, 0) + 1; cuartos.append((l, nombre, tema, r)); return r, i
def link(l, a, da, b, db): enlaces.append((l, a, da, b, db)); enlaces.append((l, b, db, a, da))
def new0(l, nombre, tema, w, h):   # cuarto 0 nuevo (para lugares sin cuarto base)
    cnt[l] = 0; return room(l, nombre, tema, w, h)

# ---------- CASAS ----------
r, a = new0('H', 'Casa roja: sala', 'casa', 14, 9)
r.r('R', 4, 3, 6, 3).s('S', 4, 6).s('S', 5, 6).s('T', 7, 4).r('W', 0, 1, 1, 3).s('P', 13, 1).s('P', 13, 7).s('1', 3, 0).s('2', 10, 0).n(10, 6, 'vecino')
r2, b = room('H', 'Casa roja: cocina', 'casa', 10, 7); r2.r('C', 2, 0, 6, 1).s('F', 0, 0).s('F', 9, 0).s('T', 4, 3).s('T', 5, 3).s('S', 3, 3).s('S', 6, 3).s('1', 4, 6).n(7, 4, 'chef'); link('H', a, '1', b, '1')
r3, c = room('H', 'Casa roja: dormitorio', 'casa', 10, 7); r3.s('B', 7, 0).s('B', 8, 0).s('W', 4, 0).r('R', 3, 3, 4, 2).s('P', 0, 6).s('1', 4, 6).n(2, 3, 'nino'); link('H', a, '2', c, '1')

r, a = new0('J', 'Casa azul: sala', 'casab', 16, 9)
r.r('R', 5, 3, 6, 4).s('S', 5, 7).s('S', 6, 7).s('S', 9, 7).s('T', 7, 5).r('W', 0, 1, 1, 4).s('P', 15, 1).s('P', 0, 8).s('1', 3, 0).s('2', 8, 0).s('3', 13, 0).n(12, 6, 'vecino')
r2, b = room('J', 'Casa azul: cocina', 'casab', 11, 7); r2.r('C', 1, 0, 7, 1).s('F', 0, 0).s('F', 10, 0).s('T', 5, 3).s('S', 4, 3).s('S', 6, 3).s('1', 5, 6).n(8, 4, 'chef'); link('J', a, '1', b, '1')
r3, c = room('J', 'Casa azul: cuarto del niño', 'casab', 10, 7); r3.s('B', 1, 0).s('B', 8, 0).s('W', 4, 0).r('R', 3, 3, 4, 2).s('1', 4, 6).n(5, 4, 'nino'); link('J', a, '2', c, '1')
r4, d = room('J', 'Casa azul: estudio', 'casab', 10, 7); r4.r('W', 0, 0, 3, 1).r('W', 7, 0, 3, 1).s('T', 4, 3).s('T', 5, 3).s('S', 4, 4).s('P', 9, 6).s('1', 4, 6).n(2, 3, 'artista'); link('J', a, '3', d, '1')

r, a = new0('K', 'Casa moderna: sala', 'casam', 16, 9)
r.r('R', 4, 3, 8, 4).s('S', 4, 7).s('S', 5, 7).s('S', 10, 7).s('T', 7, 5).r('W', 13, 1, 3, 1).s('P', 0, 1).s('P', 15, 8).s('1', 4, 0).s('2', 11, 0).n(2, 5, 'vecino')
r2, b = room('K', 'Casa moderna: cocina', 'casam', 11, 7); r2.r('C', 1, 0, 8, 1).s('F', 10, 0).s('T', 4, 3).s('T', 5, 3).s('T', 6, 3).s('S', 4, 4).s('1', 5, 6).n(8, 3, 'chef'); link('K', a, '1', b, '1')
r3, c = room('K', 'Casa moderna: estudio', 'casam', 11, 7); r3.s('T', 3, 2).s('T', 4, 2).s('M', 8, 0).r('W', 0, 0, 2, 1).s('P', 10, 6).s('1', 5, 6).n(6, 3, 'artista'); link('K', a, '2', c, '1')

# ---------- BANCO, CAFÉ, FARMACIA, COMISARÍA, HOSPITAL (cuarto 0 ya existe) ----------
r, b = room('X', 'Banco: oficina del gerente', 'banco', 12, 7); r.s('T', 5, 2).s('T', 6, 2).s('S', 5, 4).s('W', 0, 0).s('W', 11, 0).s('P', 0, 6).s('1', 5, 6).n(8, 3, 'gerente'); link('X', 0, '1', b, '1')
r, c = room('X', 'Banco: bóveda', 'banco', 12, 7); r.r('M', 3, 0, 6, 1).r('W', 0, 1, 1, 3).r('W', 11, 1, 1, 3).r('R', 4, 3, 4, 2).s('1', 5, 6).n(9, 4, 'guardia'); link('X', 0, '2', c, '1')
r, b = room('Q', 'Café: cocina', 'cafe', 11, 7); r.r('C', 2, 0, 6, 1).s('F', 0, 0).s('F', 10, 0).s('T', 5, 3).s('S', 4, 3).s('S', 6, 3).s('1', 5, 6).n(3, 4, 'chef').n(8, 4, 'chef'); link('Q', 0, '1', b, '1')
r, b = room('M', 'Farmacia: almacén', 'farm', 11, 7); r.r('W', 0, 0, 11, 1).r('W', 0, 2, 3, 1).r('W', 8, 2, 3, 1).s('T', 5, 4).s('1', 5, 6).n(6, 3, 'oficin'); link('M', 0, '1', b, '1')
r, b = room('c', 'Comisaría: celdas', 'poli', 14, 8); r.r('B', 0, 0, 2, 1).r('B', 6, 0, 2, 1).r('B', 12, 0, 2, 1).r('W', 4, 2, 1, 2).r('W', 10, 2, 1, 2).s('1', 6, 7).n(3, 3, 'preso').n(8, 4, 'preso'); link('c', 0, '1', b, '1')
r, c = room('c', 'Comisaría: oficina del jefe', 'poli', 12, 7); r.s('T', 5, 2).s('T', 6, 2).s('S', 5, 4).r('W', 0, 0, 2, 1).s('M', 10, 0).s('P', 11, 6).s('1', 5, 6).n(8, 3, 'policia'); link('c', 0, '2', c, '1')
# hospital: recepción (0) -> pasillo -> consultorio, habitaciones, laboratorio
r, p = room('Y', 'Hospital: pasillo', 'hosp', 20, 5); r.s('1', 3, 4).s('2', 6, 0).s('3', 10, 0).s('4', 16, 0).s('P', 0, 2).s('P', 19, 2).s('S', 12, 4).n(14, 2, 'enfermo'); link('Y', 0, '1', p, '1')
r, c = room('Y', 'Hospital: consultorio', 'hosp', 10, 7); r.s('B', 1, 0).s('T', 5, 2).s('S', 5, 4).s('W', 8, 0).s('W', 9, 0).s('1', 4, 6).n(7, 4, 'doctor'); link('Y', p, '2', c, '1')
r, d = room('Y', 'Hospital: habitaciones', 'hosp', 14, 7); r.r('B', 0, 0, 2, 1).r('B', 6, 0, 2, 1).r('B', 12, 0, 2, 1).s('P', 4, 4).s('1', 6, 6).n(3, 3, 'enfermo').n(9, 3, 'enfermo'); link('Y', p, '3', d, '1')
r, e = room('Y', 'Hospital: laboratorio', 'hosp', 11, 7); r.r('M', 0, 0, 4, 1).r('W', 7, 0, 4, 1).s('T', 4, 3).s('T', 5, 3).s('1', 5, 6).n(6, 4, 'doctor'); link('Y', p, '4', e, '1')

# ---------- HOTEL ----------
r, a = new0('U', 'Hotel: vestíbulo', 'hotel', 18, 9)
r.r('C', 6, 2, 6, 1).r('R', 6, 4, 6, 3).s('S', 2, 3).s('S', 3, 3).s('S', 14, 3).s('S', 15, 3).s('P', 0, 1).s('P', 17, 1).s('P', 0, 8).s('P', 17, 8).s('1', 8, 0).n(9, 1, 'recep').n(4, 6, 'huesped')
r2, p = room('U', 'Hotel: pasillo de habitaciones', 'hotel', 20, 5); r2.s('1', 3, 4).s('2', 6, 0).s('3', 12, 0).s('4', 17, 0).r('R', 4, 1, 12, 3).s('P', 0, 2).s('P', 19, 2).n(10, 3, 'huesped'); link('U', a, '1', p, '1')
r3, c = room('U', 'Hotel: habitación 101', 'hotel', 10, 7); r3.s('B', 1, 0).s('B', 2, 0).s('T', 7, 0).s('W', 9, 0).r('R', 3, 2, 4, 3).s('1', 4, 6).n(6, 4, 'huesped'); link('U', p, '2', c, '1')
r4, d = room('U', 'Hotel: habitación 102', 'hotel', 10, 7); r4.s('B', 7, 0).s('B', 8, 0).s('T', 1, 0).s('P', 0, 6).r('R', 3, 2, 4, 3).s('1', 4, 6); link('U', p, '3', d, '1')
r5, e = room('U', 'Hotel: suite presidencial', 'hotel', 14, 8); r5.r('B', 5, 0, 3, 1).s('S', 2, 4).s('S', 3, 4).s('T', 10, 4).r('R', 4, 3, 6, 4).r('W', 0, 0, 2, 1).s('P', 13, 7).s('1', 6, 7).n(11, 2, 'gerente'); link('U', p, '4', e, '1')

# ---------- CENTRO COMERCIAL ----------
r, a = new0('s', 'Centro comercial: plaza', 'mall', 22, 10)
r.r('R', 5, 3, 12, 4).s('S', 6, 5).s('S', 15, 5).s('P', 2, 4).s('P', 19, 4).s('P', 11, 5).s('1', 3, 0).s('2', 10, 0).s('3', 18, 0).n(8, 6, 'cliente').n(14, 7, 'nino').n(11, 2, 'vend')
for nom, cual, col in (('tienda de ropa', 'a', 0), ('juguetería', 'b', 1), ('comida rápida', 'c', 2)):
    rr, i = room('s', 'Centro comercial: ' + nom, 'mall', 12, 7)
    if cual == 'c': rr.r('C', 1, 0, 8, 1).s('F', 10, 0).s('T', 3, 3).s('T', 7, 3).s('S', 2, 3).s('S', 8, 3).s('1', 5, 6).n(5, 1, 'chef').n(5, 4, 'cliente')
    else: rr.r('W', 0, 0, 12, 1).r('W', 0, 2, 3, 1).r('W', 9, 2, 3, 1).r('C', 4, 3, 4, 1).s('1', 5, 6).n(8, 4, 'vend')
    link('s', 0, str(col + 1), i, '1')

# ---------- OFICINA, EDIFICIOS ----------
r, a = new0('I', 'Oficina: recepción', 'ofic', 16, 9); r.r('C', 5, 2, 6, 1).r('R', 4, 4, 8, 3).s('S', 2, 4).s('S', 13, 4).s('P', 0, 8).s('P', 15, 8).s('1', 3, 0).s('2', 12, 0).n(8, 1, 'recep').n(13, 6, 'oficin')
r2, b = room('I', 'Oficina: sala de juntas', 'ofic', 12, 7); r2.r('T', 3, 2, 6, 2).r('S', 3, 1, 6, 1).r('S', 3, 4, 6, 1).s('W', 0, 0).s('M', 11, 0).s('1', 5, 6).n(1, 4, 'gerente'); link('I', a, '1', b, '1')
r3, c = room('I', 'Oficina: espacio de trabajo', 'ofic', 16, 8); r3.r('T', 2, 1, 3, 1).r('T', 8, 1, 3, 1).r('T', 2, 4, 3, 1).r('T', 8, 4, 3, 1).s('F', 15, 0).s('P', 0, 7).s('1', 7, 7).n(5, 2, 'oficin').n(12, 3, 'oficin').n(6, 5, 'oficin'); link('I', a, '2', c, '1')

r, a = new0('R', 'Edificio rojo: vestíbulo', 'rojo', 14, 8); r.r('R', 4, 3, 6, 3).s('S', 2, 3).s('S', 11, 3).s('P', 0, 7).s('P', 13, 7).s('1', 3, 0).s('2', 10, 0).n(7, 2, 'recep')
r2, b = room('R', 'Edificio rojo: oficina', 'rojo', 11, 7); r2.r('T', 2, 2, 2, 1).r('T', 7, 2, 2, 1).s('F', 10, 0).s('W', 0, 0).s('1', 5, 6).n(5, 3, 'oficin'); link('R', a, '1', b, '1')
r3, c = room('R', 'Edificio rojo: archivo', 'rojo', 11, 7); r3.r('W', 0, 0, 11, 1).r('W', 0, 2, 4, 1).r('W', 7, 2, 4, 1).s('T', 5, 4).s('1', 5, 6).n(5, 3, 'oficin'); link('R', a, '2', c, '1')

r, a = new0('Z', 'Edificio colorido: recepción', 'color', 14, 8); r.r('R', 4, 3, 6, 3).s('C', 6, 1).s('C', 7, 1).s('P', 0, 1).s('P', 13, 1).s('1', 3, 0).s('2', 10, 0).n(7, 4, 'recep')
r2, b = room('Z', 'Edificio colorido: estudio de arte', 'color', 12, 7); r2.r('R', 3, 2, 6, 3).s('T', 2, 1).s('T', 9, 1).r('W', 0, 0, 3, 1).s('P', 11, 6).s('1', 5, 6).n(6, 3, 'artista'); link('Z', a, '1', b, '1')
r3, c = room('Z', 'Edificio colorido: sala de música', 'color', 12, 7); r3.r('M', 0, 0, 3, 1).r('M', 9, 0, 3, 1).s('S', 4, 4).s('S', 7, 4).r('R', 3, 2, 6, 2).s('1', 5, 6).n(6, 2, 'artista'); link('Z', a, '2', c, '1')

# ---------- TORRE DORADA ----------
r, a = new0('V', 'Torre Dorada: gran vestíbulo', 'oro', 22, 10)
r.r('R', 6, 4, 10, 3).r('C', 8, 2, 6, 1).s('P', 0, 1).s('P', 21, 1).s('P', 0, 9).s('P', 21, 9).s('S', 3, 5).s('S', 18, 5).s('1', 3, 0).s('2', 11, 0).s('3', 18, 0).n(13, 1, 'recep').n(8, 6, 'huesped').n(15, 6, 'guardia')
r2, b = room('V', 'Torre Dorada: mirador', 'oro', 18, 8); r2.r('R', 3, 2, 12, 3).s('S', 4, 5).s('S', 13, 5).s('P', 0, 0).s('P', 17, 0).s('T', 8, 3).s('1', 8, 7).n(5, 4, 'huesped').n(12, 4, 'huesped'); link('V', a, '1', b, '1')
r3, c = room('V', 'Torre Dorada: sala de oro', 'oro', 16, 8); r3.r('R', 4, 2, 8, 4).r('W', 0, 0, 16, 1).s('T', 7, 3).s('T', 8, 3).s('1', 7, 7).n(3, 4, 'guardia').n(12, 4, 'guardia'); link('V', a, '2', c, '1')
r4, d = room('V', 'Torre Dorada: oficina del presidente', 'oro', 14, 8); r4.s('T', 5, 2).s('T', 6, 2).s('T', 7, 2).s('S', 6, 4).r('W', 0, 0, 3, 1).r('W', 11, 0, 3, 1).r('R', 4, 3, 6, 3).s('1', 6, 7).n(9, 5, 'gerente'); link('V', a, '3', d, '1')

# ---------- TORRES GEMELAS ----------
r, a = new0('O', 'Torres gemelas: lobby', 'ofic', 18, 9); r.r('C', 6, 1, 6, 1).r('R', 5, 4, 8, 3).s('P', 0, 1).s('P', 17, 1).s('S', 2, 5).s('S', 15, 5).s('1', 3, 0).s('2', 14, 0).n(9, 2, 'recep').n(6, 6, 'oficin')
r2, b = room('O', 'Torre A: oficinas', 'ofic', 14, 8); r2.r('T', 2, 1, 3, 1).r('T', 9, 1, 3, 1).r('T', 2, 4, 3, 1).r('T', 9, 4, 3, 1).s('3', 6, 0).s('1', 6, 7).n(6, 3, 'oficin').n(12, 6, 'oficin'); link('O', a, '1', b, '1')
r3, c = room('O', 'Torre B: oficinas', 'ofic', 14, 8); r3.r('T', 2, 1, 3, 1).r('T', 9, 1, 3, 1).r('T', 2, 4, 3, 1).r('T', 9, 4, 3, 1).s('3', 6, 0).s('1', 6, 7).n(5, 3, 'gerente').n(11, 6, 'oficin'); link('O', a, '2', c, '1')
r4, d = room('O', 'Torres gemelas: puente', 'ofic', 16, 4); r4.s('3', 0, 1).s('4', 15, 1).s('P', 8, 0).s('S', 4, 3).s('S', 11, 3).n(8, 2, 'huesped')
link('O', b, '3', d, '3'); link('O', c, '3', d, '4')

# ---------- Salida C ----------
def esc(s): return s.replace('\\', '\\\\').replace('"', '\\"')
out = ['// Generado por tools/hacer_interiores.py: no editar a mano']
out.append('static const Cuarto EXTRA[] = {')
cont = {}
for (l, nombre, tema, r) in cuartos:
    fa, fb, pa, bo, me, cu, ac = TEMAS[tema]
    filas = ', '.join('"%s"' % f for f in r.filas())
    gs = []
    for k, rol in enumerate(r.gente):
        nombres, tandas = ROLES[rol]
        nm = nombres[(len(cuartos) + k) % len(nombres)]
        t = []
        for ta in tandas:
            fr = ', '.join('"%s"' % esc(x) for x in ta) + (', 0' * (3 - len(ta)))
            t.append('{ %s }' % fr)
        gs.append('{ "%s", "%s", { %s } }' % (nm, JOB[rol], ', '.join(t)))
    out.append(" { '%s', \"%s\", 0x%06x, 0x%06x, 0x%06x, 0x%06x, 0x%06x, 0x%06x, 0x%06x, %d, %d,\n   { %s },\n   { %s } }," % (l, esc(nombre), fa, fb, pa, bo, me, cu, ac, r.w, r.h, filas, ', '.join(gs)))
out.append('};')
out.append('static const struct { char lugar; int room; char dig; int to_room; char to_dig; } ENLACES[] = {')
for e in enlaces: out.append(" { '%s', %d, '%s', %d, '%s' }," % e)
out.append('};')
for (l, nombre, tema, r) in cuartos:
    assert r.w <= 22 and r.h <= 10, (nombre, r.w, r.h)
    assert len(r.gente) == sum(f.count('N') for f in r.filas()), nombre
open(sys.argv[1] if len(sys.argv) > 1 else 'src/world/interior_datos.h', 'w').write('\n'.join(out) + '\n')
print(len(cuartos), 'cuartos extra,', len(enlaces), 'enlaces')

# ---- comprobación: cada puerta se puede alcanzar caminando desde donde apareces ----
BLOQ = set('CBTSPWMFN')
def alcanzables(filas, w, h, ini):
    vis = {ini}; cola = [ini]
    while cola:
        x, z = cola.pop()
        for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, nz = x + dx, z + dz
            if 0 <= nx < w and 0 <= nz < h and (nx, nz) not in vis and filas[nz][nx] not in BLOQ:
                vis.add((nx, nz)); cola.append((nx, nz))
    return vis
def revisar(nombre, filas, w, h, sala0):
    puertas = [(x, z, filas[z][x]) for z in range(h) for x in range(w) if filas[z][x] in '1234']
    inicios = [(w // 2, h - 1)] if sala0 else []
    for (x, z, d) in puertas:
        dq = 1 if x == 0 else -1 if x == w - 1 else 0; dr = 1 if z == 0 else -1 if z == h - 1 else 0
        inicios.append((x + dq, z + dr))
    base = alcanzables(filas, w, h, inicios[0])
    for (x, z, d) in puertas:
        assert (x, z) in base, 'puerta inalcanzable: %s %s en (%d,%d)' % (nombre, d, x, z)
    assert all(i in base for i in inicios), 'zona aislada en ' + nombre
for (l, nombre, tema, r) in cuartos: revisar(nombre, r.filas(), r.w, r.h, idx_sala0 if False else (nombre.endswith(('sala','vestíbulo','plaza','recepción','lobby','sala')) and ': ' in nombre and nombre.split(': ')[1] in ('sala','vestíbulo','plaza','recepción','lobby','gran vestíbulo')))
print('puertas alcanzables: OK')
