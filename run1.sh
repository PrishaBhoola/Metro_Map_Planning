#!/bin/bash
# Usage: ./run1.sh test
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <basename>"
  exit 1
fi
BASE=$1
if [ ! -f "${BASE}.city" ]; then
  echo "${BASE}.city not found"
  exit 1
fi
bin/run1 "${BASE}.city" "${BASE}.satinput" "${BASE}.vars"
echo "Wrote ${BASE}.satinput and ${BASE}.vars"