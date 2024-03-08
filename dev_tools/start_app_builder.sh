#!/bin/bash

docker run --user $(id -u):$(id -g) --privileged -e DISPLAY='host.docker.internal:0' -v '/tmp/.X11-unix:/tmp/.X11-unix' -p 5000:5000 -v '.:/app' -t -d --name LedgerHQ-app-klaytn-container ghcr.io/ledgerhq/ledger-app-builder/ledger-app-dev-tools:latest

