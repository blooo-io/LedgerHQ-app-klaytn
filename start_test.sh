#!/usr/bin/env bash

device_label=${1:-"nanox"} # "nanos", "nanox", "nanosp"
golden_run=${2:-false} # true or false, treated as a boolean

mkdir -p tests/elfs
[ -f bin/app.elf ] && cp bin/app.elf tests/elfs/klaytn_${device_label}.elf

echo "Running tests on ${device_label}"

# Construct pytest_cmd in one go, using a conditional expression to append the --golden_run flag if necessary
pytest_cmd="pytest -s tests/python/ -v --tb=short --device ${device_label}"
[[ $golden_run == true ]] && pytest_cmd="$pytest_cmd --golden_run"

eval $pytest_cmd