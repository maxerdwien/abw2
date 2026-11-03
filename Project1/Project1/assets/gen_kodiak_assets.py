# Source generator for the Kodiak ship assets (sprites, attack sprites, roar ring, sound effects).
# Run from Project1/Project1:   python assets/gen_kodiak_assets.py assets
# Optional second arg = directory to write upscaled preview sheets into.

"""Generates all Kodiak ship assets: ship sprites (7 team colors), sawblade + harpoon
attack sprites (team colors + orange bounce variant), the roar ring, and 5 synthesized
sound effects. Pure Python (zlib/struct/wave/math) - no third party deps."""
import zlib, struct, wave, math, random, os, sys

ASSETS = sys.argv[1] if len(sys.argv) > 1 else "assets"
PREVIEW = sys.argv[2] if len(sys.argv) > 2 else None

# ---------------------------------------------------------------- PNG writer
def write_png(path, w, h, pixels):
    """pixels: list of rows, each a list of (r,g,b,a)"""
    raw = bytearray()
    for row in pixels:
        raw.append(0)
        for (r, g, b, a) in row:
            raw += bytes((r, g, b, a))
    def chunk(t, d):
        c = struct.pack('>I', len(d)) + t + d
        return c + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    png = b'\x89PNG\r\n\x1a\n'
    png += chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(bytes(raw), 9))
    png += chunk(b'IEND', b'')
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(png)

def upscale(pixels, k):
    out = []
    for row in pixels:
        big = []
        for p in row:
            big += [p] * k
        out += [big] * k
    return out

def ascii_to_pixels(art, palette):
    rows = [l for l in art.strip('\n').split('\n')]
    w = max(len(r) for r in rows)
    pixels = []
    for r in rows:
        r = r.ljust(w, '.')
        line = []
        for ch in r:
            if ch == '.':
                line.append((0, 0, 0, 0))
            else:
                c = palette[ch]
                line.append((c[0], c[1], c[2], 255))
        pixels.append(line)
    return w, len(rows), pixels

# ---------------------------------------------------------------- Ship sprite
# Same shade roles as the existing bear sprites (red master palette). Recolored
# per team with the exact mapping used by the panda/grizzly/polar sprites.
SHIP_RED = {
    'A': (12, 12, 12),      # outline
    'B': (184, 27, 27),     # highlight
    'N': (184, 96, 96),     # bright highlight
    'C': (184, 0, 0),       # base
    'D': (168, 0, 0),
    'M': (128, 0, 0),
    'E': (112, 0, 0),
    'G': (94, 0, 0),
    'F': (73, 0, 0),
    'S': (57, 0, 0),
    'H': (52, 52, 52),      # metal dark
    'K': (68, 68, 68),      # metal
    'I': (160, 164, 160),   # metal light
    'L': (181, 174, 174),   # eye shine
    'J': (24, 24, 24),      # nose / pupil
}

TEAM_MAP = {
    'red':     {},
    'blue':    {(184,27,27):(0,24,184),(184,0,0):(0,24,184),(168,0,0):(0,24,184),(112,0,0):(0,21,156),(73,0,0):(0,14,102),(94,0,0):(0,17,130),(128,0,0):(0,24,178),(184,96,96):(44,63,184),(57,0,0):(0,11,78)},
    'yellow':  {(184,27,27):(230,218,0),(184,0,0):(230,218,0),(168,0,0):(230,218,0),(112,0,0):(194,185,0),(73,0,0):(128,122,0),(94,0,0):(163,155,0),(128,0,0):(222,211,0),(184,96,96):(230,221,55),(57,0,0):(98,94,0)},
    'green':   {(184,27,27):(15,184,0),(184,0,0):(15,184,0),(168,0,0):(15,184,0),(112,0,0):(13,156,0),(73,0,0):(9,102,0),(94,0,0):(11,130,0),(128,0,0):(15,178,0),(184,96,96):(55,184,44),(57,0,0):(6,78,0)},
    'teal':    {(184,27,27):(0,134,184),(184,0,0):(0,134,184),(168,0,0):(0,134,184),(112,0,0):(0,115,156),(73,0,0):(0,75,102),(94,0,0):(0,95,130),(128,0,0):(0,131,178),(184,96,96):(44,147,184),(57,0,0):(0,58,78)},
    'magenta': {(184,27,27):(184,27,121),(184,0,0):(184,0,110),(168,0,0):(168,0,101),(112,0,0):(112,0,67),(73,0,0):(73,0,44),(94,0,0):(94,0,56),(128,0,0):(128,0,77),(184,96,96):(184,96,149),(57,0,0):(57,0,34)},
    'white':   {(184,27,27):(202,202,202),(184,0,0):(197,197,197),(168,0,0):(196,196,196),(112,0,0):(191,191,191),(73,0,0):(187,187,187),(94,0,0):(189,189,189),(128,0,0):(192,192,192),(184,96,96):(216,216,216),(57,0,0):(186,186,186)},
}

# Kodiak: the big one. Broad head with a long muzzle, small round ears set wide,
# heavy shoulder hump, thick armored hull, twin riveted engine pods and a stubby tail.
# Nose points up (direction of travel), like every other bear.
KODIAK = """
....AAA....AA....AAA....
...ACCEA..AJJA..AECCA...
..ACBCEGA.AJJA.AGECBCA..
..ACCCEGAADCCDAAGECCCA..
..AECCEGADCCCCDAGECCEA..
...AEEGAECCCCCCEAGEEA...
....AAAKLKDCCEKLKAAA....
.....ACKKKDCCEKKKCA.....
....ADCCCCDCCECCCCEA....
...ADCCCCCDCCECCCCCEA...
..ADCCCCCCDCCECCCCCCEA..
..ACCCCCCCCCCCCCCCCCEA..
.ADCCCBCCCCCCCCCCBCCEGA.
.ACCCBNBCCCCCCCCBNBCEGA.
ADCCCCBCCCECCECCCBCCEGFA
ACCCCCCCCECCCCECCCCEEGFA
AMCCCCCCCECCCCECCCCEEGFA
AMCCCCCCCCECCECCCCEEGGFA
.AMCCCCCCCCEECCCCEEEGFA.
.AAMCCCCCCCCCCCCEEEGGAA.
AKHAADCCCCCCCCCCEEGAAHKA
AHKIAAMDCCCCCCCEEGAAIKHA
AKHKKAAAMCCCCCEEGAAAKKHA
AHKHKHAAAMCCCEEGAAAHKHKA
AAAAAAAAAMCCCEEGAAAAAAAA
.........AMCCEGA........
.........AKJJKA.........
..........AJJA..........
...........AA...........
"""

def recolor(pixels, mapping):
    out = []
    for row in pixels:
        line = []
        for (r, g, b, a) in row:
            if a and (r, g, b) in mapping:
                nr, ng, nb = mapping[(r, g, b)]
                line.append((nr, ng, nb, a))
            else:
                line.append((r, g, b, a))
        out.append(line)
    return out

# ---------------------------------------------------------------- Attack palettes
# Bright "attack" team colors: (main, dark, light) - matched to the existing bullet tints.
ATTACK = {
    'Red':     ((255, 0, 0),    (170, 0, 0),    (255, 120, 120)),
    'Blue':    ((28, 60, 224),  (0, 0, 196),    (120, 188, 228)),
    'Yellow':  ((255, 216, 0),  (203, 160, 30), (255, 240, 140)),
    'Green':   ((2, 235, 52),   (0, 150, 30),   (130, 255, 150)),
    'Teal':    ((28, 178, 224), (0, 118, 196),  (120, 228, 203)),
    'Magenta': ((255, 0, 153),  (170, 0, 110),  (255, 130, 200)),
    'Orange':  ((255, 101, 31), (200, 80, 20),  (250, 180, 100)),
}
OUT = (12, 12, 12)
M_BRIGHT = (236, 252, 252)
M_LIGHT = (205, 212, 220)
M_MID = (140, 148, 160)
M_DARK = (80, 86, 96)

# ---------------------------------------------------------------- Sawblade (14x14)
def sawblade(main, dark, light):
    w = h = 14
    cx = cy = 6.5
    teeth = 8
    pixels = []
    for y in range(h):
        row = []
        for x in range(w):
            dx, dy = x - cx, y - cy
            r = math.hypot(dx, dy)
            ang = math.atan2(dy, dx)
            # tooth profile: radius bulges out at 8 points, saw-like (asymmetric)
            t = (ang * teeth / (2 * math.pi)) % 1.0
            tooth_r = 5.1 + (1.7 * (1.0 - t) if t > 0.45 else 0.0)
            if r > tooth_r + 0.6:
                row.append((0, 0, 0, 0))
            elif r > tooth_r - 0.4:
                row.append(OUT + (255,))
            elif r > 4.2:
                # blade rim, with brighter leading edges of the teeth
                row.append((M_LIGHT if t > 0.6 else M_MID) + (255,))
            elif r > 2.6:
                # disc body: subtle radial spokes
                spoke = int(((ang + math.pi) * 4 / (2 * math.pi)) % 1.0 < 0.18)
                row.append((M_DARK if spoke else M_MID) + (255,))
            elif r > 2.0:
                row.append(OUT + (255,))
            elif r > 0.9:
                row.append((light if (dx < 0 and dy < 0) else main) + (255,))
            else:
                row.append(dark + (255,))
        pixels.append(row)
    return w, h, pixels

# ---------------------------------------------------------------- Harpoon (7x17)
HARPOON_ART = """
...A...
..ALA..
..ALA..
.ALLLA.
.AILIA.
ALHLHLA
A.AIA..
..ACA..
..ABA..
..ACA..
..ABA..
..ACA..
..ACA..
..ADA..
.AKKKA.
.AK.KA.
..AAA..
"""
def harpoon(main, dark, light):
    pal = {'A': OUT, 'L': M_BRIGHT, 'I': M_LIGHT, 'H': M_MID, 'K': M_DARK,
           'C': main, 'B': light, 'D': dark}
    return ascii_to_pixels(HARPOON_ART, pal)

# ---------------------------------------------------------------- Roar ring (64x64, white, alpha)
def roar_ring():
    w = h = 64
    cx = cy = 31.5
    pixels = []
    for y in range(h):
        row = []
        for x in range(w):
            r = math.hypot(x - cx, y - cy)
            # main ring at r~29, soft inside, hard-ish outside edge
            d = r - 29.0
            if d > 2.0:
                a = 0.0
            elif d > 0.0:
                a = 1.0 - d / 2.0
            elif d > -6.0:
                a = 1.0 + d / 6.0
            else:
                a = 0.0
            # second, fainter inner ring for a "sound wave" feel
            d2 = r - 20.0
            if -3.0 < d2 < 1.0:
                a = max(a, 0.35 * (1.0 - abs(d2 + 1.0) / 2.0))
            # jagged "roar" spikes on the outer edge
            ang = math.atan2(y - cy, x - cx)
            spike = 0.5 + 0.5 * math.cos(ang * 12)
            if 0.0 < d < 2.0 + 3.0 * spike and spike > 0.7:
                a = max(a, 0.75 * (1.0 - d / (2.0 + 3.0 * spike)))
            v = int(255 * min(1.0, a))
            row.append((255, 255, 255, v))
        pixels.append(row)
    return w, h, pixels

# ---------------------------------------------------------------- Sound synth
SR = 44100
def write_wav(path, samples):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    peak = max(1e-9, max(abs(s) for s in samples))
    scale = 0.9 / peak
    with wave.open(path, 'wb') as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(b''.join(struct.pack('<h', int(max(-1, min(1, s * scale)) * 32767)) for s in samples))

def env(t, attack, hold, release, total):
    if t < attack:
        return t / attack
    if t < attack + hold:
        return 1.0
    x = (t - attack - hold) / max(1e-6, release)
    return max(0.0, 1.0 - x)

class Biquad:
    """2-pole resonant bandpass."""
    def __init__(self, f, q):
        self.set(f, q)
        self.x1 = self.x2 = self.y1 = self.y2 = 0.0
    def set(self, f, q):
        w0 = 2 * math.pi * f / SR
        alpha = math.sin(w0) / (2 * q)
        b0 = alpha; b1 = 0.0; b2 = -alpha
        a0 = 1 + alpha; a1 = -2 * math.cos(w0); a2 = 1 - alpha
        self.b0, self.b1, self.b2 = b0 / a0, b1 / a0, b2 / a0
        self.a1, self.a2 = a1 / a0, a2 / a0
    def run(self, x):
        y = self.b0 * x + self.b1 * self.x1 + self.b2 * self.x2 - self.a1 * self.y1 - self.a2 * self.y2
        self.x2, self.x1 = self.x1, x
        self.y2, self.y1 = self.y1, y
        return y

def sawblade_throw():
    """Whoosh with a fluttering rotor tone: throw + spin-up."""
    rnd = random.Random(1)
    dur = 0.55
    n = int(SR * dur)
    bp = Biquad(900, 1.2)
    out = []
    ph1 = ph2 = 0.0
    for i in range(n):
        t = i / SR
        # noise whoosh, band sweeping down
        bp.set(1800 - 1300 * (t / dur), 1.0)
        whoosh = bp.run(rnd.uniform(-1, 1)) * env(t, 0.03, 0.05, 0.4, dur)
        # spinning blade tone: two detuned partials, amplitude-modulated by rotation
        f = 620 + 380 * min(1.0, t / 0.18)
        ph1 += 2 * math.pi * f / SR
        ph2 += 2 * math.pi * f * 1.51 / SR
        flutter = 0.55 + 0.45 * math.sin(2 * math.pi * (18 + 26 * t / dur) * t)
        tone = (math.sin(ph1) * 0.7 + math.sin(ph2) * 0.3) * flutter * env(t, 0.02, 0.15, 0.35, dur)
        out.append(whoosh * 1.4 + tone * 0.5)
    return out

def sawblade_hit():
    """Metallic clang: inharmonic partials with a noise transient."""
    rnd = random.Random(2)
    dur = 0.35
    n = int(SR * dur)
    partials = [(1.0, 1.0, 9.0), (2.76, 0.6, 14.0), (5.40, 0.45, 20.0), (8.93, 0.25, 26.0), (13.3, 0.12, 34.0)]
    base = 540.0
    out = []
    for i in range(n):
        t = i / SR
        s = 0.0
        for ratio, amp, decay in partials:
            s += amp * math.sin(2 * math.pi * base * ratio * t) * math.exp(-decay * t)
        s += rnd.uniform(-1, 1) * math.exp(-90 * t) * 0.8
        out.append(s)
    return out

def harpoon_fire():
    """Pneumatic ka-chunk followed by a rattling chain paying out."""
    rnd = random.Random(3)
    dur = 0.7
    n = int(SR * dur)
    ring = Biquad(2400, 6.0)
    out = []
    ph = 0.0
    next_click = 0.09
    click_env = 0.0
    for i in range(n):
        t = i / SR
        # thump: pitch sweep 160 -> 55 Hz
        f = 55 + 105 * math.exp(-28 * t)
        ph += 2 * math.pi * f / SR
        thump = math.sin(ph) * math.exp(-14 * t) * 1.2
        # gas burst
        burst = rnd.uniform(-1, 1) * math.exp(-40 * t) * 0.9
        # chain: clicks at ~45 Hz -> 25 Hz through a metallic resonator
        if t >= next_click:
            click_env = 1.0
            gap = 0.018 + 0.022 * (t / dur) + rnd.uniform(-0.004, 0.004)
            next_click += gap
        click_env *= 0.94
        chain = ring.run(rnd.uniform(-1, 1) * click_env) * (1.0 if t > 0.09 else 0.0) * env(t, 0.1, 0.25, 0.3, dur) * 2.2
        out.append(thump + burst + chain)
    return out

def roar():
    """A bear roar: rough low sawtooth with pitch contour + vibrato, through two formant
    filters, plus breath noise."""
    rnd = random.Random(4)
    dur = 1.05
    n = int(SR * dur)
    f1 = Biquad(520, 3.0)
    f2 = Biquad(1150, 4.0)
    f3 = Biquad(260, 2.0)
    out = []
    ph = 0.0
    for i in range(n):
        t = i / SR
        # pitch: 78 -> 118 -> 70 Hz with fast vibrato and jitter (growl roughness)
        base = 78 + 40 * math.sin(math.pi * min(1.0, t / dur)) ** 0.7
        vib = 1.0 + 0.04 * math.sin(2 * math.pi * 27 * t) + 0.02 * rnd.uniform(-1, 1)
        f = base * vib
        ph = (ph + f / SR) % 1.0
        saw = 2.0 * ph - 1.0
        # add sub-harmonic pulse for chestiness
        pulse = 1.0 if ph < 0.18 else -0.25
        src = saw * 0.7 + pulse * 0.5
        breath = rnd.uniform(-1, 1) * 0.35
        x = src + breath
        y = f1.run(x) * 1.6 + f2.run(x) * 1.0 + f3.run(x) * 1.4 + x * 0.25
        # soft clip for aggression
        y = math.tanh(y * 1.8)
        out.append(y * env(t, 0.07, 0.55, 0.42, dur))
    return out

def roar_charge():
    """Rising rumble while the roar winds up: low growl pulses speeding up under a
    swept resonant noise bed. Slightly longer than the max charge time."""
    rnd = random.Random(5)
    dur = 1.0
    n = int(SR * dur)
    bp = Biquad(80, 2.5)
    out = []
    ph = 0.0
    for i in range(n):
        t = i / SR
        k = t / dur
        bp.set(70 + 260 * k, 2.0 + 3.0 * k)
        bed = bp.run(rnd.uniform(-1, 1)) * (0.3 + 0.7 * k) * 3.0
        # growl pulses: rate 12 -> 40 Hz
        rate = 12 + 28 * k
        ph = (ph + rate / SR) % 1.0
        pulse = math.exp(-ph * 9) * math.sin(2 * math.pi * 95 * t) * (0.25 + 0.75 * k)
        out.append((bed + pulse) * env(t, 0.05, 0.9, 0.05, dur))
    return out

# ---------------------------------------------------------------- Main
def main():
    ships = os.path.join(ASSETS, 'ships')
    attacks = os.path.join(ASSETS, 'attacks')
    sounds = os.path.join(ASSETS, 'sounds')

    w, h, base = ascii_to_pixels(KODIAK, SHIP_RED)
    previews = []
    for team, mapping in TEAM_MAP.items():
        px = recolor(base, mapping)
        write_png(os.path.join(ships, f'kodiak-{team}.png'), w, h, px)
        previews.append(px)

    for team, (main_c, dark, light) in ATTACK.items():
        sw, sh, spx = sawblade(main_c, dark, light)
        write_png(os.path.join(attacks, f'sawblade{team}.png'), sw, sh, spx)
        hw, hh, hpx = harpoon(main_c, dark, light)
        write_png(os.path.join(attacks, f'harpoon{team}.png'), hw, hh, hpx)

    rw, rh, rpx = roar_ring()
    write_png(os.path.join(attacks, 'roar_ring.png'), rw, rh, rpx)

    write_wav(os.path.join(sounds, 'sawblade_throw.wav'), sawblade_throw())
    write_wav(os.path.join(sounds, 'sawblade_hit.wav'), sawblade_hit())
    write_wav(os.path.join(sounds, 'harpoon.wav'), harpoon_fire())
    write_wav(os.path.join(sounds, 'roar.wav'), roar())
    write_wav(os.path.join(sounds, 'roar_charge.wav'), roar_charge())

    if PREVIEW:
        # ship sheet: all 7 team colors side by side at 6x, plus attack sprites
        k = 6
        gap = 2
        sheet_w = len(previews) * (w + gap) - gap
        sheet = [[(40, 40, 40, 255)] * sheet_w for _ in range(h)]
        for idx, px in enumerate(previews):
            for y in range(h):
                for x in range(w):
                    p = px[y][x]
                    if p[3]:
                        sheet[y][idx * (w + gap) + x] = p
        big = upscale(sheet, k)
        write_png(os.path.join(PREVIEW, 'kodiak_ships_preview.png'), sheet_w * k, h * k, big)

        sw, sh, spx = sawblade(*ATTACK['Red'])
        hw, hh, hpx = harpoon(*ATTACK['Red'])
        row_h = max(sh, hh, rh)
        sheet_w = sw + 2 + hw + 2 + rw
        sheet = [[(40, 40, 40, 255)] * sheet_w for _ in range(row_h)]
        def blit(px, ox, pw, ph):
            for y in range(ph):
                for x in range(pw):
                    p = px[y][x]
                    if p[3]:
                        a = p[3] / 255.0
                        bg = sheet[y][ox + x]
                        sheet[y][ox + x] = (int(p[0]*a + bg[0]*(1-a)), int(p[1]*a + bg[1]*(1-a)), int(p[2]*a + bg[2]*(1-a)), 255)
        blit(spx, 0, sw, sh)
        blit(hpx, sw + 2, hw, hh)
        blit(rpx, sw + 2 + hw + 2, rw, rh)
        big = upscale(sheet, 4)
        write_png(os.path.join(PREVIEW, 'kodiak_attacks_preview.png'), sheet_w * 4, row_h * 4, big)

    print('done')

if __name__ == '__main__':
    main()
