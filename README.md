[![build_and_functional_tests](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/build_and_functional_tests.yml/badge.svg)](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/build_and_functional_tests.yml)
[![codeql_checks](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/codeql_checks.yml/badge.svg)](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/codeql_checks.yml)
[![guidelines_enforcer](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/guidelines_enforcer.yml/badge.svg)](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/guidelines_enforcer.yml)
 [![lint-workflow](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/lint-workflow.yml/badge.svg)](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/lint-workflow.yml)
[![Sonarcloud](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/sonarcloud.yml/badge.svg)](https://github.com/blooo-io/LedgerHQ-app-klaytn/actions/workflows/sonarcloud.yml)


# Klaytn Application for Ledger Wallets

## Introduction

The Klaytn application enables Ledger Nano S and Ledger Nano X hardware wallets to support the Klaytn native token. This document provides comprehensive guidelines for setting up your development environment, building, cleaning, loading, and testing the Klaytn application.


## Prerequisites for Development

Ensure you have the following tools installed and configured for your development environment:

- [Docker](https://docs.docker.com/get-docker/) for container management.
- [Visual Studio Code](https://code.visualstudio.com/download) as the integrated development environment.
- Ledger Nano [udev rules](https://github.com/LedgerHQ/udev-rules) for Linux users to ensure device compatibility.

### Setting Up Your Environment

To facilitate a seamless development experience across macOS, Windows, or Linux, utilize the [ledger-app-dev-tools](https://github.com/LedgerHQ/ledger-app-builder/pkgs/container/ledger-app-builder%2Fledger-app-dev-tools) Docker image. This setup allows you to build, test on Speculos emulator, and load applications onto Ledger devices efficiently.

#### Steps to Initialize Your Development Environment:

1. Start Docker and ensure it's running correctly on your system.
2. Verify the presence of an X11 server:
   - Default on Ubuntu Linux.
   - On macOS, install and configure [XQuartz](https://www.xquartz.org/).
   - For Windows, use [VcXsrv](https://sourceforge.net/projects/vcxsrv/), configuring it to disable access control.
3. Clone the Klaytn application repository: `git clone https://github.com/blooo-io/LedgerHQ-app-klaytn.git`.
4. Open the cloned repository folder in Visual Studio Code.
5. Use the `ctrl + shift + b` (or `command + shift + b` on Mac) shortcut to access Visual Studio Code tasks and execute the `Run dev-tools image` to pull and run the ledger-app-dev-tools Docker image.

## Building the Application

To build the Klaytn application for your Ledger device, follow these steps:

1. Open Visual Studio Code tasks using `ctrl + shift + b` (or `command + shift + b` on Mac).
2. Choose `Build app` for standard build or `Build app [Debug]` for debugging purposes.

## Cleaning Build Files

For cleaning up build files:

1. Access Visual Studio Code tasks with `ctrl + shift + b` (or `command + shift + b` on Mac).
2. Select `Clean build files` to remove any existing build artifacts.

## Loading the Application onto a Ledger Device

To load the built application onto a Ledger device:

1. Use `ctrl + shift + b` (or `command + shift + b` on Mac) to open tasks in Visual Studio Code.
2. Choose `Load app on device`.

### Troubleshooting Load Errors

If encountering errors during the load process, such as failed device open paths, ensure:

- The device is connected directly to the computer using the official Ledger cable. Avoid using adapters or hubs as they can cause connection issues.
- Retry the `Build app` process, then run the command shown in the terminal as `sudo su` in a new terminal window.
- Attempt loading the application again.

## Testing the Application

### Python Tests

1. Access tasks in Visual Studio Code with `ctrl + shift + b` (or `command + shift + b` on Mac).
2. Execute `Run functional tests (without logs)`.
3. Select the appropriate SDK and model for testing.

### Unit Tests

Execute C tests using the following command:

```bash
make -C lib
