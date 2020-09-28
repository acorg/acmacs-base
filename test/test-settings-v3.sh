#! /bin/bash

TDIR=$(mktemp -d -t XXXXXX)
TESTDIR=$(dirname $0)

# ======================================================================

function on_exit
{
    if [[ -d "${TDIR}" ]]; then
        rm -rf "${TDIR}"
    fi
}

trap on_exit EXIT

function failed
{
    echo ERROR: "$@" FAILED >&2
    exit 1
}

trap failed ERR

# ======================================================================

cd "${TESTDIR}"

../dist/test-settings-v3 ./test-settings-v3.1.json main "$@" >"${TDIR}/test-settings-v3.output.txt"
if ! cmp "${TDIR}/test-settings-v3.output.txt" test-settings-v3.expected.txt; then
    diff "${TDIR}/test-settings-v3.output.txt" test-settings-v3.expected.txt || true
    echo
    echo
    ../dist/test-settings-v3
    false
fi
