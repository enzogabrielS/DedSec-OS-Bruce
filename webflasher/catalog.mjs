export const FIRMWARES = Object.freeze({
  classic: Object.freeze({
    id: "classic", name: "DedSec OS Classic",
    description: "A versão atual: carrossel, artes DedSec, fundo alternativo e boot configurável.",
    manifest: "manifest.json", release: "release.json",
    binary: "firmware/DedSec-OS-CYD-2432S028.bin", environment: "DedSec-CYD-2432S028"
  }),
  grid: Object.freeze({
    id: "grid", name: "DedSec OS Grid",
    description: "Nova interface: seis módulos por página, toque direto, listas de ferramentas e destaques vermelhos.",
    manifest: "manifest-grid.json", release: "release-grid.json",
    binary: "firmware/DedSec-OS-Grid-CYD-2432S028.bin", environment: "DedSec-Grid-CYD-2432S028"
  })
});
export function firmwareById(id) {
  const firmware = Object.hasOwn(FIRMWARES, id) ? FIRMWARES[id] : null;
  if (!firmware) throw new Error("Firmware desconhecida.");
  return firmware;
}
// A late download cannot replace a more recent selection.
export function selectionGate() {
  let revision = 0;
  return { next: () => ++revision, current: token => token === revision };
}
