#!/usr/bin/env bash
set -euo pipefail

########################################################
#                                                      #
#                   Compile Flags                      #
#                                                      #
########################################################

MacroSwitches=(
    -DASSERT=1
    -DPROFILE=1
    -DARENA_DEBUG=1
    -DVULKAN_DEBUG=1
    -DWAYLAND_DEBUG=1
)

CommonFlags=(
    -Wall
    -Wextra
    -pedantic
    -Wno-unused-function
    -Wno-missing-braces
    -Wno-initializer-overrides
    -fuse-ld=lld
    -mavx2
    -mfma
    -fno-exceptions
    -fno-trapping-math
    -fno-math-errno
    -pthread
)

DebugFlags=(
    -g3
    -fsanitize=address
)

ReleaseFlags=(
    -O2
    -ffinite-math-only
    -fno-signed-zeros
    -freciprocal-math
    -fassociative-math
)

Libraries=(
    -lvulkan
)


########################################################
#                                                      #
#                       Script                         #
#                                                      #
########################################################

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 [--release|-r | --debug|-d] <file>"
    exit 1
fi


case "$1" in
    --debug|-d)
        declare -n Flags=DebugFlags
        ;;
    --release|-r)
        declare -n Flags=ReleaseFlags
        ;;
    *)
        echo "Error: Invalid mode '$1'. Use --debug/-d or --release/-r"
        exit 1
        ;;
esac

File="$2"
if [[ ! -f "$File" ]]; then
    echo "Error: File '$File' does not exist or is not accessible"
    exit 1
fi

clang "${MacroSwitches[@]}" "${CommonFlags[@]}" "${Flags[@]}" "${Libraries[@]}" "$File"
