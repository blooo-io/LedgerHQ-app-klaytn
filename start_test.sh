device_label=$1 # "nanos", "nanox", "nanosp"
cp bin/app.elf bin/solana_${device_label}.elf
mkdir tests/elfs
cp bin/*.elf tests/elfs

pytest tests/python/ -v --tb=short --device ${device_label}