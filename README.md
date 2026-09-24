# Kiwi Bathroom Identifier

This repository contains the original ESPHome / Home Assistant sensor configuration and Julian’s [ESP32 Wi-Fi / web prototype](prototype/README.md). The prototype has reported live VL53L0X distances to its hosted page; occupancy calibration and [two-AA power testing](prototype/POWER.md) are still pending.

The ESPHome configuration reads local values from `secrets.yaml`; copy `secrets.example.yaml` to that ignored file and fill it locally. Credentials previously committed to this public repository need rotation because Git retains older revisions.

Public copies of the connection logs omit the device MAC, private IP, and local serial port identifiers. The raw logs remain in the original private project.
