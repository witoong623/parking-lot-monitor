#!/bin/bash

echo "Building prod image"

if [ $# -lt 1 ]; then
	echo "Usage: $0 <image_name>"
	exit 1
fi

docker build \
	-t $1 \
	--target prod \
	-f docker/Dockerfile .
