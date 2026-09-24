#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build_dir="${TMPDIR:-/tmp}/bathroom-sensor-monitor"
mkdir -p "$build_dir/swift-modules" "$build_dir/clang-modules"

# This Mac's default 27.0 SDK does not match its installed Swift compiler.
sdk=/Library/Developer/CommandLineTools/SDKs/MacOSX26.5.sdk
if [ ! -d "$sdk" ]; then
  sdk=$(xcrun --show-sdk-path)
fi

swiftc \
  -sdk "$sdk" \
  -module-cache-path "$build_dir/swift-modules" \
  -Xcc "-fmodules-cache-path=$build_dir/clang-modules" \
  -o "$build_dir/bathroom_sensor_monitor" \
  "$project_dir/mac_ble_monitor.swift"

exec "$build_dir/bathroom_sensor_monitor"
