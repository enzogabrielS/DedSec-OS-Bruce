import { test } from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { webcrypto } from "node:crypto";
import { validateManifest, verifyFirmware } from "../../webflasher/integrity.mjs";

const base = new URL("../../webflasher/", import.meta.url);
const manifest = JSON.parse(await readFile(new URL("manifest.json", base)));
const release = JSON.parse(await readFile(new URL("release.json", base)));
const data = new Uint8Array(await readFile(new URL(manifest.builds[0].parts[0].path, base)));

test("published image and manifest match the validated release", async () => {
  validateManifest(manifest, release);
  assert.equal(await verifyFirmware(data, release.validation, webcrypto), release.validation.merged_sha256);
});

test("a damaged download fails SHA-256 verification", async () => {
  const damaged = data.slice();
  damaged[0x20000] ^= 1;
  await assert.rejects(verifyFirmware(damaged, release.validation, webcrypto), /SHA-256/);
});

test("truncated images and application-only images are rejected", async () => {
  await assert.rejects(verifyFirmware(data.slice(0, -1), release.validation, webcrypto), /inválida/);
  await assert.rejects(verifyFirmware(data.slice(0x10000), release.validation, webcrypto), /inválida/);
});

test("wrong chip, offset, erase policy and remote binary are rejected", () => {
  const edits = [
    value => { value.builds[0].chipFamily = "ESP32-S3"; },
    value => { value.builds[0].parts[0].offset = 65536; },
    value => { value.builds[0].parts[0].path = "https://example.com/other.bin"; },
    value => { value.new_install_prompt_erase = false; },
    value => { value.new_install_improv_wait_time = 10000; }
  ];
  for (const edit of edits) {
    const candidate = structuredClone(manifest);
    edit(candidate);
    assert.throws(() => validateManifest(candidate, release), /Manifesto/);
  }
});

test("a different target in the ESP image header is rejected", async () => {
  const different = data.slice();
  different[0x100c] = 9;
  await assert.rejects(verifyFirmware(different, release.validation, webcrypto), /incompatível/);
});
