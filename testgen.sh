#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./filepoke.sh gen 500 2000 test.bin   -> generate random file 500–2000 bytes
#   ./filepoke.sh poke test.bin poked.bin -> apply random edits

cmd="${1:-}"

# Generate random file
if [[ "$cmd" == "gen" ]]; then
    min="${2}"
    max="${3}"
    out="${4}"
    len=$((RANDOM % (max - min + 1) + min))
    dd if=/dev/urandom of="$out" bs=1 count="$len" status=none
    printf "generated %s (%d bytes)\n" "$out" "$len"
    exit 0
fi

# Poke (mutate) a file
if [[ "$cmd" == "poke" ]]; then
    src="${2}"
    out="${3}"
    data=$(<"$src")
    size=${#data}

    # 1. Remove a random segment (5–20% of file)
    rlen=$((RANDOM % (size/5) + 1))  # up to 20%
    rlen=$((rlen < 5 ? 5 : rlen))    # minimum 5 bytes
    rlen=$((rlen > size ? size/10 : rlen))
    rpos=$((RANDOM % (size - rlen + 1)))
    data="${data:0:rpos}${data:rpos+rlen}"

    # 2. Insert 1–3 random chunks
    for _ in $(seq 1 $((RANDOM % 3 + 1))); do
        ilen=$((RANDOM % 32 + 1)) # tiny chunks ≤32 bytes
        ipos=$((RANDOM % (size + 1)))
        chunk=$(dd if=/dev/urandom bs=1 count="$ilen" 2>/dev/null)
        data="${data:0:ipos}${chunk}${data:ipos}"
        size=${#data}
    done

    # 3. Move a small region (4–16 bytes)
    mlen=$((RANDOM % 12 + 4))
    mpos=$((RANDOM % (size - mlen + 1)))
    region="${data:mpos:mlen}"
    data="${data:0:mpos}${data:mpos+mlen}"
    size=${#data}
    newpos=$((RANDOM % (size + 1)))
    data="${data:0:newpos}${region}${data:newpos}"

    printf "%s" "$data" > "$out"
    printf "poked %s -> %s (%d bytes)\n" "$src" "$out" "${#data}"
    exit 0
fi

cat <<EOF
Commands:
  gen  <min> <max> <outfile>
  poke <src> <outfile>
EOF
exit 1
