import { validateManifest, verifyFirmware } from "./integrity.mjs";
import { firmwareById, selectionGate } from "./catalog.mjs";
const $ = selector => document.querySelector(selector);
const installer = $("#installer"), button = $("#install-button"), status = $("#integrity-status");
const selector = $("#firmware-select"), selection = selectionGate(), packets = new Map();
let ready = false, installerLoader;
function updateButton() {
  const confirmed = $("#board-confirm").checked && $("#data-confirm").checked;
  button.disabled = !ready || !confirmed || selector.disabled;
  if (ready && !selector.disabled) $("#readiness").textContent = confirmed
    ? "Pronto. Confira a edição escolhida e clique para selecionar a porta USB."
    : "Confirme o modelo da placa e a substituição da firmware para continuar.";
}
async function getJSON(path) {
  const response = await fetch(new URL(path, import.meta.url), { cache: "no-store" });
  if (!response.ok) throw new Error(`Não foi possível carregar ${path} (HTTP ${response.status}).`);
  return response.json();
}
function loadInstaller() {
  if (installerLoader) return installerLoader;
  const script = document.createElement("script");
  script.type = "module";
  script.src = "https://unpkg.com/esp-web-tools@10.4.0/dist/web/install-button.js?module";
  installerLoader = new Promise((resolve, reject) => {
    script.onload = () => customElements.get("esp-web-install-button")
      ? resolve() : reject(new Error("Componente USB não registrado."));
    script.onerror = () => reject(new Error("Não foi possível carregar o ESP Web Tools. Recarregue a página."));
  });
  document.head.append(script);
  return installerLoader;
}
async function loadPacket(edition) {
  const [manifest, release] = await Promise.all([getJSON(edition.manifest), getJSON(edition.release)]);
  validateManifest(manifest, release, edition.id);
  const response = await fetch(new URL(edition.binary, import.meta.url), { cache: "no-store" });
  if (!response.ok) throw new Error(`Falha ao baixar a firmware (HTTP ${response.status}).`);
  const buffer = await response.arrayBuffer(), sha = await verifyFirmware(buffer, release.validation);
  const firmwareURL = URL.createObjectURL(new Blob([buffer], { type: "application/octet-stream" }));
  const installManifest = structuredClone(manifest);
  installManifest.builds[0].parts[0].path = firmwareURL;
  const manifestURL = URL.createObjectURL(new Blob([JSON.stringify(installManifest)], { type: "application/json" }));
  return { release, sha, firmwareURL, manifestURL };
}
async function prepare() {
  const token = selection.next();
  ready = false;
  button.disabled = true;
  installer.hidden = true;
  installer.manifest = undefined;
  $("#download").hidden = true;
  $("#data-confirm").checked = false;
  $("#version").textContent = $("#size").textContent = "—";
  $("#sha").textContent = "Aguardando verificação.";
  status.dataset.state = "";
  try {
    const edition = firmwareById(selector.value);
    $("#edition").textContent = $("#selected-name").textContent = edition.name;
    $("#firmware-description").textContent = edition.description;
    document.documentElement.dataset.edition = edition.id;
    status.textContent = "Baixando e conferindo a firmware escolhida…";
    $("#readiness").textContent = "Verificando o pacote de instalação…";
    if (!window.isSecureContext || !globalThis.crypto?.subtle)
      throw new Error("Abra por HTTPS ou localhost para verificar e gravar a firmware.");
    if (!packets.has(edition.id)) packets.set(edition.id, loadPacket(edition).catch(error => {
      packets.delete(edition.id); throw error;
    }));
    const packet = await packets.get(edition.id);
    if (!selection.current(token)) return;
    $("#version").textContent = packet.release.version;
    $("#size").textContent = `${packet.release.validation.merged_bytes.toLocaleString("pt-BR")} bytes`;
    $("#sha").textContent = packet.sha;
    const download = $("#download");
    download.href = packet.firmwareURL;
    download.download = edition.binary.split("/").pop();
    download.hidden = false;
    status.textContent = "SHA-256 conferido · " + edition.name;
    status.dataset.state = "ok";
    if (!("serial" in navigator)) {
      $("#compatibility").textContent = "Para instalar, use Chrome ou Edge no computador. O download verificado continua disponível.";
      $("#compatibility").hidden = false;
      $("#readiness").textContent = "Navegador sem acesso à porta serial.";
      return;
    }
    await loadInstaller();
    if (!selection.current(token)) return;
    installer.manifest = packet.manifestURL;
    installer.hidden = false;
    ready = true;
    updateButton();
  } catch (error) {
    if (!selection.current(token)) return;
    ready = false;
    button.disabled = true;
    status.textContent = error.message;
    status.dataset.state = "error";
    $("#readiness").textContent = "Instalação bloqueada. Corrija o erro e recarregue a página.";
  }
}
selector.addEventListener("change", prepare);
$("#board-confirm").addEventListener("change", updateButton);
$("#data-confirm").addEventListener("change", updateButton);
// The official component captures its manifest when creating this dialog.
// Keep both Blob pairs alive and lock edition changes until that dialog closes.
new MutationObserver(records => {
  for (const record of records) for (const node of record.addedNodes) {
    if (node.nodeName !== "EWT-INSTALL-DIALOG") continue;
    selector.disabled = true;
    updateButton();
    $("#readiness").textContent = "Instalador aberto. Acompanhe a gravação no diálogo; mantenha o cabo conectado.";
    node.addEventListener("closed", () => { selector.disabled = false; updateButton(); }, { once: true });
  }
}).observe(document.body, { childList: true });
// At most two verified packets; URLs stay alive during installation and bfcache.
prepare();
