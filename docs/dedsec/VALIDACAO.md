# Relatório de entrega — DedSec OS / CYD-2432S028

Data: 24/10/2026. Código e imagens compilados; validação física pendente.
Projeto local: `C:\Users\enzog\Documents\New project\dedsec-os`.

## 1. Resumo

Personalização implementada diretamente sobre o [Bruce oficial](https://github.com/BruceDevices/firmware),
commit `a59213f3ec6cd302fa32c2042b2700c9c24d991a`. Foram adicionados tema DedSec,
boot por etapas reais, painel, observação Wi-Fi e ajustes de descoberta BLE,
persistência, tratamento de falhas e interação. A estrutura, as ferramentas,
os drivers e as dependências do Bruce foram reutilizados. LICENSE e THIRD_PARTY.md
foram preservados; nenhum recurso visual do jogo foi extraído.

## 2. Arquivos

Lista completa dos arquivos de fonte, configuração, ferramentas e documentação
alterados/criados: [FILES.txt](FILES.txt).

Principais integrações: `src/main.cpp`, `src/core/main_menu.*`, `display.cpp`,
`config.*`, `theme.*`, `menu_items/ConfigMenu.cpp`, `menu_items/FMMenu.cpp`,
`scrollableTextArea.cpp`, `settings.cpp`, `utils.cpp`, `modules/ble/ble_common.*`,
`modules/wifi/ap_info.h` e `boards/CYD-2432S028/interface.cpp`.

Adições principais: `dedsec_palette.h`, `menu_items/DedSecMenu.*`,
`modules/others/dedsec.*`, `boards/CYD-2432S028/DedSec-CYD-2432S028.ini`,
`tools/dedsec/` e esta documentação. `patch.py` recebeu reparo de execução
portátil e preservação da biblioteca quando o patch falha.

Artefatos gerados:

* `Bruce-DedSec-CYD-2432S028.bin`: imagem completa final, endereço 0x0.
* `.pio/build/DedSec-CYD-2432S028/firmware.bin`: aplicação, endereço 0x10000.
* `Bruce-CYD-2432S028.bin`: imagem da referência Bruce compilada nesta sessão.
* `DedSec-CYD-2432S028-image.json`: tamanhos e SHA-256 finais.
* `build-evidence.zip`: logs reais das compilações e da inspeção esptool.

`.pio`, `.pio-core` e `.venv` contêm dependências/saídas de desenvolvimento,
não alterações na arquitetura de fontes.

## 3. Integração com Bruce

O novo ambiente herda `CYD-2432S028` e acrescenta `DEDSEC_OS`. As personalizações
são condicionais a esse perfil. `DedSecMenu` implementa `MenuItemInterface` e
usa `MainMenu`, `loopOptions`, `ScrollableTextArea`, TFT_eSPI, BruceConfig,
LittleFS/SD e os serviços de rádio existentes. A observação Wi-Fi usa a API
esp_wifi já disponível, com callback de conclusão e buffer limitado.

Os pinos, a calibração do driver, o ambiente original e `custom_4Mb_full.csv`
permanecem como referência. Não foi acrescentada biblioteca nem tarefa FreeRTOS.
O reparo comum do construtor de texto inicializa um booleano anteriormente
indeterminado; o reparo de build evita depender do PATH e de caminhos sem espaços.

## 4. Funcionalidades implementadas no código

* Paleta RGB565 centralizada, terminal geométrico original, textos monoespaçados,
  mensagens com cores consistentes e rodapé corrigido para a reserva de 20 px.
* Boot com quatro etapas vinculadas ao setup, sem a espera decorativa original
  de sete segundos; opção de boot reduzido usa a persistência existente.
* Painel inicial e `System`: modelo, Wi-Fi, IP/RSSI válidos quando disponíveis,
  controlador BLE, montagem SD, heap interno livre, maior bloco e saúde FS/config.
  Refresh de 1 Hz por linhas alteradas; toque retorna ao menu.
* Busca Wi-Fi passiva: SSID, BSSID, canal, RSSI, autenticação e indicação de
  OPEN/WEP; até 24 registros; cancelamento, timeout, status real de conclusão,
  liberação da lista do SDK e restauração do modo Wi-Fi anterior.
* Descoberta BLE reutilizada: passiva, até 24 anúncios, cancelável, com detalhes
  de nome/endereço/RSSI. Os resultados copiados podem ser consultados após
  encerrar o controlador. Uma indicação de limite é desabilitada para seleção.
* Snapshot de diagnóstico real em `/dedsec/`, no SD montado ou no LittleFS,
  com heap mínimo, maior bloco DMA, flash detectada, tamanho de aplicação e FS.
  Abertura, escrita, flush e rename são verificados. Exportação usa Files/WebUI.
* Persistência com arquivo temporário, conferência de tamanho e rename; erro
  visível; limite de configuração de 32 KiB e validação dos tipos/valores das cores.
  O arquivo interno tem prioridade sobre um espelho SD antigo. Não há gravações
  persistentes no refresh do painel.
* Feedback de seleção, tratamento de toque segurado, retorno/ESC e confirmações
  nas entradas identificadas de ferramentas ativas, com efeito esperado.
* Ferramentas originais de conectividade, análise de protocolos, portas/hosts,
  console, arquivos e expansões permanecem na arquitetura Bruce.

Essas implementações foram compiladas. Execução na placa não foi verificada.

## 5. Hardware adicional

RF: CC1101 ou módulos OOK; NRF24: nRF24L01; RFID/NFC: leitor configurado;
IR: LED/receptor; GPS: UART; LoRa: transceptor; Ethernet: W5500; FM: Si4713;
USB HID externo: CH9329 UART; iButton: interface 1-Wire.

As categorias de expansão indicam EXT, sem presumir presença. Grove27/22,
CS/controle dos rádios, SD/SPI18/19/23 e UART0/GPS3/1 têm conflitos documentados.
O preset FM original usa RST40, inexistente no ESP32 clássico; sua entrada
é bloqueada até haver GPIO de saída válido configurado. Não se presume PSRAM,
bateria monitorada, NFC, GPS ou USB HID nativo integrados.

## 6. Resultado real das compilações

| Etapa | Resultado | Aplicação em bytes | RAM estática PIO |
| --- | --- | ---: | ---: |
| Referência CYD, após recuperar biblioteca do script original | SUCCESS | 3.584.496 | 92.256 |
| Tema/configuração | SUCCESS | 3.580.432 | 92.280 |
| Painel e integração de menus | SUCCESS | 3.594.496 | 92.312 |
| BLE, touch, confirmações e falhas | SUCCESS | 3.599.568 | 92.352 |
| Otimização e validação final pelo script entregue | SUCCESS | **3.599.680** | **92.360** |

A primeira tentativa DedSec detectou uma inclusão ausente para `sdcardMounted`;
foi corrigida. Os três avisos de printf introduzidos no diagnóstico também foram
corrigidos. A compilação final durou 253,73 s, usando caches e o script
`tools/dedsec/build.ps1`; não é uma estimativa do tempo da primeira instalação.
Os logs brutos registram também as tentativas intermediárias.

## 7. Flash e memória

* Partição factory: **3.932.160 bytes** (0x3C0000).
* Aplicação final: **3.599.680 bytes**, 91,54% da partição; **332.480 bytes livres**.
* Imagem completa: **3.665.216 bytes**; cabe antes do LittleFS em 0x3D0000.
* RAM estática PIO: **92.360 / 327.680 bytes**, acréscimo de **104 bytes**
  sobre a referência. O acréscimo de aplicação é **15.184 bytes**.
* IRAM informada pelo linker: **113.268 / 131.072 bytes**.
* Cache explícito de texto do painel: 243 bytes + linha de 27 bytes na stack,
  sem framebuffer de 153.600 bytes para 320×240 RGB565.
* Menu System reaproveitado durante a sessão; listas de busca limitadas e
  capacidade BLE reservada para resultados e controles antes de preenchê-la.

RAM estática não é heap livre. Heap em execução, fragmentação, consumo durante
buscas e vazamentos prolongados **não foram medidos fisicamente**. O painel,
snapshots e RAMLOG permitem essa medição depois da gravação.

SHA-256 da imagem completa:
`aed760b6101061b536434a90efddd714e6e143b072b06cdad1a57735ec9f71f9`.
O JSON final contém também o hash da aplicação isolada.

## 8. Testes executados

| Verificação no ambiente de desenvolvimento | Resultado |
| --- | --- |
| Compilação do ambiente original e quatro estágios DedSec | Passou após as correções registradas |
| Verificador independente: limite factory, posições de app/bootloader/tabela e imagem completa | Passou em todos os estágios compilados |
| Seis testes host do verificador: imagem válida, app excedente, endereço errado, bootloader ausente, invasão do FS e magic inválido | 6/6 passaram |
| esptool image-info offline | ESP32, 4 MB, DIO, 40 MHz; checksum e validation hash válidos |
| Verificação sintática dos scripts Python e git diff --check | Passou |
| Novos avisos de compilação em dedsec.cpp e ble_common.cpp no build final | Nenhum observado |

O esptool manual deve usar o Python `.pio-core/penv` criado pelo pioarduino.
A chamada por `.venv` não encontrou rich_click; as instruções foram corrigidas
e a inspeção offline passou no runtime correto.

## 9. Limitações e validação física pendente

Nenhuma CYD foi identificada nas portas disponíveis; apenas COM1 ACPI apareceu.
Não houve gravação, teste de boot, display, touch, SD físico, rádio no ambiente,
periféricos externos, interrupção de energia nem teste prolongado na placa.
Esses critérios de aceitação permanecem pendentes, com roteiro no README.

BLE significa anúncios BLE, sem descoberta BR/EDR Classic. A busca passiva
pode não obter nomes presentes apenas em scan responses. As listas limitam
a apresentação/retenção a 24 registros. SD mostra o estado de montagem, sem
inventar um sensor de presença; uma remoção pode ser percebida só no próximo I/O.

Continuam avisos da base em OneWire, PNGdec/AnimatedGIF, APIs BLE obsoletas,
tipos NimBLE/LTO e callbacks ODR de módulos externos. A auditoria registra os
avisos e outras limitações herdadas. Não há garantia de funcionamento de cada
ferramenta avançada ou módulo externo sem teste de hardware correspondente.

Confirmações são aplicadas às entradas identificadas da interface. CLI, scripts
e APIs originais preservam seu comportamento; elas não são um sandbox de segurança.
As dependências upstream que usam branches/intervalos podem mudar; o inventário
`dependencies-resolved.json` registra as versões desta compilação.

## 10. Compilação e gravação reproduzíveis

As instruções completas estão em [README.md](README.md), incluindo criação da
venv, PlatformIO 6.2.0, seleção explícita de ambiente, backup e identificação
da flash física. Com o ambiente preparado, executar na raiz:

```powershell
./tools/dedsec/build.ps1
```

O procedimento foi executado com sucesso. Para gravar a imagem completa validada,
identifique primeiro a porta da CYD; `COMx` é apenas um marcador:

```powershell
$env:PLATFORMIO_CORE_DIR = Join-Path (Get-Location) '.pio-core'
./.pio-core/penv/Scripts/python.exe -m platformio pkg exec -p tool-esptoolpy -- esptool.py --chip esp32 --port COMx write-flash 0x0 Bruce-DedSec-CYD-2432S028.bin
```

O comando de gravação acima foi documentado, **não executado**. A aplicação
isolada não deve ser colocada em 0x0. O esquema usa factory, sem segundo slot OTA.

## 11. Recuperar ou reinstalar Bruce

O [README](README.md#recuperação--reinstalar-bruce) descreve BOOT/RESET para
entrar no bootloader ROM, restauração de backup integral e uso do flasher
oficial para CYD-2432S028. A imagem `Bruce-CYD-2432S028.bin` compilada na sessão
também está disponível na raiz. A opção `build.ps1 -Environment CYD-2432S028`
compila sem as personalizações condicionais DedSec. Apagar flash remove NVS e
LittleFS; preservar backup antes de uma reinstalação limpa.
