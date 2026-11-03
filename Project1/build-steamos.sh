#!/usr/bin/env bash
# Builds the Linux version of the game on SteamOS (or any Linux with podman).
#
# SteamOS has no compiler and a read-only system partition, so the build runs
# inside a throwaway Ubuntu 24.04 container. The SDL2 add-on libraries that
# SteamOS doesn't ship are copied into ./linux/lib, which the binary finds via
# its rpath ($ORIGIN/lib). Steam library artwork is generated into
# ./linux/steam-art from the game's own sprites.
#
# Usage: ./build-steamos.sh                  build
#        ./build-steamos.sh install [DIR]    build, then install to DIR
#                                            (default ~/Games/acsf) and give any
#                                            Steam shortcut to it library artwork
#        ./build-steamos.sh clean            remove build output
set -euo pipefail
cd "$(dirname "$0")"

INSTALL_DIR_DEFAULT="$HOME/Games/acsf"
IMAGE=docker.io/library/ubuntu:24.04

# libraries that SteamOS does not provide (SDL2 itself comes from the system)
BUNDLED_LIBS=(
	libSDL2_image-2.0.so.0
	libSDL2_ttf-2.0.so.0
	libSDL2_mixer-2.0.so.0
	libFLAC.so.12
	libfluidsynth.so.3
	libinstpatch-1.0.so.2
	libopusfile.so.0
)

build() {
	podman run --rm -v "$PWD":/w:Z -w /w "$IMAGE" bash -euo pipefail -c "
		apt-get update -qq >/dev/null
		DEBIAN_FRONTEND=noninteractive apt-get install -y -qq \
			g++ make libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev python3-pil >/dev/null
		make -j\$(nproc)
		mkdir -p linux/lib
		for l in ${BUNDLED_LIBS[*]}; do
			cp -L /usr/lib/x86_64-linux-gnu/\$l linux/lib/
		done
		python3 steam-art/make_steam_art.py Project1/assets linux/steam-art
	"

	local missing
	missing=$(ldd ./linux/acsf | grep 'not found' || true)
	if [ -n "$missing" ]; then
		echo "warning: some libraries could not be found on this system:" >&2
		echo "$missing" >&2
		exit 1
	fi
	echo "built $(pwd)/linux/acsf"
}

# Installs the game with the same layout as the repo, so the executable's
# "../Project1/assets" paths resolve:
#   DIR/ACSF/acsf, DIR/ACSF/lib/, DIR/Project1/assets/
install_game() {
	local dir="$1"
	if pgrep -xf "$dir/ACSF/acsf" >/dev/null; then
		echo "error: the game is running from $dir; quit it first" >&2
		exit 1
	fi
	mkdir -p "$dir/ACSF/lib" "$dir/Project1/assets"
	if [ -f "$dir/ACSF/acsf" ] && ! cmp -s linux/acsf "$dir/ACSF/acsf"; then
		cp -a "$dir/ACSF/acsf" "$dir/ACSF/acsf.previous"
	fi
	cp linux/acsf "$dir/ACSF/acsf"
	cp linux/lib/* "$dir/ACSF/lib/"
	cp -a Project1/assets/. "$dir/Project1/assets/"
	cp Project1/max-sdl-controller-mappings.txt "$dir/Project1/"
	echo "installed to $dir (run $dir/ACSF/acsf)"
	install_steam_art "$dir/ACSF/acsf"
}

# Copies the artwork into Steam's grid folder for every non-Steam shortcut that
# launches EXE. Steam names these files after the shortcut's app id.
install_steam_art() {
	local exe="$1"
	local found=0
	for vdf in "$HOME"/.local/share/Steam/userdata/*/config/shortcuts.vdf; do
		[ -f "$vdf" ] || continue
		local grid
		grid="$(dirname "$vdf")/grid"
		for id in $(python3 - "$vdf" "$exe" <<'EOF'
import struct, sys
data = open(sys.argv[1], "rb").read()
want = sys.argv[2]

def parse(i):
	out = {}
	while True:
		t = data[i]; i += 1
		if t == 8:
			return out, i
		j = data.index(b"\0", i); key = data[i:j].decode("utf-8", "replace"); i = j + 1
		if t == 0:
			val, i = parse(i)
		elif t == 1:
			j = data.index(b"\0", i); val = data[i:j].decode("utf-8", "replace"); i = j + 1
		elif t == 2:
			val = struct.unpack("<I", data[i:i + 4])[0]; i += 4
		else:
			raise SystemExit(f"unsupported vdf field type {t}")
		out[key] = val

shortcuts = parse(0)[0].get("shortcuts", {})
for s in shortcuts.values():
	s = {k.lower(): v for k, v in s.items()}
	if s.get("exe", "").strip('"') == want and "appid" in s:
		print(s["appid"])
EOF
		); do
			mkdir -p "$grid"
			cp linux/steam-art/capsule.png "$grid/${id}p.png"
			cp linux/steam-art/wide.png "$grid/${id}.png"
			cp linux/steam-art/hero.png "$grid/${id}_hero.png"
			cp linux/steam-art/logo.png "$grid/${id}_logo.png"
			echo "installed Steam artwork for shortcut $id"
			found=1
		done
	done
	if [ "$found" = 0 ]; then
		echo "no Steam shortcut launches $exe yet; add it as a non-Steam game, then run install again to add its artwork"
	fi
}

case "${1:-}" in
	clean)
		rm -rf build linux
		;;
	install)
		build
		install_game "${2:-$INSTALL_DIR_DEFAULT}"
		;;
	"")
		build
		;;
	*)
		echo "usage: $0 [install [DIR] | clean]" >&2
		exit 2
		;;
esac
