#!/bin/bash
# Usage: ./run2.sh test
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <basename>"
  exit 1
fi
BASE=$1
if [ ! -f "${BASE}.city" ]; then
  echo "${BASE}.city not found"
  exit 1
fi
if [ ! -f "${BASE}.satoutput" ]; then
  echo "${BASE}.satoutput not found"
  exit 1
fi
bin/run2 "${BASE}.city" "${BASE}.satoutput" "${BASE}.metromap" "${BASE}.vars"
echo "Wrote ${BASE}.metromap"