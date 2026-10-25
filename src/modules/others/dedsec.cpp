#ifdef DEDSEC_OS
#include "dedsec.h"
#include "core/dedsec_art.h"
#include "core/dedsec_palette.h"
#include "core/display.h"
#include "core/radio_mem.h"
#include "core/ram_profile.h"
#include "core/scrollableTextArea.h"
#include "core/utils.h"
#include "core/wifi/wifi_common.h"
#include "esp_bt.h"
#include "esp_heap_caps.h"
#include "esp_wifi.h"
#include "modules/ble/ble_common.h"
#include "modules/wifi/ap_info.h"
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <cstring>
#include <atomic>
#include <memory>
#include <new>

namespace {
std::atomic<bool> surveyFinished{false};
std::atomic<uint32_t> surveyStatus{0};

void surveyComplete(void *, esp_event_base_t, int32_t, void *data) {
    const auto *event = static_cast<const wifi_event_sta_scan_done_t *>(data);
    surveyStatus.store(event ? event->status : 1U);
    surveyFinished.store(true);
}

const char *wifiState() {
    if (WiFi.isConnected()) return "conectado";
    switch (WiFi.getMode()) {
        case WIFI_AP: return "AP";
        case WIFI_AP_STA: return "AP + STA";
        case WIFI_STA: return "STA sem link";
        default: return "OFF";
    }
}

const char *bluetoothState() {
    switch (esp_bt_controller_get_status()) {
        case ESP_BT_CONTROLLER_STATUS_ENABLED: return "ativo";
        case ESP_BT_CONTROLLER_STATUS_INITED: return "inicializado";
        default: return "OFF";
    }
}

bool confirm(const char *effect, const char *action) {
    bool accepted = false;
    std::vector<Option> choices = {
        {"Cancelar", []() {}},
        {action, [&accepted]() { accepted = true; }}
    };
    // Cancel is the default; a held press is filtered by Bruce's loopOptions.
    loopOptions(choices, MENU_TYPE_SUBMENU, effect);
    return accepted;
}

void drawBootLogo(unsigned frame, bool backdrop) {
    // Raw panel dimensions are valid even before settings/storage are loaded.
    const int cx = tft.width() / 2, cy = tft.height() / 2 - 22;
    const int width = min(160, tft.width() - 32), height = min(150, tft.height() - 84);
    tft.setTextFont(1);
    tft.setTextDatum(TL_DATUM);
    if (backdrop) {
        tft.fillScreen(DedSec::Background);
        tft.setTextSize(2);
        tft.setTextColor(DedSec::Text, DedSec::Background);
        tft.drawCentreString("DEDSEC OS", cx, cy + height / 2 + 10, 1);
    }
    tft.fillRect(cx - width / 2 - 4, cy - height / 2 - 4, width + 8, height + 8, DedSec::Background);
    const int shift = frame == 2 ? 3 : (frame == 3 ? -3 : (frame == 5 ? 2 : (frame == 7 ? -1 : 0)));
    dedsecDrawArt(DedSecArt::Logo, cx, cy, width, height, DedSec::Text, DedSec::Background, shift);
    tft.setTextSize(1);
}

// SSIDs are untrusted bytes. Keep control characters out of the UI/serial report.
void readableSsid(const char *ssid, char *buffer, size_t capacity) {
    size_t count = min(strnlen(ssid, 32), capacity - 1);
    for (size_t i = 0; i < count; ++i) {
        uint8_t c = static_cast<uint8_t>(ssid[i]);
        buffer[i] = c >= 32 && c <= 126 ? c : '?';
    }
    buffer[count] = 0;
    if (!count) strlcpy(buffer, "<oculto>", capacity);
}

void networkInfo(const wifi_ap_record_t &network) {
    ScrollableTextArea area("WIFI / OBSERVACAO");
    char ssid[33];
    readableSsid(reinterpret_cast<const char *>(network.ssid), ssid, sizeof(ssid));
    area.addLine(String("SSID: ") + ssid);
    char address[18];
    snprintf(address, sizeof(address), "%02X:%02X:%02X:%02X:%02X:%02X",
             network.bssid[0], network.bssid[1], network.bssid[2],
             network.bssid[3], network.bssid[4], network.bssid[5]);
    area.addLine(String("BSSID: ") + address);
    area.addLine("Canal: " + String(network.primary));
    area.addLine("RSSI: " + String(network.rssi) + " dBm");
    area.addLine("Auth: " + autoMode2String(network.authmode));
    if (network.authmode == WIFI_AUTH_OPEN) area.addLine("Rede aberta: sem autenticacao WiFi.");
    if (network.authmode == WIFI_AUTH_WEP) area.addLine("WEP: protecao legada insegura.");
    area.addLine("Nenhuma conexao ou teste ativo foi realizado.");
    area.addLine("SEL: voltar");
    area.show();
}
}

void dedsecBootStage(const char *stage, unsigned step) {
    // Real diagnostics remain on serial; the panel shows the supplied logo
    // from the first display access, including while storage initializes.
    Serial.printf("[DedSec boot %u/4] %s\n", step, stage);
    if (bruceConfig.instantBoot) return;
    // begin_tft() clears the panel after storage, so repaint at step 2.
    // Other setup checkpoints leave the same logo in place without a full redraw.
    if (step <= 2) drawBootLogo(0, true);
}

bool dedsecBootAnimationTick() {
    static bool finished = false;
    static bool started = false;
    static uint32_t began = 0;
    static unsigned previousFrame = ~0U;
    if (finished) return false;
    if (bruceConfig.instantBoot || !bruceConfig.startupApp.isEmpty()) {
        finished = true;
        return false;
    }
    if (!started) {
        started = true;
        began = millis();
        drawBootLogo(0, true);
        previousFrame = 0;
        tft.setTextColor(DedSec::Muted, DedSec::Background);
        // InputHandler is ready here; no unresponsive touch hint during storage.
        tft.drawCentreString("TOQUE PARA PULAR", tft.width() / 2, tft.height() - 14, 1);
    }
    const uint32_t elapsed = millis() - began;
    // Scheduled by loop(), never a delay sequence or another FreeRTOS task.
    const uint32_t duration = static_cast<uint32_t>(bruceConfig.dedsecBootDuration);
    if (elapsed >= duration || (elapsed >= 120 && check(AnyKeyPress))) {
        finished = true;
        tft.setTextSize(1);
        return false;
    }
    // Repeat short glitches, then hold a clean logo during the final 400 ms.
    const unsigned frame = elapsed + 400 >= duration ? 0 : (elapsed / 80) % 12;
    if (frame != previousFrame) {
        previousFrame = frame;
        drawBootLogo(frame, false);
    }
    return true;
}

void dedsecDashboard() {
    RAM_LOG("dedsec-dashboard-in");
    char previous[9][27] = {};
    char line[27];
    bool first = true;
    uint32_t updated = 0;
    const uint32_t entered = millis();
    drawMainBorder();
    tft.fillRect(6, 27, tftWidth - 12, tftHeight - 28, bruceConfig.bgColor);
    // The entire screen acts as Back, so the resistive touch target is generous.
    tft.fillRect(0, tftHeight, tftWidth, 20, DedSec::Surface);
    tft.setTextSize(2);
    tft.setTextColor(DedSec::Text, DedSec::Surface);
    tft.drawCentreString("TOQUE PARA MENU", tftWidth / 2, tftHeight + 2, 1);
    while (!returnToMenu) {
        if (millis() - entered > 600 && check(AnyKeyPress)) break;
        if (!isScreenOff && (first || millis() - updated >= DedSec::RefreshMs)) {
            updated = millis();
            const bool connected = WiFi.isConnected();
            IPAddress ip;
            if (connected) ip = WiFi.localIP();
            else if (WiFi.getMode() & WIFI_AP) ip = WiFi.softAPIP();
            const size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            const size_t largest = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            for (unsigned row = 0; row < 9; ++row) {
                switch (row) {
                    case 0: strlcpy(line, "CYD-2432S028 / ESP32", sizeof(line)); break;
                    case 1: snprintf(line, sizeof(line), "WiFi: %s", wifiState()); break;
                    case 2:
                        if (uint32_t(ip)) snprintf(line, sizeof(line), "IP: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
                        else strlcpy(line, "IP: --", sizeof(line));
                        break;
                    case 3:
                        if (connected) snprintf(line, sizeof(line), "RSSI: %ld dBm", long(WiFi.RSSI()));
                        else strlcpy(line, "RSSI: --", sizeof(line));
                        break;
                    case 4: snprintf(line, sizeof(line), "BLE: %s", bluetoothState()); break;
                    case 5: snprintf(line, sizeof(line), "SD: %s", sdcardMounted ? "montado" : "nao montado"); break;
                    case 6: snprintf(line, sizeof(line), "Heap: %u B", unsigned(freeHeap)); break;
                    case 7: snprintf(line, sizeof(line), "Bloco: %u B", unsigned(largest)); break;
                    default: snprintf(line, sizeof(line), "%s / %lus", !LittleFS.totalBytes() ? "ERRO FS" :
                                      (bruceConfig.configStorageOk ? "RUN" : "ERRO CONFIG"), millis() / 1000UL);
                }
                // Preserve complete values (especially IP) if the user rotates
                // to portrait. The standard 320px view always uses size 2.
                const unsigned textSize = strlen(line) * 12 > unsigned(tftWidth - 20) ? 1 : 2;
                if (first || strcmp(line, previous[row])) {
                    tft.fillRect(10, 34 + row * 19, tftWidth - 20, 18, bruceConfig.bgColor);
                    tft.setTextSize(textSize);
                    tft.setTextColor(row == 0 ? bruceConfig.priColor : DedSec::Text, bruceConfig.bgColor);
                    tft.drawString(line, 10, 34 + row * 19 + (2 - textSize) * 4, 1);
                    strlcpy(previous[row], line, sizeof(previous[row]));
                }
            }
            first = false;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    tft.setTextDatum(TL_DATUM);
    RAM_LOG("dedsec-dashboard-out");
}

void dedsecWifiSurvey() {
    RAM_LOG("dedsec-survey-in");
    if (isWebUIActive || WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
        displayWarning("Pare WebUI/scan primeiro", true);
        return;
    }
    if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_IDLE) {
        if (!confirm("Busca WiFi desliga BLE", "Encerrar e continuar")) return;
        stopBLEStack();
        if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_IDLE) {
            displayError("BLE ocupado; encerre a ferramenta", true);
            return;
        }
    }
    // One bounded allocation. Unlike Arduino scanNetworks(), this does not copy
    // every discovered AP into a second, unbounded list in the application heap.
    std::unique_ptr<wifi_ap_record_t[]> records(new (std::nothrow) wifi_ap_record_t[DedSec::MaxNetworks]);
    if (!records) {
        displayError("Sem memoria para resultados", true);
        return;
    }
    if (!radioHasMemForWifi()) {
        displayError("Heap DMA insuficiente", true);
        return;
    }
    const wifi_mode_t originalMode = WiFi.getMode();
    if (!WiFi.mode(static_cast<wifi_mode_t>(originalMode | WIFI_STA))) {
        displayError("Falha ao iniciar WiFi", true);
        return;
    }
    drawMainBorderWithTitle("WIFI / PASSIVO");
    tft.setTextSize(2);
    tft.drawString("Buscando redes...", 12, 80, 1);
    tft.drawString("ESC: cancelar", 12, 110, 1);
    surveyFinished.store(false);
    surveyStatus.store(0);
    esp_event_handler_instance_t handler;
    esp_err_t error = esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_SCAN_DONE, surveyComplete, nullptr, &handler
    );
    const bool registered = error == ESP_OK;
    wifi_scan_config_t scan = {};
    scan.show_hidden = true;
    scan.scan_type = WIFI_SCAN_TYPE_PASSIVE;
    scan.scan_time.passive = 120;
    if (registered) error = esp_wifi_scan_start(&scan, false);
    const uint32_t started = millis();
    bool cancelled = false;
    bool timedOut = false;
    while (error == ESP_OK && !surveyFinished.load()) {
        timedOut = millis() - started > 15000;
        if (check(EscPress) || returnToMenu || timedOut) {
            cancelled = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    if (cancelled) esp_wifi_scan_stop();
    if (registered) esp_event_handler_instance_unregister(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, handler);
    if (!cancelled && error == ESP_OK && surveyStatus.load() != 0) error = ESP_FAIL;
    uint16_t total = 0, captured = DedSec::MaxNetworks;
    if (!cancelled && error == ESP_OK) error = esp_wifi_scan_get_ap_num(&total);
    if (!cancelled && error == ESP_OK && total) {
        captured = min(total, static_cast<uint16_t>(DedSec::MaxNetworks));
        error = esp_wifi_scan_get_ap_records(&captured, records.get());
    } else captured = 0;
    // Always release the driver's AP list, including failure/cancellation paths.
    esp_wifi_clear_ap_list();
    if (timedOut) displayError("Timeout na busca WiFi", true);
    if (!cancelled && error == ESP_OK && captured > 0) {
        std::vector<Option> networks;
        const int shown = captured;
        networks.reserve(shown + 1);
        for (int i = 0; i < shown; ++i) {
            char label[27], ssid[16];
            readableSsid(reinterpret_cast<const char *>(records[i].ssid), ssid, sizeof(ssid));
            snprintf(label, sizeof(label), "%-15s %ddB", ssid, int(records[i].rssi));
            networks.emplace_back(label, [&records, i]() { networkInfo(records[i]); });
        }
        networks.emplace_back("Voltar", []() {});
        int selection = 0;
        do {
            char title[48];
            snprintf(title, sizeof(title), "%u redes / exibindo %d", unsigned(total), shown);
            selection = loopOptions(networks, MENU_TYPE_SUBMENU, title, selection);
        } while (!returnToMenu && selection >= 0 && selection < shown);
    } else if (!cancelled) {
        if (error == ESP_OK) displayInfo("Nenhuma rede encontrada", true);
        else {
            Serial.printf("[DedSec] WiFi survey: %s\n", esp_err_to_name(error));
            displayError("Falha na busca WiFi", true);
        }
    }
    // Restore AP/STA/off mode without disconnecting an existing station link.
    if (WiFi.getMode() != originalMode && !WiFi.mode(originalMode)) {
        displayError("Falha ao restaurar modo WiFi", true);
    }
    RAM_LOG("dedsec-survey-out");
}

bool dedsecWriteDiagnostic(Print &output) {
    const esp_partition_t *running = esp_ota_get_running_partition();
    const IPAddress ip = WiFi.isConnected() ? WiFi.localIP() : IPAddress();
    char record[768];
    const int length = snprintf(
        record, sizeof(record),
        "DedSec OS / Bruce %s\nboard=%s\nchip=%s\nrevision=%u\nuptime_ms=%lu\n"
        "wifi=%s\nip=%u.%u.%u.%u\nrssi_dbm=%ld\nrssi_valid=%u\nble_controller=%s\n"
        "sd_mounted=%u\nconfig_ok=%u\nheap_internal_free=%u\nheap_internal_largest=%u\nheap_internal_min=%u\n"
        "dma_largest=%u\nflash_detected=%u\nsketch_bytes=%u\napp_partition=%u\n"
        "littlefs_total=%u\nlittlefs_used=%u\npsram_detected=%u\n",
        BRUCE_VERSION, DEVICE_NAME, ESP.getChipModel(), ESP.getChipRevision(), millis(),
        wifiState(), ip[0], ip[1], ip[2], ip[3], WiFi.isConnected() ? long(WiFi.RSSI()) : 0L,
        WiFi.isConnected(), bluetoothState(), sdcardMounted, unsigned(bruceConfig.configStorageOk),
        unsigned(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)),
        unsigned(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)),
        unsigned(heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)),
        unsigned(radioLargestDmaBlock()), unsigned(ESP.getFlashChipSize()), unsigned(ESP.getSketchSize()),
        running ? unsigned(running->size) : 0U,
        unsigned(LittleFS.totalBytes()), unsigned(LittleFS.usedBytes()), unsigned(ESP.getPsramSize())
    );
    return length > 0 && size_t(length) < sizeof(record) &&
           output.write(reinterpret_cast<const uint8_t *>(record), length) == size_t(length);
}

void dedsecSaveDiagnostic() {
    FS *fs = sdcardMounted ? static_cast<FS *>(&SD) : static_cast<FS *>(&LittleFS);
    const bool onSD = sdcardMounted;
    if (!onSD && LittleFS.totalBytes() - LittleFS.usedBytes() < 2048) {
        displayError("LittleFS sem espaco", true);
        return;
    }
    if (!fs->exists("/dedsec") && !fs->mkdir("/dedsec")) {
        displayError("Falha ao criar /dedsec", true);
        return;
    }
    char path[64];
    unsigned sequence = 0;
    do {
        snprintf(path, sizeof(path), "/dedsec/diag-%08lx-%03u.txt", millis(), sequence++);
    } while (fs->exists(path) && sequence < 1000);
    if (fs->exists(path)) {
        displayError("Limite de nomes atingido", true);
        return;
    }
    String temporary = String(path) + ".tmp";
    File file = fs->open(temporary, FILE_WRITE);
    if (!file) {
        displayError("Falha ao abrir log", true);
        return;
    }
    bool ok = dedsecWriteDiagnostic(file);
    file.flush();
    ok = ok && !file.getWriteError() && file.size() > 0;
    file.close();
    if (!ok || !fs->rename(temporary, path)) {
        fs->remove(temporary);
        displayError("Falha ao salvar diagnostico", true);
        return;
    }
    Serial.printf("[DedSec] %s:%s\n", onSD ? "SD" : "LittleFS", path);
    displaySuccess(String(onSD ? "SD:" : "FS:") + path, true);
}

void dedsecHardwareInfo() {
    ScrollableTextArea area("CYD / HARDWARE");
    area.addLine("DedSec OS / Bruce " + String(BRUCE_VERSION));
    area.addLine("ESP32 classico / ILI9341 320x240");
    area.addLine("XPT2046 resistivo / driver Bruce");
    area.addLine("Flash detectada: " + String(ESP.getFlashChipSize()));
    area.addLine("Firmware: " + String(ESP.getSketchSize()));
    area.addLine("PSRAM detectada: " + String(ESP.getPsramSize()));
    area.addLine("WiFi 2.4 GHz / descoberta BLE");
    area.addLine("SD opcional: CS5 SCK18 MISO19 MOSI23");
    area.addLine("[EXPANSAO EXTERNA - NAO INTEGRADA]");
    area.addLine("RF: CC1101/modulo OOK; NRF: nRF24L01");
    area.addLine("RFID: PN532/RC522/ST25 configurado");
    area.addLine("IR: LED/receptor externo; GPS: UART");
    area.addLine("LoRa: transceptor; Ethernet: W5500");
    area.addLine("Grove: SDA27/SCL22 conflita com CS/IO RF.");
    area.addLine("SPI de expansao compartilha SD.");
    area.addLine("GPS TX3/RX1 compartilha serial USB.");
    area.addLine("FM RST40 da base e invalido no ESP32.");
    area.addLine("USB HID nativo nao existe nesta CYD.");
    area.addLine("USB HID externo: CH9329 UART TX27/RX22.");
    area.addLine("iButton: interface 1-Wire externa.");
    area.addLine("Verifique revisao, alimentacao e pinos antes de conectar modulos.");
    area.addLine("SEL: voltar");
    area.show();
}

void dedsecSettings() {
    int selected = 0;
    do {
        std::vector<Option> items = {
            {bruceConfig.instantBoot ? "Animacao boot: OFF" : "Animacao boot: ON", []() {
                bruceConfig.instantBoot = !bruceConfig.instantBoot;
                bruceConfig.saveFile();
            }},
            {"Duracao boot: " + String(bruceConfig.dedsecBootDuration / 1000) + " s", []() {
                const int current = bruceConfig.dedsecBootDuration;
                bruceConfig.dedsecBootDuration = current == 4000 ? 7000 : (current == 7000 ? 1000 : 4000);
                bruceConfig.saveFile();
            }},
            {bruceConfig.dedsecWallpaper ? "Fundo: imagem DedSec" :
                (bruceConfig.bgColor == 0 ? "Fundo: preto" : "Fundo: cor do tema"), []() {
                bruceConfig.dedsecWallpaper = !bruceConfig.dedsecWallpaper;
                bruceConfig.saveFile();
            }},
            {"Preto e branco", []() {
                if (bruceConfig.priColor == DedSec::Accent && bruceConfig.secColor == DedSec::Muted &&
                    bruceConfig.bgColor == DedSec::Background && bruceConfig.themePath.isEmpty()) return;
                bruceConfig.removeTheme();
                bruceConfig.themePath = "";
                uint16_t secondary = DedSec::Muted, background = DedSec::Background;
                bruceConfig.setUiColor(DedSec::Accent, &secondary, &background);
            }},
            {"Voltar", []() {}}
        };
        selected = loopOptions(items, MENU_TYPE_SUBMENU, "DEDSEC / UI", selected);
        if (!bruceConfig.configStorageOk) displayError("Config nao salva; veja serial", true);
    } while (!returnToMenu && selected >= 0 && selected < 4);
}

const char *dedsecHardwareRequirement(const String &name) {
    if (name == "RF") return "EXT: CC1101 / modulo RF";
    if (name == "NRF24") return "EXT: nRF24L01";
    if (name == "RFID") return "EXT: leitor RFID/NFC";
    if (name == "IR") return "EXT: LED / receptor IR";
    if (name == "GPS") return "EXT: GPS UART";
    if (name == "FM") return "EXT: Si4713 / rever GPIO";
    if (name == "LoRa") return "EXT: transceptor LoRa";
    if (name == "Ethernet") return "EXT: W5500";
    return nullptr;
}

bool dedsecConfirmMenuOperation(const String &label, bool hardwareEntry) {
    // Only the actual main-menu entry checks expansion hardware. A settings
    // checkbox with the same label (Hide Apps) must not trigger this prompt.
    if (hardwareEntry) {
        if (const char *hardware = dedsecHardwareRequirement(label)) {
            return confirm(hardware, "Hardware configurado");
        }
        return true;
    }
    // Resource transitions only. Tool callbacks and target selection follow
    // Bruce directly, without intercepting generic menu labels.
    if (label == "BLE radio switch") return confirm("Reinicia BLE; desliga WiFi/WebUI", "Continuar");
    if (label == "BLE radio reset") return confirm("Reinicia BLE e suas conexoes", "Encerrar e continuar");
    return true;
}
#endif
