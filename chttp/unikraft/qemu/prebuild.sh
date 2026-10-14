#!/usr/bin/env bash
# Build the chttp binary into the build context, where the bunnyfile picks it
# up. The unikraft base kernel runs Linux ELF binaries, so build a static PIE
# for the native architecture. The compiler image is pinned by digest so the
# binary is reproducible.
set -euo pipefail

GCC_IMAGE=gcc:15.2.0@sha256:3ae15afe768b06d0c0fe088d822ba5f8045c26630bdacc8d8e7713cf5d8e7289

cd "$(dirname "$0")"
docker run --rm -u "$(id -u):$(id -g)" -v "$PWD:/src" -w /src "$GCC_IMAGE" \
	gcc -O2 -Wall -fPIE -static-pie -o chttp chttp.c
