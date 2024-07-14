#!/bin/bash

set -e

config="config/dstest3_config.yml"

if [ "$1" == "Release" ]; then
    ./build-release/apps/plm $config
else
    ./build-debug/apps/plm $config
fi
