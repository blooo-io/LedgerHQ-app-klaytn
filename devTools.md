# Developer tools documentation

## Building the app

The `docker-make` script is used to build and clean the binaries of the Klaytn app.
This script will call the `make` command in the docker image ledger-app-builder.
It builds for the specified nano device and will pass the additional parameters to the `make` called.

### Usage
- Build
    ```bash
    ./docker-make [TARGET_SDK] [GCC_PARAMS]
    ```
    `TARGET_SDK`: The targeted sdk for building the app
    - Acceptable values: `s`, `x`, `sp`.

    `GCC_PARAMS`: The parameters passed to the gcc compiler

- Clean
    ```bash
    ./docker-make [TARGET_SDK] clean
    ```
    `TARGET_SDK`: (Optional) The sdk used for building the app

### Examples
Build the klaytn app on nanosp with debug enabled

```bash
./docker-make sp DEBUG=1
```

Clean the binaries
```bash
./docker-make sp clean
```

## Launch python tests

The `start_test.sh` script is used to initiate testing for various devices with an optional golden run parameter.

### Prerequisites
Ensure you have the necessary permissions to execute the `start_test.sh` script.

### Usage
```
./start_test.sh [DEVICE_LABEL] [GOLDEN_RUN]
```
`DEVICE_LABEL`: The label of the device on which you intend to run the tests.
- Default: `nanos`.
- Acceptable values: `nanos`, `nanox`, `nanosp`.

`GOLDEN_RUN`: Determines if the tests should run in golden mode. Default: `false`.
Acceptable values:truefalse.

### Examples

Running tests on "nanox" device without golden run:
```bash
./start_test.sh nanox
```

Running tests on "nanosp" device with a golden run:
```bash
./start_test.sh nanosp true
```

## Debug with GDB

Debug the klaytn application using the speculos emulator mounted with a GDB debugger.

### Usage

```bash
# Load the app and launch the GDB debugger
./gdb.sh
```
### Commands

The commands are listed on https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf

> **_NOTE:_**  If the GDB is stuck, open a new terminal and run `docker compose down`