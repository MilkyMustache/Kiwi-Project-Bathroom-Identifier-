const KEY = "bathroom:mvp:latest";

async function command(parts) {
  const url = process.env.KV_REST_API_URL || process.env.UPSTASH_REDIS_REST_URL;
  const token = process.env.KV_REST_API_TOKEN || process.env.UPSTASH_REDIS_REST_TOKEN;
  if (!url || !token) throw new Error("Storage is not configured");

  const response = await fetch(url, {
    method: "POST",
    headers: {
      Authorization: `Bearer ${token}`,
      "Content-Type": "application/json",
    },
    body: JSON.stringify(parts),
    signal: AbortSignal.timeout(5000),
  });
  if (!response.ok) throw new Error(`Storage returned ${response.status}`);
  const body = await response.json();
  if (body.error) throw new Error("Storage rejected the command");
  return body.result;
}

export async function saveReading(reading) {
  // Only the latest update matters. The key expires if the ESP32 stops reporting.
  await command(["SET", KEY, JSON.stringify(reading), "EX", 90]);
}

export async function getReading() {
  const value = await command(["GET", KEY]);
  return value ? JSON.parse(value) : null;
}
