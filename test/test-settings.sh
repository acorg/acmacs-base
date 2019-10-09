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
../dist/test-settings >"${TDIR}/test-settings.output.txt" 2>/dev/null
if ! cmp "${TDIR}/test-settings.output.txt" test-settings-expected-output.txt; then
    diff "${TDIR}/test-settings.output.txt" test-settings-expected-output.txt
    echo
    echo
    ../dist/test-settings
    false
fi
