#!/bin/sh
#
# Build BlackShades.app.
#
# Builds for this machine's architecture (arm64 on Apple Silicon) and bundles
# SDL2, so the .app can be moved or handed to someone else as is.
#
#   sh macos/build-app.sh                   # native arch, bundled SDL2.framework
#   UNIVERSAL=1 sh macos/build-app.sh       # arm64 + x86_64 fat binary
#   ARCHS="arm64" sh macos/build-app.sh     # pick the slices yourself
#   USE_SYSTEM_SDL2=1 sh macos/build-app.sh # your own SDL2 (Homebrew etc.)
#   USE_SYSTEM_OPENAL=1 ...                 # OpenAL.framework instead of MiniAL
#
# With USE_SYSTEM_SDL2 the build is native-only (a Homebrew dylib is one
# architecture), and the SDL2 dylib it linked against is copied into the bundle
# and re-pointed with install_name_tool, so the .app still stands on its own.
#
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

APP="BlackShades.app"
FRAMEWORK="macos/Frameworks/SDL2.framework"

if [ -z "${USE_SYSTEM_SDL2:-}" ]; then
	sh macos/get-sdl2.sh
fi

# Which architectures to build.  Native first, always: this machine is what
# the result has to run on.
NATIVE="$(uname -m)"

if [ -n "${ARCHS:-}" ]; then
	: # caller picked
elif [ -n "${UNIVERSAL:-}" ] && [ -z "${USE_SYSTEM_SDL2:-}" ] && [ -d "$FRAMEWORK" ]; then
	# fat build, but only with slices the bundled SDL2 actually has
	ARCHS="$NATIVE"
	for a in $(lipo -archs "$FRAMEWORK/Versions/A/SDL2" 2>/dev/null || echo ""); do
		case "$a" in
			"$NATIVE") ;;
			arm64|x86_64) ARCHS="$ARCHS $a" ;;
		esac
	done
else
	ARCHS="$NATIVE"
fi

echo "Target architecture(s):$(printf ' %s' $ARCHS)"

if [ -n "${UNIVERSAL:-}" ] && [ -n "${USE_SYSTEM_SDL2:-}" ]; then
	echo "note: UNIVERSAL with USE_SYSTEM_SDL2 only works if that SDL2 is itself universal."
fi

SLICES=""
for arch in $ARCHS; do
	case "$arch" in
		arm64)  MINVER=11.0  ;;
		x86_64) MINVER=10.13 ;;
		*)      MINVER=11.0  ;;
	esac
	echo "==> building $arch (macOS $MINVER+)"
	make OBJDIR="build-$arch" EXE="blackshades-$arch" \
	     ARCHFLAGS="-arch $arch -mmacosx-version-min=$MINVER" \
	     ${USE_SYSTEM_SDL2:+USE_SYSTEM_SDL2=1} \
	     ${USE_SYSTEM_OPENAL:+USE_SYSTEM_OPENAL=1}
	SLICES="$SLICES blackshades-$arch"
done

echo "==> assembling $APP"
rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources" "$APP/Contents/Frameworks"

BIN="$APP/Contents/MacOS/BlackShades"
if [ "$(echo $SLICES | wc -w)" -gt 1 ]; then
	# shellcheck disable=SC2086
	lipo -create $SLICES -output "$BIN"
else
	cp $SLICES "$BIN"
fi

cp macos/Info.plist "$APP/Contents/Info.plist"
cp macos/BlackShades.icns "$APP/Contents/Resources/BlackShades.icns"
printf 'APPL????' > "$APP/Contents/PkgInfo"

# The game's data, read from Contents/Resources at runtime (SDL_GetBasePath).
cp -R Data "$APP/Contents/Resources/Data"

if [ -z "${USE_SYSTEM_SDL2:-}" ] && [ -d "$FRAMEWORK" ]; then
	cp -R "$FRAMEWORK" "$APP/Contents/Frameworks/"
else
	# Carry the SDL2 dylib we linked against into the bundle so the app does
	# not depend on the exact Homebrew/MacPorts prefix it was built with.
	DYLIB="$(otool -L "$BIN" | awk '/libSDL2[^ ]*\.dylib/ {print $1; exit}')"
	if [ -n "$DYLIB" ] && [ -f "$DYLIB" ]; then
		BASE="$(basename "$DYLIB")"
		cp "$DYLIB" "$APP/Contents/Frameworks/$BASE"
		chmod u+w "$APP/Contents/Frameworks/$BASE"
		install_name_tool -id "@executable_path/../Frameworks/$BASE" \
		                  "$APP/Contents/Frameworks/$BASE"
		install_name_tool -change "$DYLIB" "@executable_path/../Frameworks/$BASE" "$BIN"
		echo "    bundled $BASE"

		# Homebrew's "sdl2" is sdl2-compat these days: a shim that dlopens
		# SDL3 at run time (it tries @loader_path/libSDL3.dylib, i.e. a
		# sibling file).  Its own rpath points back into the Homebrew prefix,
		# which stops resolving the moment we copy it in here, so SDL3 has to
		# travel with it - otherwise the app dies at startup with
		# "Failed loading SDL3 library".
		if LC_ALL=C grep -qa "sdl2-compat" "$DYLIB" 2>/dev/null; then
			LIBDIR="$(cd "$(dirname "$DYLIB")" && pwd)"
			CANDIDATES="$LIBDIR $LIBDIR/../../../../opt/sdl3/lib"
			for rp in $(otool -l "$DYLIB" | awk '$1=="path"{print $2}'); do
				case "$rp" in
					@loader_path*)     CANDIDATES="$CANDIDATES $LIBDIR${rp#@loader_path}" ;;
					@executable_path*) CANDIDATES="$CANDIDATES $LIBDIR${rp#@executable_path}" ;;
					@*)                ;;
					*)                 CANDIDATES="$CANDIDATES $rp" ;;
				esac
			done

			SDL3=""
			for dir in $CANDIDATES; do
				for name in libSDL3.dylib libSDL3.0.dylib; do
					if [ -z "$SDL3" ] && [ -f "$dir/$name" ]; then
						SDL3="$dir/$name"
					fi
				done
			done

			if [ -n "$SDL3" ]; then
				cp "$SDL3" "$APP/Contents/Frameworks/libSDL3.dylib"
				chmod u+w "$APP/Contents/Frameworks/libSDL3.dylib"
				install_name_tool -id "@loader_path/libSDL3.dylib" \
				                  "$APP/Contents/Frameworks/libSDL3.dylib"
				echo "    bundled libSDL3.dylib ($BASE is sdl2-compat and loads SDL3 at run time)"
			else
				echo "    WARNING: $BASE is sdl2-compat but libSDL3 was not found next to it."
				echo "             The app would stop with 'Failed loading SDL3 library'."
				echo "             Either 'brew install sdl3', or build without USE_SYSTEM_SDL2=1"
				echo "             to bundle the official SDL2 framework instead."
			fi
		fi
	elif [ -n "$DYLIB" ]; then
		echo "    note: linked against $DYLIB, which is not a file to copy - leaving as is"
	fi
fi

# Anything bundled that still points outside the app is worth knowing about.
for lib in "$APP"/Contents/Frameworks/*.dylib; do
	[ -f "$lib" ] || continue
	otool -L "$lib" | tail -n +2 | awk '{print $1}' |
		grep -v '^/usr/lib/\|^/System/\|@executable_path\|@rpath\|@loader_path' |
		while read -r dep; do
			echo "    note: $(basename "$lib") also needs $dep (not bundled)"
		done
done

# lipo and install_name_tool invalidate the linker's ad-hoc signature, and
# arm64 refuses to run unsigned code, so sign the libraries and then the app.
if [ -d "$APP/Contents/Frameworks/SDL2.framework" ]; then
	codesign --force --sign - "$APP/Contents/Frameworks/SDL2.framework" >/dev/null
fi
for lib in "$APP"/Contents/Frameworks/*.dylib; do
	if [ -f "$lib" ]; then
		codesign --force --sign - "$lib" >/dev/null
	fi
done
codesign --force --sign - "$APP" >/dev/null

rm -f $SLICES

echo
echo "Built $APP"
lipo -archs "$BIN" | sed 's/^/  architecture(s): /'
otool -L "$BIN" | awk '/libSDL2|SDL2.framework/ {print "  links SDL2: "$1}'
echo "  open it with:  open $APP"
echo "  or run it in a terminal (to see its output):  $APP/Contents/MacOS/BlackShades"
