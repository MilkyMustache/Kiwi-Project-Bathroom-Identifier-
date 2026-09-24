#pragma once

// Copy this file to config.h in this folder. config.h is ignored by Git.
// Register this ESP32's Wi-Fi MAC in the campus Wi-Fi device portal first.
constexpr char kWifiSsid[] = "YOUR_WIFI_SSID";
constexpr char kWifiPassword[] = "";
constexpr char kReportUrl[] = "";  // https://your-project.vercel.app/api/report
constexpr char kDeviceToken[] = ""; // Same random value as Vercel DEVICE_TOKEN.
