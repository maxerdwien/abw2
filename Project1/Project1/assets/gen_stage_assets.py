# Source generator for stage assets. Currently: the wind gust sound for the Nome stage.
# Run from Project1/Project1:   python assets/gen_stage_assets.py assets

"""Synthesizes wind.wav: band-passed noise with a slow swell that tracks a gust's
ramp-up / hold / ramp-down. Pure Python (wave/struct/math/random) - no third party deps."""
import struct, wave, math, random, os, sys

ASSETS = sys.argv[1] if len(sys.argv) > 1 else "assets"
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

class Biquad:
    """Simple resonant band-pass (state variable filter)."""
    def __init__(self, f, q):
        self.low = self.band = 0.0
        self.set(f, q)
    def set(self, f, q):
        self.f = 2.0 * math.sin(math.pi * min(f, SR / 2.2) / SR)
        self.q = 1.0 / q
    def run(self, x):
        high = x - self.low - self.q * self.band
        self.band += self.f * high
        self.low += self.f * self.band
        return self.band

def wind(total=8.0, attack=1.0, release=1.5):
    random.seed(7)
    n = int(SR * total)
    out = []
    bp = Biquad(400, 2.0)
    bp2 = Biquad(900, 3.0)
    lfo_phase = 0.0
    for i in range(n):
        t = i / SR
        if t < attack:
            e = t / attack
        elif t > total - release:
            e = (total - t) / release
        else:
            e = 1.0
        e = e * e
        # slow wobble in the filter center frequency gives the howling character
        lfo_phase += 2 * math.pi * 0.35 / SR
        wob = 0.5 + 0.5 * math.sin(lfo_phase)
        bp.set(300 + 500 * wob * e, 2.0)
        bp2.set(700 + 900 * wob, 4.0)
        x = random.uniform(-1, 1)
        s = bp.run(x) * 1.2 + bp2.run(x) * 0.35 * wob
        out.append(s * e)
    return out

def main():
    sounds = os.path.join(ASSETS, 'sounds')
    write_wav(os.path.join(sounds, 'wind.wav'), wind())
    print('wrote', os.path.join(sounds, 'wind.wav'))

if __name__ == '__main__':
    main()
