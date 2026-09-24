import test from "node:test";
import assert from "node:assert/strict";
import { POST } from "./api/report.mjs";
import { GET } from "./api/status.mjs";

test("authenticated sensor updates appear, then become unknown when stale", async () => {
  process.env.DEVICE_TOKEN = "test-token";
  process.env.KV_REST_API_URL = "https://test.upstash.io";
  process.env.KV_REST_API_TOKEN = "test-storage-token";
  const originalFetch = globalThis.fetch;
  let saved = null;
  globalThis.fetch = async (_url, options) => {
    const command = JSON.parse(options.body);
    if (command[0] === "SET") {
      saved = command[2];
      assert.deepEqual(command.slice(3), ["EX", 90]);
      return Response.json({ result: "OK" });
    }
    if (command[0] === "GET") return Response.json({ result: saved });
    throw new Error("Unexpected storage command");
  };

  try {
    const makeRequest = (token, body) => new Request("https://example.test/api/report", {
      method: "POST",
      headers: { "x-device-token": token },
      body: JSON.stringify(body),
    });
    assert.equal((await POST(makeRequest("wrong", { sensorOk: true, distanceMm: 123 }))).status, 401);
    assert.equal((await POST(makeRequest("test-token", { sensorOk: true, distanceMm: -1 }))).status, 400);
    assert.equal((await POST(makeRequest("test-token", { sensorOk: true, distanceMm: 123 }))).status, 200);
    const fresh = await (await GET()).json();
    assert.equal(fresh.online, true);
    assert.equal(fresh.distanceMm, 123);
    saved = JSON.stringify({ ...JSON.parse(saved), reportedAt: new Date(Date.now() - 61000).toISOString() });
    const stale = await (await GET()).json();
    assert.equal(stale.online, false);
    assert.equal(stale.distanceMm, null);
  } finally {
    globalThis.fetch = originalFetch;
  }
});
