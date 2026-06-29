#!/usr/bin/env bash
# Build inside the ps2dev toolchain image (+ make). No host install required.
#
#   ./build.sh          # build smpte-bars.elf
#   ./build.sh clean    # remove build artefacts
#
# The image is built once from the local Dockerfile and cached afterwards.
set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IMAGE="ps2-smpte-bars-build"

if ! docker image inspect "$IMAGE" >/dev/null 2>&1; then
    echo ">> Building toolchain image '$IMAGE' (first run only)..."
    docker build -t "$IMAGE" "$DIR"
fi

exec docker run --rm \
    -v "$DIR":/src \
    -w /src \
    -u "$(id -u):$(id -g)" \
    "$IMAGE" \
    make "$@"
