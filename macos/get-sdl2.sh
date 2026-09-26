#!/bin/sh
#
# Fetch the official universal (arm64 + x86_64) SDL2.framework and drop it in
# macos/Frameworks/, which is where the Makefile and the .app build look for
# it.  Nothing else to install: the Xcode Command Line Tools are enough.
#
#   SDL2_VERSION=2.32.10 sh macos/get-sdl2.sh
#
# Set USE_SYSTEM_SDL2=1 to skip this entirely and build against the SDL2 you
# already have (Homebrew, MacPorts, ...).
#
set -e

VERSION="${SDL2_VERSION:-2.32.10}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DEST="$ROOT/macos/Frameworks"

if [ "$(uname -s)" != "Darwin" ]; then
	echo "get-sdl2.sh: not macOS, nothing to do."
	exit 0
fi

if [ -n "${USE_SYSTEM_SDL2:-}" ]; then
	echo "USE_SYSTEM_SDL2 is set - not downloading SDL2."
	exit 0
fi

if [ -d "$DEST/SDL2.framework" ]; then
	echo "SDL2.framework is already in macos/Frameworks."
	exit 0
fi

TMP="$(mktemp -d)"

# Every step here is best-effort: a failure in an EXIT trap under `set -e`
# would otherwise make a perfectly successful download exit non-zero.
cleanup() {
	hdiutil detach -quiet "$TMP/mnt" >/dev/null 2>&1 || :
	rm -rf "$TMP" || :
}
trap cleanup EXIT

URL="https://github.com/libsdl-org/SDL/releases/download/release-$VERSION/SDL2-$VERSION.dmg"
echo "Downloading SDL2 $VERSION ..."
curl -# -f -L -o "$TMP/SDL2.dmg" "$URL"

mkdir -p "$TMP/mnt" "$DEST"
hdiutil attach -quiet -nobrowse -readonly -mountpoint "$TMP/mnt" "$TMP/SDL2.dmg"
cp -R "$TMP/mnt/SDL2.framework" "$DEST/"
hdiutil detach -quiet "$TMP/mnt" || :

# The download flag would make every launch ask Gatekeeper about it.
xattr -dr com.apple.quarantine "$DEST/SDL2.framework" 2>/dev/null || :

echo "SDL2 $VERSION installed in macos/Frameworks/SDL2.framework"
exit 0
