#!/bin/sh
# Packs the build into dist/lander-<target>.tar.gz (.zip for Windows) for
# av: the binary at the top beside its man page, README.md, LICENSE.md and
# PATCHES.md.
#
#   asciiverse/package.sh x86_64-unknown-linux-musl
set -eu
cd "$(dirname "$0")/.."
target="$1"
case "$target" in *windows*) exe=.exe ;; *) exe= ;; esac
stage=stage/lander
rm -rf "$stage" && mkdir -p "$stage" dist
cp "out/lander$exe" lander.6 README.md LICENSE.md PATCHES.md "$stage/"
if [ -n "$exe" ]; then
  (cd "$stage" && zip -qr "../../dist/lander-$target.zip" .)
else
  tar -czf "dist/lander-$target.tar.gz" -C "$stage" .
fi
ls dist
