#!/usr/bin/env bash

#!/usr/bin/env bash
set -e

run_exp_using_descriptor.py -d lab1.json -a cse220 -g cse220 -m 220

grep --recursive --extended-regexp --exclude *.warmup --regexp="^BP_ON_PATH_CORRECT_pct" ./exp/
