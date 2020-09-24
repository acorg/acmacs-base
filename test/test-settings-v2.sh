#! /bin/bash

TDIR=$(mktemp -d -t XXXXXX)
TESTDIR=$(dirname $0)

# ======================================================================

function on_exit
{
    rm -rf "$TDIR"
}

trap on_exit EXIT

function failed
{
    echo ERROR: "$@" FAILED >&2
    exit 1
}

trap failed ERR

# ======================================================================

cd "$TESTDIR"
# ../dist/test-settings
../dist/test-settings-v2 >"${TDIR}/test-settings.output.txt" 2>"${TDIR}/test-settings.err.txt" || ( echo "********************"; cat "${TDIR}/test-settings.output.txt"; echo "********************"; cat "${TDIR}/test-settings.err.txt"; false )
if ! cmp "${TDIR}/test-settings.output.txt" test-settings-expected-output.txt; then
    diff "${TDIR}/test-settings.output.txt" test-settings-expected-output.txt || true
    echo
    echo
    ../dist/test-settings-v2
    false
fi

if [[ -f "${ACMACSD_ROOT}/share/conf/tal.json" ]]; then
    ../dist/test-settings-v2 "${ACMACSD_ROOT}/share/conf/tal.json"
fi
