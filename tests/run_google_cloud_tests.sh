#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_binary=$(mktemp /tmp/google-cloud-test.XXXXXX)
trap 'rm -f "$test_binary"' EXIT HUP INT TERM
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -IApplication Application/google_cloud_protocol.c tests/google_cloud_protocol_test.c \
  -o "$test_binary"
"$test_binary"
node tests/google_apps_script_test.js
python3 tests/net_would_block_test.py
python3 tests/google_cloud_time_test.py
