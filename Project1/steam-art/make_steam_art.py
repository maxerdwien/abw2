#!/usr/bin/env python3
"""Generates Steam library artwork for the game from its own sprites.

Usage: make_steam_art.py <assets dir> <output dir>

Writes the four images Steam's library uses for a (non-Steam) shortcut:
  capsule.png  600x900   portrait capsule (library grid)
  wide.png     920x430   wide capsule (recent games / home row)
  hero.png     3840x1240 banner behind the game page header
  logo.png     transparent title drawn over the hero

Needs Pillow. Output is deterministic (fixed random seeds).
"""
import math
import os
import random
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont

ASSETS = sys.argv[1]
OUT = sys.argv[2]

SHIPS = ["kodiak", "grizzly", "polar", "panda", "black"]

# sky colors
SKY_TOP = (4, 6, 16)
SKY_BOTTOM = (12, 24, 48)
AURORA_GREEN = (60, 255, 160)
AURORA_TEAL = (40, 200, 255)
AURORA_VIOLET = (150, 90, 255)
ICE = (170, 220, 255)

# glow color per player color, used for engine light and shots
PLAYER_GLOW = {
	"red": (255, 70, 60),
	"blue": (70, 120, 255),
	"yellow": (255, 220, 60),
	"green": (80, 255, 110),
	"teal": (60, 230, 230),
	"magenta": (255, 70, 230),
}


def asset(path):
	return Image.open(os.path.join(ASSETS, path)).convert("RGBA")


def lerp(a, b, t):
	return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(len(a)))


def font(size):
	return ImageFont.truetype(os.path.join(ASSETS, "caladea-regular.ttf"), size)


def sky(w, h, seed, aurora_strength=1.0, aurora_height=0.55):
	"""Gradient sky with aurora curtains and pixel stars."""
	rnd = random.Random(seed)
	img = Image.new("RGBA", (w, h))
	d = ImageDraw.Draw(img)
	for y in range(h):
		d.line([(0, y), (w, y)], fill=lerp(SKY_TOP, SKY_BOTTOM, y / max(1, h - 1)) + (255,))

	# aurora: drawn at low resolution as vertical curtains, then blurred and scaled up
	s = 6
	lw, lh = max(1, w // s), max(1, h // s)
	aur = Image.new("RGB", (lw, lh), (0, 0, 0))
	ad = ImageDraw.Draw(aur)
	for band in range(3):
		phase = rnd.uniform(0, math.tau)
		freq = rnd.uniform(1.2, 2.4) / lw * math.tau
		base = lh * aurora_height * rnd.uniform(0.55, 0.9)
		amp = lh * rnd.uniform(0.08, 0.18)
		length = lh * rnd.uniform(0.25, 0.45)
		color = [AURORA_GREEN, AURORA_TEAL, AURORA_GREEN][band]
		# each curtain fades in and out along its length so it doesn't read as a stripe
		env_freq = rnd.uniform(0.6, 1.3) / lw * math.tau
		env_phase = rnd.uniform(0, math.tau)
		for x in range(lw):
			bottom = base + amp * math.sin(x * freq + phase) + 0.4 * amp * math.sin(x * freq * 3.1 + phase * 2)
			# curtains flicker in brightness along their length
			bright = 0.55 + 0.45 * math.sin(x * freq * 5.3 + phase * 3) ** 2
			bright *= max(0.0, math.sin(x * env_freq + env_phase)) ** 1.5
			top = bottom - length * (0.7 + 0.3 * bright)
			for y in range(max(0, int(top)), min(lh, int(bottom) + 1)):
				t = min(1.0, max(0.0, (bottom - y) / max(1.0, bottom - top)))  # 0 at the bright lower edge, 1 at the top
				c = lerp(color, AURORA_VIOLET, min(1.0, t * 1.3))
				a = (1 - t) ** 1.6 * bright * 0.55 * aurora_strength
				old = aur.getpixel((x, y))
				aur.putpixel((x, y), tuple(min(255, int(old[i] + c[i] * a)) for i in range(3)))
	aur = aur.filter(ImageFilter.GaussianBlur(2.2)).resize((w, h), Image.BICUBIC)
	img = Image.merge("RGBA", (*ImageChops.add(img.convert("RGB"), aur).split(), Image.new("L", (w, h), 255)))

	# pixel stars, like the in-game background, sized to the image
	d = ImageDraw.Draw(img)
	px = max(2, round(min(w, h) / 300))
	for _ in range(int(w * h / 5500)):
		x, y = rnd.randrange(w), rnd.randrange(h)
		size = px * rnd.choice([1, 1, 1, 1, 2])
		v = rnd.randint(150, 255)
		tint = rnd.choice([(v, v, v), (v, v, v), (int(v * 0.8), int(v * 0.9), v)])
		d.rectangle([x, y, x + size - 1, y + size - 1], fill=tint + (255,))
	# a few bright twinkles
	for _ in range(max(3, int(w * h / 400000))):
		x, y = rnd.randrange(w), rnd.randrange(h)
		r = px * rnd.randint(3, 6)
		d.rectangle([x - r, y - px // 2, x + r, y + px // 2], fill=(200, 230, 255, 255))
		d.rectangle([x - px // 2, y - r, x + px // 2, y + r], fill=(200, 230, 255, 255))
		d.rectangle([x - px, y - px, x + px, y + px], fill=(255, 255, 255, 255))
	return img


def alaska(img, cx, cy, width, alpha=0.18):
	"""Faint icy outline of the stage-select map of Alaska."""
	src = asset("alaska.png")
	scale = width / src.width
	src = src.resize((int(src.width * scale), int(src.height * scale)), Image.LANCZOS)
	mask = src.getchannel("A")
	edge = mask.filter(ImageFilter.FIND_EDGES).filter(ImageFilter.MaxFilter(3))
	fill = Image.new("RGBA", src.size, ICE + (0,))
	fill.putalpha(mask.point(lambda v: int(v * alpha * 0.35)))
	line = Image.new("RGBA", src.size, ICE + (0,))
	line.putalpha(edge.point(lambda v: int(v * alpha * 2.2)))
	glow = line.filter(ImageFilter.GaussianBlur(max(2, width // 250)))
	for layer in (fill, glow, line):
		img.alpha_composite(layer, (int(cx - src.width / 2), int(cy - src.height / 2)))


def glow_dot(img, cx, cy, radius, color, strength=1.0):
	layer = Image.new("RGBA", img.size, color + (0,))
	m = Image.new("L", img.size, 0)
	ImageDraw.Draw(m).ellipse([cx - radius, cy - radius, cx + radius, cy + radius], fill=int(255 * strength))
	layer.putalpha(m.filter(ImageFilter.GaussianBlur(radius * 0.6)))
	img.alpha_composite(layer)


def ship(img, kind, color, cx, cy, scale, angle):
	"""Draws a ship sprite, pixel-crisp, rotated so `angle` (degrees, 0 = up,
	clockwise) is its heading, with an engine flame and colored underglow."""
	spr = asset(f"ships/{kind}-{color}.png")
	spr = spr.resize((spr.width * scale, spr.height * scale), Image.NEAREST)

	rad = math.radians(angle)
	fx, fy = math.sin(rad), -math.cos(rad)  # forward
	back = spr.height * 0.5

	# engine flame: hot core plus colored plume trailing behind the ship
	for i, (dist, r, c, s) in enumerate([
		(back + scale * 9, scale * 7, PLAYER_GLOW[color], 0.55),
		(back + scale * 4, scale * 5, (255, 170, 60), 0.9),
		(back + scale * 1, scale * 3, (255, 245, 200), 1.0),
	]):
		glow_dot(img, cx - fx * dist, cy - fy * dist, r, c, s)
	glow_dot(img, cx, cy, spr.width * 0.8, PLAYER_GLOW[color], 0.18)

	rot = spr.rotate(-angle, resample=Image.NEAREST, expand=True)
	img.alpha_composite(rot, (int(cx - rot.width / 2), int(cy - rot.height / 2)))


def shot(img, kind, color, x, y, scale, angle, trail):
	"""A projectile sprite with a motion streak behind it."""
	spr = asset(f"attacks/{kind}{color.capitalize()}.png")
	spr = spr.resize((spr.width * scale, spr.height * scale), Image.NEAREST)
	rad = math.radians(angle)
	fx, fy = math.sin(rad), -math.cos(rad)
	g = PLAYER_GLOW[color]
	streak = Image.new("RGBA", img.size, g + (0,))
	m = Image.new("L", img.size, 0)
	md = ImageDraw.Draw(m)
	steps = 12
	for i in range(steps):
		t0, t1 = i / steps, (i + 1) / steps
		md.line([(x - fx * trail * t0, y - fy * trail * t0), (x - fx * trail * t1, y - fy * trail * t1)],
			fill=int(200 * (1 - t0)), width=max(2, int(scale * 2.5 * (1 - t0))))
	streak.putalpha(m.filter(ImageFilter.GaussianBlur(scale)))
	img.alpha_composite(streak)
	rot = spr.rotate(-angle, resample=Image.NEAREST, expand=True)
	img.alpha_composite(rot, (int(x - rot.width / 2), int(y - rot.height / 2)))


def title(lines, size, spacing=0.18, line_gap=0.12, align="center"):
	"""Title text: white letters with an icy glow and a dark outline, on transparency."""
	f = [font(int(size * s)) for _, s in lines]
	widths, heights = [], []
	for (text, _), fnt in zip(lines, f):
		track = fnt.size * spacing
		w = sum(fnt.getlength(ch) for ch in text) + track * (len(text) - 1)
		widths.append(w)
		heights.append(fnt.size)
	pad = int(size * 0.5)
	W = int(max(widths)) + 2 * pad
	H = int(sum(heights) + line_gap * size * (len(lines) - 1)) + 2 * pad
	text_mask = Image.new("L", (W, H), 0)
	d = ImageDraw.Draw(text_mask)
	y = pad
	line_spans = []
	for (text, _), fnt, w, h in zip(lines, f, widths, heights):
		line_spans.append((y, h))
		x = pad + (0 if align == "left" else (W - 2 * pad - w) / 2)
		track = fnt.size * spacing
		for ch in text:
			d.text((x, y), ch, font=fnt, fill=255, anchor="lt")
			x += fnt.getlength(ch) + track
		y += h + line_gap * size

	out = Image.new("RGBA", (W, H), (0, 0, 0, 0))
	outline = text_mask.filter(ImageFilter.MaxFilter(2 * max(1, size // 40) + 1))
	for blur, color, a in [(size * 0.35, AURORA_TEAL, 0.55), (size * 0.12, ICE, 0.8)]:
		layer = Image.new("RGBA", (W, H), color + (0,))
		layer.putalpha(text_mask.filter(ImageFilter.GaussianBlur(blur)).point(lambda v: min(255, int(v * a * 1.6))))
		out.alpha_composite(layer)
	dark = Image.new("RGBA", (W, H), (5, 10, 25, 0))
	dark.putalpha(outline)
	out.alpha_composite(dark)
	# letters: white with a faint icy gradient toward the bottom of each line
	grad = Image.new("RGBA", (W, H))
	gd = ImageDraw.Draw(grad)
	for ly, lh in line_spans:
		for yy in range(int(ly), int(ly + lh * 1.3) + 1):
			t = min(1.0, max(0.0, (yy - ly) / (lh * 1.1)))
			gd.line([(0, yy), (W, yy)], fill=lerp((255, 255, 255), (175, 225, 255), t) + (255,))
	grad.putalpha(text_mask)
	out.alpha_composite(grad)
	return out.crop(out.getbbox())


def paste_center(img, layer, cx, cy):
	img.alpha_composite(layer, (int(cx - layer.width / 2), int(cy - layer.height / 2)))


def vignette(img, strength=0.55):
	w, h = img.size
	m = Image.new("L", (w, h), 0)
	ImageDraw.Draw(m).ellipse([-w * 0.25, -h * 0.25, w * 1.25, h * 1.25], fill=255)
	m = m.filter(ImageFilter.GaussianBlur(min(w, h) * 0.2))
	dark = Image.new("RGBA", (w, h), (0, 0, 8, 0))
	dark.putalpha(m.point(lambda v: int((255 - v) * strength)))
	img.alpha_composite(dark)


TITLE = [("ALASKAN", 1.0), ("COSMOBEAR", 1.0), ("SPACEFIGHTING", 0.62)]


def make_hero():
	w, h = 3840, 1240
	img = sky(w, h, seed=1, aurora_height=0.6)
	alaska(img, w * 0.68, h * 0.55, w * 0.42, alpha=0.2)
	s = 12
	# dogfight across the right two thirds; the left is kept calm for Steam's logo
	shot(img, "bullet", "blue", w * 0.54, h * 0.30, s, 70, s * 60)
	shot(img, "missile", "red", w * 0.86, h * 0.70, s, -110, s * 70)
	shot(img, "sawblade", "teal", w * 0.70, h * 0.83, s, 20, s * 40)
	shot(img, "harpoon", "yellow", w * 0.94, h * 0.27, s, -60, s * 50)
	ship(img, "kodiak", "red", w * 0.72, h * 0.47, 16, 35)
	ship(img, "polar", "blue", w * 0.48, h * 0.62, 13, 60)
	ship(img, "grizzly", "teal", w * 0.88, h * 0.32, 11, -130)
	ship(img, "panda", "yellow", w * 0.94, h * 0.78, 10, -45)
	ship(img, "black", "magenta", w * 0.60, h * 0.18, 9, 100)
	vignette(img, 0.45)
	return img


def make_capsule():
	w, h = 600, 900
	img = sky(w, h, seed=2, aurora_height=0.35, aurora_strength=1.1)
	alaska(img, w * 0.5, h * 0.74, w * 1.05, alpha=0.22)
	shot(img, "missile", "red", w * 0.20, h * 0.86, 4, 30, 110)
	shot(img, "bullet", "teal", w * 0.86, h * 0.47, 4, -20, 90)
	ship(img, "polar", "blue", w * 0.20, h * 0.66, 5, 25)
	ship(img, "grizzly", "teal", w * 0.84, h * 0.88, 5, -35)
	ship(img, "kodiak", "red", w * 0.53, h * 0.62, 9, 12)
	vignette(img, 0.5)
	t = title(TITLE, 92, spacing=0.06)
	t = t.resize((int(t.width * (w * 0.9) / t.width), int(t.height * (w * 0.9) / t.width)), Image.LANCZOS)
	paste_center(img, t, w / 2, h * 0.2)
	return img


def make_wide():
	w, h = 920, 430
	img = sky(w, h, seed=3, aurora_height=0.5)
	alaska(img, w * 0.74, h * 0.55, w * 0.5, alpha=0.22)
	shot(img, "missile", "blue", w * 0.92, h * 0.85, 4, -60, 90)
	ship(img, "kodiak", "red", w * 0.75, h * 0.48, 7, 40)
	ship(img, "panda", "blue", w * 0.92, h * 0.25, 4, -120)
	vignette(img, 0.45)
	t = title(TITLE, 80, spacing=0.05)
	scale = min((w * 0.55) / t.width, (h * 0.8) / t.height)
	t = t.resize((int(t.width * scale), int(t.height * scale)), Image.LANCZOS)
	paste_center(img, t, w * 0.31, h * 0.5)
	return img


def make_logo():
	return title(TITLE, 240, spacing=0.06)


os.makedirs(OUT, exist_ok=True)
for name, fn in [("hero", make_hero), ("capsule", make_capsule), ("wide", make_wide), ("logo", make_logo)]:
	im = fn()
	if name != "logo":
		im = im.convert("RGB")
	im.save(os.path.join(OUT, f"{name}.png"), optimize=True)
	print(f"{name}.png {im.width}x{im.height}")
