#!/usr/bin/env bash
device_label=${1:-"nanos"} # "nanos", "nanox", "nanosp"
app_name=${2:-"solana"} # "solana", "ethereum"
if [[ $app_name == "solana" ]]; then
    mkdir -p tests/elfs
    [ -f bin/app.elf ] && cp bin/app.elf tests/elfs/solana_${device_label}.elf
else
    mkdir -p tests/elfs
    [ -f bin_eth/app.elf ] && cp bin_eth/app.elf tests/elfs/solana_${device_label}.elf
fi

echo "Running tests for ${app_name} on ${device_label}"

pytest -s tests/python/ -v --tb=short --device ${device_label} #--golden_run