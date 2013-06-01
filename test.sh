#!/bin/bash

BASE_PORT=5000
NUM_PORTS=10

i=0
CMD='./proxy'
while [ $i -lt $NUM_PORTS ]; do
  CMD="${CMD} -l 0.0.0.0:$((i + BASE_PORT))"
  i=$((i + 1))
done
CMD="${CMD} -r 127.0.0.1:10000"
echo "${CMD}"
${CMD}
