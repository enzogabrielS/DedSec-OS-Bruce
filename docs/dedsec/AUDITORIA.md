# Auditoria inicial — DedSec OS / CYD-2432S028

Base: https://github.com/BruceDevices/firmware, commit
`a59213f3ec6cd302fa32c2042b2700c9c24d991a` (clone de main em 24/10/2026).
O código original, LICENSE (AGPL-3.0) e THIRD_PARTY.md são preservados.

## Componentes examinados antes de alterar a firmware

| Área | Implementação reutilizável |
| --- | --- |
| Entrada | `src/main.cpp`: setup GPIO, TFT, armazenamento/config, relógio, input task, serial e MainMenu |
| Navegação | `MainMenu`, `MenuItemInterface`, `loopOptions`, `ScrollableTextArea`; sem LVGL ou novo framework |
| Display | TFT_eSPI local, ILI9341_2_DRIVER, HSPI, rotação 3; framebuffer completo não é necessário |
| Touch | interface da CYD + CYD28_TouchR/XPT2046 por software; preservar mapeamento de zonas Bruce |
| Config | BruceConfig/BruceTheme, JSON `/bruce.conf`; BruceConfigPins por MAC, LittleFS e cópia SD |
| Wi-Fi | WiFi Arduino/esp_wifi, wifi_common, AP info, channel analyzer, scan hosts, TCP e CLI |
| Bluetooth | NimBLE-Arduino 2.5, ble_common, gestão de memória/teardown; BLE, não descoberta Bluetooth Classic |
| Arquivos | sd_functions, FileMenu, LittleFS, SD, WebUI para exportação |
| Compilação | PlatformIO, pioarduino 55.03.39, Arduino 3.3.9, libs Arduino custom 20260715, GCC Xtensa, LTO |
| Scripts | patch.py, pre_build_current_year, gen_mqjs_headers (requer compilador host 32 bits), library conflicts, flto_prep, build.py |

## Configuração real

Origem: `boards/CYD-2432S028/CYD-2432S028.ini`, JSON da placa, interface.cpp
e `lib/CYD-touch/CYD28_TouchscreenR.h`.

* ESP32 clássico, CPU 240 MHz, flash declarada 4 MiB DIO/40 MHz, sem BOARD_HAS_PSRAM.
* TFT físico 240×320, rotação 3 → paisagem 320×240; área Bruce 320×220 + rodapé de 20 px.
* TFT: MISO12, MOSI13, SCLK14, CS15, DC2, RST=-1, BL21; SPI40 MHz, leitura16 MHz.
* Touch: MOSI32, MISO39, CLK25, CS33, IRQ36; calibração fixa X185..3700/Y280..3850.
  SPI_TOUCH_FREQUENCY=2,5 MHz não muda o touch por software desta variante.
* SD: CS5, SCK18, MISO19, MOSI23; barramento compartilhável com expansão SPI.
* Grove I2C: SDA27/SCL22. CC1101, nRF24 e W5500 usam CS27 e controle22 por padrão:
  são alternativas de expansão, não podem ser presumidos simultâneos ou integrados.
* GPS: flags TX3/RX1; interface pós-init aplica esses valores; conflita com UART0/USB serial.
* FM_SI4713 aparece habilitado com RST40 (GPIO inexistente no ESP32 clássico): problema
  da configuração original, não representa um rádio FM instalado na CYD.
* Nenhum sensor de bateria, NFC, GPS, transmissor RF, IR ou PSRAM integrado é presumido.
* `custom_4Mb_full.csv`: NVS0x9000/0x6000; factory0x10000/0x3C0000
  (3.932.160 bytes); LittleFS0x3D0000/0x30000 (196.608 bytes). Sem segundo slot OTA.
* JSON anuncia maximum_size=4 MiB; a partição real de aplicação é menor. Validar
  explicitamente o binário contra 0x3C0000 antes de gravar.

## Limitações/problemas da base observados por leitura

* Splash original espera até sete segundos com polling; será substituído somente no perfil DedSec.
* Rodapé TouchFooter desenha 43 px em uma reserva de 20 px: fica cortado nesta tela.
* `showDeviceInfo()` imprime carga de bateria mesmo sem ADC configurado; omitir no perfil DedSec.
* `BruceConfig::saveFile()` original não é transacional, imprime JSON completo na serial
  e retorna void. As novas opções devem usar esse mecanismo, reduzindo gravações e
  melhorando segurança contra gravação incompleta de forma localizada.
* `ble_scan()` original bloqueia durante getResults e oferece ações além de observação em
  outros menus; nova navegação deve distinguir diagnósticos e pedir confirmação para efeitos ativos.
* Os guardas de memória e teardown Wi-Fi/BLE já existem, mas FORCE_RADIO_TEARDOWN_ON_SWITCH
  está desativado na base examinada. O perfil DedSec deverá habilitar a política
  para evitar manter dois stacks ativos por padrão na placa sem PSRAM.
* Flash instalada, cores/inversão, revisão/calibração, touch, SD e estabilidade só podem
  ser confirmados na placa física. Nenhum teste físico foi realizado nesta auditoria.

## Superfície planejada de alteração

Perfil PlatformIO adicional que herda CYD-2432S028; paleta central; módulo de diagnóstico
sob src/modules/others; item de menu que usa MenuItemInterface; integrações condicionais
em main, config, theme, settings, display e utils. Drivers, pinos e partições originais
permanecem a referência. Não adicionar dependências gráficas.

## Compilação original

Log bruto: `baseline-build.log`. Primeiro ensaio foi impedido pela rede do sandbox.
A retentativa autorizada executou o ambiente original, com DEDSEC_OS ausente.
O script upstream patch.py falhou porque `pio` não estava no PATH e o caminho
do workspace contém espaços. O script renomeou libnet80211.a para .old mesmo
sem produzir o patch e criou o marcador de sucesso. A biblioteca foi recuperada
com o objcopy instalado, executando exatamente a operação prevista pelo Bruce.

Após essa correção do ambiente gerado, a compilação foi concluída:
SUCCESS em 1.496,61 s; aplicação 3.584.496 bytes; imagem completa 3.650.032 bytes;
RAM estática PIO 92.256 bytes. A verificação independente dos offsets e do limite
factory passou; detalhes e hashes em `CYD-2432S028-image.json`.

As adições de fonte DedSec criadas durante o download estão integralmente sob
`#ifdef DEDSEC_OS`; portanto não participaram do código executável da referência.
O reparo permanente do script só será usado nas invocações posteriores.

Avisos herdados observados: redefinições PNGdec/AnimatedGIF; diretivas #undef
OneWire; APIs BLE obsoletas; tipo de retorno NimBLE com LTO; nomes de callbacks
BLE de expansões que violam ODR; valores 180000 truncados em uint16_t na ferramenta
Karma; eventos WebSocket não tratados no módulo reverseShell. Não se alteraram
esses mecanismos de teste ofensivo para desenvolver a interface.
