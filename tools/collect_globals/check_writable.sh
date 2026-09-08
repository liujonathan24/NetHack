#!/usr/bin/env bash
# Prove the built library has no mutable process-global state left: list every
# symbol in a writable section (.data/.bss/COMMON/TLS) of the object files
# and fail on any that is not on the whitelist.
#
#   tools/collect_globals/check_writable.sh <build>/CMakeFiles/nethack.dir [whitelist]
#
# Function-local statics show up here too (gcc names them var.N), so a missed
# object cannot hide. Run it after every build that touches the game.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
OBJDIR="$(cd "${1:?object dir}" && pwd)"
WL="${2:-$HERE/whitelist.txt}"
PY="${PYTHON:-python3}"
allowed=$(grep -v '^#' "$WL" | sed 's/[[:space:]].*//' | grep -v '^$' || true)
inv=$(cd "$HERE" && $PY -m nhglobals inventory "$OBJDIR") || { echo "inventory failed" >&2; exit 2; }
[ -n "$inv" ] || { echo "empty inventory for $OBJDIR" >&2; exit 2; }
fail=0; n=0
while IFS=$'\t' read -r sym size sec obj; do
    [ -z "$sym" ] && continue
    base="${sym%%.*}"
    if grep -qxF "$sym" <<<"$allowed" || grep -qxF "$base" <<<"$allowed"; then
        continue
    fi
    echo "WRITABLE GLOBAL: $sym ($size bytes, $sec, $obj)" >&2
    fail=$((fail + 1))
done <<<"$inv"
if [ "$fail" -gt 0 ]; then
    echo "FAIL: $fail non-whitelisted writable symbol(s); each is state shared by every environment" >&2
    exit 1
fi
echo "OK: no writable globals outside $(basename "$WL")"
