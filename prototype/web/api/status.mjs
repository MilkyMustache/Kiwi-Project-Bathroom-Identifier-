import { getReading } from "../store.mjs";

export async function GET() {
  const headers = { "Cache-Control": "no-store, max-age=0" };
  try {
    const reading = await getReading();
    const ageMs = reading ? Date.now() - Date.parse(reading.reportedAt) : Infinity;
    const online = ageMs >= 0 && ageMs <= 60000;
    return Response.json({
      online,
      sensorOk: online ? reading.sensorOk : false,
      distanceMm: online && reading.sensorOk ? reading.distanceMm : null,
      reportedAt: reading?.reportedAt ?? null,
    }, { headers });
  } catch {
    return Response.json({ online: false, error: "Status unavailable" },
      { status: 503, headers });
  }
}
