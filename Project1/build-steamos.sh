#!/usr/bin/env bash
# Builds the Linux version of the game on SteamOS (or any Linux with podman).
#
# SteamOS has no compiler and a read-only system partition, so the build runs
# inside a throwaway Ubuntu 24.04 container. The SDL2 add-on libraries that
# SteamOS doesn't ship are copied into ./linux/lib, which the binary finds via
# its rpath ($ORIGIN/lib).
#
# Usage: ./build-steamos.sh          (build)
#        ./build-steamos.sh clean    (remove build output)
set -euo pipefail
cd "$(dirname "$0")"

if [ "${1:-}" = "clean" ]; then
	rm -rf build linux
	exit 0
fi

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

podman run --rm -v "$PWD":/w:Z -w /w "$IMAGE" bash -euo pipefail -c "
	apt-get update -qq >/dev/null
	DEBIAN_FRONTEND=noninteractive apt-get install -y -qq \
		g++ make libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev >/dev/null
	make -j\$(nproc)
	mkdir -p linux/lib
	for l in ${BUNDLED_LIBS[*]}; do
		cp -L /usr/lib/x86_64-linux-gnu/\$l linux/lib/
	done
"

missing=$(ldd ./linux/acsf | grep 'not found' || true)
if [ -n "$missing" ]; then
	echo "warning: some libraries could not be found on this system:" >&2
	echo "$missing" >&2
	exit 1
fi
echo "built $(pwd)/linux/acsf"
