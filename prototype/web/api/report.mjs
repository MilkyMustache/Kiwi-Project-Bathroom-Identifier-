import { timingSafeEqual } from "node:crypto";
import { saveReading } from "../store.mjs";

const headers = { "Cache-Control": "no-store" };

function authorized(received, expected) {
  if (!received || !expected) return false;
  const left = Buffer.from(received);
  const right = Buffer.from(expected);
  return left.length === right.length && timingSafeEqual(left, right);
}

export async function POST(request) {
  if (!authorized(request.headers.get("x-device-token"), process.env.DEVICE_TOKEN)) {
    return Response.json({ error: "Unauthorized" }, { status: 401, headers });
  }

  let reading;
  try {
    const raw = await request.text();
    if (raw.length > 256) throw new Error("Body too large");
    reading = JSON.parse(raw);
    if (typeof reading.sensorOk !== "boolean") throw new Error("Invalid sensor status");
    if (reading.sensorOk && (!Number.isInteger(reading.distanceMm) ||
        reading.distanceMm < 0 || reading.distanceMm > 65534)) {
      throw new Error("Invalid distance");
    }
  } catch {
    return Response.json({ error: "Invalid reading" }, { status: 400, headers });
  }

  try {
    await saveReading({
      sensorOk: reading.sensorOk,
      distanceMm: reading.sensorOk ? reading.distanceMm : null,
      reportedAt: new Date().toISOString(),
    });
    return Response.json({ ok: true }, { headers });
  } catch {
    return Response.json({ error: "Storage unavailable" }, { status: 503, headers });
  }
}
