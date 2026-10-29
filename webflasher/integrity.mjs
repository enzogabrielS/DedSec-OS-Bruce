import { firmwareById } from "./catalog.mjs";

export function validateManifest(manifest, release, editionId = "classic") {
  const edition = firmwareById(editionId);
  const builds = manifest.builds;
  if (builds?.length !== 1 || builds[0].chipFamily !== "ESP32" ||
      manifest.name !== edition.name + " — CYD ESP32-2432S028" ||
      builds[0].parts?.length !== 1 || builds[0].parts[0].offset !== 0 ||
      builds[0].parts[0].path !== edition.binary ||
      release.validation?.environment !== edition.environment ||
      (release.variant ?? "classic") !== edition.id ||
      manifest.version !== release.version || manifest.new_install_prompt_erase !== true ||
      manifest.new_install_improv_wait_time !== 0) {
    throw new Error("Manifesto incompatível com a CYD padrão.");
  }
}

export async function verifyFirmware(buffer, report, cryptoProvider = globalThis.crypto) {
  const bytes = new Uint8Array(buffer);
  if (bytes.length !== report.merged_bytes || bytes.length <= 0x10000 || bytes.length > 0x3d0000 ||
      bytes[0x1000] !== 0xe9 || bytes[0x10000] !== 0xe9 ||
      bytes[0x100c] !== 0 || bytes[0x100d] !== 0 ||
      bytes[0x1002] !== 2 || bytes[0x1003] !== 0x20 ||
      bytes[0x8000] !== 0xaa || bytes[0x8001] !== 0x50) {
    throw new Error("Imagem completa inválida ou incompatível com flash de 4 MiB.");
  }
  const digest = await cryptoProvider.subtle.digest("SHA-256", buffer);
  const sha = Array.from(new Uint8Array(digest), value => value.toString(16).padStart(2, "0")).join("");
  if (sha !== report.merged_sha256) throw new Error("SHA-256 divergente. A instalação foi bloqueada.");
  return sha;
}
