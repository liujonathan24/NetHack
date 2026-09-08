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
OBJDIR="${1:?object dir}"
WL="${2:-$HERE/whitelist.txt}"
PY="${PYTHON:-python3}"
allowed=$(grep -v '^#' "$WL" | sed 's/[[:space:]].*//' | grep -v '^$' || true)
fail=0; n=0
while IFS=$'\t' read -r sym size sec obj; do
    [ -z "$sym" ] && continue
    base="${sym%%.*}"
    if grep -qxF "$sym" <<<"$allowed" || grep -qxF "$base" <<<"$allowed"; then
        continue
    fi
    echo "WRITABLE GLOBAL: $sym ($size bytes, $sec, $obj)" >&2
    fail=$((fail + 1))
done < <(cd "$HERE" && $PY -m nhglobals inventory "$OBJDIR")
if [ "$fail" -gt 0 ]; then
    echo "FAIL: $fail non-whitelisted writable symbol(s); each is state shared by every environment" >&2
    exit 1
fi
echo "OK: no writable globals outside $(basename "$WL")"
