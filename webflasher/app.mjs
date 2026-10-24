import { validateManifest, verifyFirmware } from "./integrity.mjs";

const $ = selector => document.querySelector(selector);
const installer = $("#installer");
const button = $("#install-button");
const status = $("#integrity-status");
let ready = false;
let manifestURL;
let firmwareURL;

function updateButton() {
  const confirmed = $("#board-confirm").checked && $("#data-confirm").checked;
  button.disabled = !ready || !confirmed;
  if (ready) $("#readiness").textContent = confirmed
    ? "Pronto. Clique para escolher a porta USB da sua CYD."
    : "Confirme o modelo da placa e a substituição da firmware para continuar.";
}

async function getJSON(path) {
  const response = await fetch(new URL(path, import.meta.url), { cache: "no-store" });
  if (!response.ok) throw new Error(`Não foi possível carregar ${path} (HTTP ${response.status}).`);
  return response.json();
}

function loadInstaller() {
  // Pin the upstream module; never silently upgrade the flashing implementation.
  const script = document.createElement("script");
  script.type = "module";
  script.src = "https://unpkg.com/esp-web-tools@10.4.0/dist/web/install-button.js?module";
  const loaded = new Promise((resolve, reject) => {
    script.onload = () => customElements.get("esp-web-install-button") ? resolve() : reject(new Error("Componente USB não registrado."));
    script.onerror = () => reject(new Error("Não foi possível carregar o ESP Web Tools. Verifique a conexão ou o bloqueador de conteúdo."));
  });
  document.head.append(script);
  return loaded;
}

async function prepare() {
  try {
    if (!window.isSecureContext || !globalThis.crypto?.subtle) {
      throw new Error("Abra esta página por HTTPS ou localhost para verificar e gravar a firmware.");
    }
    const [manifest, release] = await Promise.all([getJSON("./manifest.json"), getJSON("./release.json")]);
    validateManifest(manifest, release);
    $("#version").textContent = release.version;
    $("#size").textContent = `${release.validation.merged_bytes.toLocaleString("pt-BR")} bytes`;
    status.textContent = "Baixando e conferindo SHA-256…";
    const response = await fetch(new URL(manifest.builds[0].parts[0].path, import.meta.url), { cache: "no-store" });
    if (!response.ok) throw new Error(`Falha ao baixar a firmware (HTTP ${response.status}).`);
    const buffer = await response.arrayBuffer();
    const sha = await verifyFirmware(buffer, release.validation);
    $("#sha").textContent = sha;
    // The installer reads the exact bytes that passed verification, not a second network download.
    firmwareURL = URL.createObjectURL(new Blob([buffer], { type: "application/octet-stream" }));
    manifest.builds[0].parts[0].path = firmwareURL;
    manifestURL = URL.createObjectURL(new Blob([JSON.stringify(manifest)], { type: "application/json" }));
    const download = $("#download");
    download.href = firmwareURL;
    download.hidden = false;
    status.textContent = "SHA-256 conferido · imagem íntegra";
    status.dataset.state = "ok";
    if (!("serial" in navigator)) {
      $("#compatibility").textContent = "Seu navegador não oferece Web Serial. Para instalar, use Chrome ou Edge no computador. O download verificado continua disponível.";
      $("#compatibility").hidden = false;
      $("#readiness").textContent = "Navegador sem acesso à porta serial.";
      return;
    }
    await loadInstaller();
    installer.manifest = manifestURL;
    installer.hidden = false;
    ready = true;
    updateButton();
  } catch (error) {
    ready = false;
    button.disabled = true;
    status.textContent = error.message;
    status.dataset.state = "error";
    $("#readiness").textContent = "Instalação bloqueada. Corrija o erro e recarregue a página.";
  }
}

$("#board-confirm").addEventListener("change", updateButton);
$("#data-confirm").addEventListener("change", updateButton);
installer.addEventListener("state-changed", event => {
  const { state, details } = event.detail;
  const messages = {
    initializing: "Conectando à placa…",
    preparing: "Preparando a gravação…",
    erasing: "Apagando a flash. Não desconecte o cabo.",
    writing: `Gravando ${Math.round(details?.percentage ?? 0)}%. Não desconecte o cabo.`,
    finished: "Gravação concluída. Aguarde o reinício da CYD; se necessário, pressione RST/EN.",
    error: "Falha na gravação. Consulte os detalhes no diálogo e as instruções abaixo."
  };
  if (messages[state]) $("#readiness").textContent = messages[state];
});
// Keep Blob URLs alive for the page lifetime, including navigation restored from the back/forward cache.
prepare();
