# DedSec OS — CYD ESP32-2432S028 resistiva

O webflasher oferece duas edições: **Classic**, com o carrossel anterior, e
**Grid**, com navegação direta em cartões e listas. Detalhes da nova edição,
compilação e verificações em [GRID.md](GRID.md). As duas usam a mesma CYD.

Personalização do Bruce oficial, sem substituir drivers, navegação, bibliotecas
ou mapa de partições. Base e análise em [AUDITORIA.md](AUDITORIA.md).
Resultados reais e limitações em [VALIDACAO.md](VALIDACAO.md).
Ícones aprovados e medições em [ICONES.md](ICONES.md); revisão atual com fundo
alternativo e duração de boot em [FUNDO.md](FUNDO.md).

## Preparar e compilar no Windows

Python 3.10–3.14 e Git devem estar disponíveis no PATH. Execute na raiz deste
repositório. O diretório pode ter espaços. A primeira compilação baixa toolchains
e bibliotecas e precisa de rede e vários GB livres.

```powershell
python -m venv --without-pip .venv
python -m pip --python .venv install platformio==6.2.0
./tools/dedsec/build.ps1
```

Se a política local impedir scripts PowerShell, use diretamente:

```powershell
$env:PLATFORMIO_CORE_DIR = Join-Path (Get-Location) '.pio-core'
./.venv/Scripts/python.exe -m platformio run -e DedSec-CYD-2432S028
./.venv/Scripts/python.exe tools/dedsec/verify_image.py --output docs/dedsec/DedSec-CYD-2432S028-image.json
```

O ambiente `DedSec-CYD-2432S028` herda `CYD-2432S028`. Não use CYD-2USB,
CYD-W328, S3 ou LAUNCHER para esta placa. O ambiente padrão do Bruce foi
preservado: sempre informe `-e` ou use o script acima.

Saídas esperadas após sucesso confirmado:

* `.pio/build/DedSec-CYD-2432S028/firmware.bin`: somente a aplicação.
* `Bruce-DedSec-CYD-2432S028.bin`: imagem completa produzida pelo `build.py` do Bruce.
* `docs/dedsec/DedSec-CYD-2432S028-image.json`: tamanhos, partição e SHA-256
  verificados pelo script; só existe após a validação real.

`verify_image.py` confere o limite factory de 0x3C0000, o limite da flash e a
posição exata da aplicação, bootloader e tabela na imagem completa. Uma
compilação malsucedida ou uma imagem rejeitada não é candidata a gravação.

## Gravar

### Pelo navegador (webflasher)

Abra [DedSec OS Webflasher](https://enzogabrielS.github.io/DedSec-OS-Bruce/)
no Chrome ou Edge do computador. Conecte a CYD com cabo USB de dados, feche
monitores seriais e retire módulos externos. Confirme o modelo exato e a
substituição da firmware; clique em **Conectar e instalar**, escolha a porta da
CYD e use **Install DedSec OS** no diálogo. Na primeira instalação, faça backup
antes e selecione **Erase device** para limpar dados antigos. Essa opção apaga
toda a flash, incluindo configurações e arquivos internos. O cartão SD não é
apagado, mas retire-o se deseja evitar que configurações antigas sejam lidas.

Na atualização de uma DedSec anterior, desmarcar `Erase device` preserva o
LittleFS, incluindo `/bruce.conf`, pois a imagem termina antes dessa partição.
A imagem completa ainda regrava o prefixo da flash, incluindo a região NVS.

A página confere tamanho, cabeçalhos e SHA-256; o instalador recebe os mesmos
bytes verificados por uma URL Blob local. O manifesto aceita apenas o ESP32
clássico e a imagem completa no endereço 0x0. O chip não identifica o modelo do
display, a revisão da CYD nem a capacidade de flash: confira sua placa antes.
O webflasher não faz backup da flash nem valida o funcionamento físico.
ESP Web Tools 10.4.0 é carregado do unpkg; é preciso conexão à Internet.

Mantenha o cabo conectado até a conclusão e aguarde reiniciar. Se necessário,
pressione RST/EN. Se não conectar e houver os botões correspondentes na placa,
segure BOOT/FLASH, pressione e solte RST/EN e mantenha BOOT durante a conexão;
solte-o ao iniciar a gravação. Se não aparecer porta, teste outro cabo de dados
e confira o chip USB serial e o driver no Gerenciador de Dispositivos.

### Pelo PlatformIO / esptool (backup e gravação manual)

Identifique a porta USB serial da CYD com `platformio device list`. `COMx` abaixo
é um marcador; não escolha automaticamente COM1. Feche monitores seriais antes
da gravação. Confirme a revisão da placa e a flash instalada.

```powershell
$env:PLATFORMIO_CORE_DIR = Join-Path (Get-Location) '.pio-core'
./.venv/Scripts/python.exe -m platformio run -e DedSec-CYD-2432S028 -t upload --upload-port COMx
./.venv/Scripts/python.exe -m platformio device monitor --port COMx --baud 115200
```

Para backup/identificação com a versão do esptool instalada pelo PlatformIO,
use o runtime `.pio-core/penv` criado pelo pioarduino durante a compilação:

```powershell
./.pio-core/penv/Scripts/python.exe -m platformio pkg exec -p tool-esptoolpy -- esptool.py --chip esp32 --port COMx flash-id
./.pio-core/penv/Scripts/python.exe -m platformio pkg exec -p tool-esptoolpy -- esptool.py --chip esp32 --port COMx read-flash 0x0 0x400000 backup-cyd-4mb.bin
```

O tamanho do backup acima pressupõe 4 MiB confirmados. Para uma imagem completa
validada, o comando equivalente de gravação é:

```powershell
./.pio-core/penv/Scripts/python.exe -m platformio pkg exec -p tool-esptoolpy -- esptool.py --chip esp32 --port COMx write-flash 0x0 Bruce-DedSec-CYD-2432S028.bin
```

Não grave o `firmware.bin` isolado em 0x0: seu endereço é 0x10000. A partição
factory ocupa 3,75 MiB e não há segundo slot OTA; use a conexão serial.

## Uso

* O menu principal do Bruce abre depois da animação DedSec de até 720 ms.
  A logo do anexo aparece desde o primeiro acesso ao display, substituindo
  a tela técnica de montagem de arquivos. As etapas reais vão para a serial.
  Durante operações bloqueantes de armazenamento, a logo permanece visível;
  a animação curta roda no loop depois da inicialização, sem uma tarefa extra.
  Toque para pular; desative em `Config > Display & UI > DedSec UI`.
  O painel em `Config > Live dashboard` mostra modelo, Wi-Fi, IP, RSSI, controlador BLE,
  último estado de montagem SD, heap interno livre, maior bloco e estado do FS/config.
  Toque para voltar ao menu. A aba adicional System foi removida.
* Os ícones padrão de WiFi, Bluetooth e Config usam os respectivos anexos
  otimizados. Arquivos de tema personalizados continuam tendo prioridade.
* `WiFi > WiFi survey`: busca passiva sem associação a APs, até 24 registros;
  SSID, BSSID, canal, RSSI e autenticação. ESC cancela. O modo Wi-Fi anterior
  é restaurado; a lista temporária é liberada.
* `Bluetooth > BLE Scan`: descoberta BLE reutiliza `ble_common`. Não é uma
  busca de dispositivos Bluetooth Classic. A troca de rádio em placa sem PSRAM
  pode desconectar Wi-Fi e encerrar WebUI, com confirmação na descoberta.
  A busca é passiva, dura até cinco segundos e mantém no máximo 24 anúncios;
  ESC cancela. O controlador é encerrado antes de navegar pelos resultados copiados.
* `Config > Save diagnostic`: salva snapshot textual em `/dedsec/` no SD
  montado ou no LittleFS; mostra erro real de abertura/escrita/rename.
  Os arquivos podem ser lidos e exportados pelo Files/WebUI original.
* `Config > Hardware / expansion`: requisitos e conflitos dos módulos externos.
* `Config > Display & UI > DedSec UI`: animação de boot e paleta preto e branco. Brilho,
  temas, orientação e conectividade continuam no Config original.
* Configuração usa `/bruce.conf` e o JSON original. No perfil DedSec o arquivo
  interno tem prioridade sobre o espelho SD; gravação temporária + rename evita
  substituir a configuração por um arquivo parcialmente serializado. Erros
  aparecem na serial e no painel. Não se grava durante o refresh do painel.
  JSON com raiz inválida, erro de leitura ou mais de 32 KiB mantém os defaults
  e sinaliza erro. Cores inválidas são rejeitadas sem passar ponteiros nulos a strtoul.
* A paleta padrão é preto, branco e cinzas. A paleta vermelha padrão da versão
  DedSec anterior migra uma vez para branco. Temas/paletas personalizados são
  respeitados; selecione `Preto e branco` em DedSec UI para restaurar o padrão.
* PREV/SEL/NEXT e a zona ESC seguem o mapa de toque do Bruce. ESC fica no
  terço superior esquerdo; SEL no centro. Não há uma segunda pilha gráfica.

As ferramentas herdadas usam o fluxo original do Bruce, incluindo seus
seletores de rede/dispositivo quando disponíveis. A confirmação extra
`Alvo proprio/autorizado` foi removida. Permanecem as confirmações de hardware,
troca de rádio e as operações destrutivas que já pertenciam ao Bruce. Use apenas
alvos próprios/autorizados. Nenhuma ferramenta ofensiva nova foi acrescentada.

## Expansão

RF, NRF24, RFID/NFC, IR, GPS, LoRa e Ethernet requerem hardware externo. O rótulo
EXT descreve uma exigência, não uma detecção automática de presença. As ferramentas
originais permanecem acessíveis para módulos corretamente configurados.

Não há bateria monitorada, NFC, GPS, PSRAM nem USB HID nativo presumidos.
Grove27/22 conflita com controles de expansão RF; SPI18/19/23 compartilha SD;
GPS3/1 conflita com UART0/USB serial. O preset FM herdado usa GPIO40 inexistente
no ESP32 clássico: o perfil bloqueia sua abertura até configurar um RST válido.
Não foi alterado nenhum pino para mascarar esse problema da base.

USB HID no ESP32 clássico pode usar o CH9329 externo já suportado pelo Bruce
(UART TX27/RX22 no preset da CYD), com conflito com os pinos Grove. iButton
requer interface 1-Wire externa. Esses recursos não são periféricos integrados.

As versões efetivamente resolvidas estão em `dependencies-resolved.json`.
O perfil herda as dependências oficiais; esse inventário é um registro da
compilação, não um lockfile aplicado automaticamente às dependências upstream
que usam branches ou intervalos de versão.

## Recuperação / reinstalar Bruce

1. Desconecte módulos externos. Ligue apenas a CYD pelo USB de dados.
2. Se a conexão automática falhar, mantenha BOOT pressionado, pressione RESET,
   solte RESET e depois BOOT para entrar no bootloader ROM do ESP32.
3. Restaure o backup integral confirmado com `write-flash 0x0 backup-cyd-4mb.bin`,
   ou use o [flasher oficial do Bruce](https://bruce.computer/flasher) escolhendo
   exatamente CYD-2432S028.
4. Para compilar a base sem o perfil DedSec neste checkout:

   ```powershell
   ./tools/dedsec/build.ps1 -Environment CYD-2432S028
   ```

   Esse ambiente exclui as alterações condicionais DedSec; o reparo de build
   portátil em `patch.py` permanece. Para código estritamente upstream, clone
   separadamente o repositório oficial e selecione o commit registrado na auditoria.
5. Para uma reinstalação limpa, faça backup do SD e das configurações antes de
   usar `erase-flash`; esse comando apaga NVS, calibrações e LittleFS. Não é
   necessário apagar toda a flash para cada atualização.

O bootloader ROM normalmente permite recuperação mesmo com uma aplicação que
não inicializa. Falhas elétricas, flash danificada e revisões incompatíveis
exigem investigação física.

## Validação física pendente

Após conferir o resultado de compilação no relatório, validar na placa:

1. Boot sem SD/expansões, cores e inversão, legibilidade, quatro orientações e
   toque nas bordas/centro; pressionar, segurar e soltar sem disparo repetido.
2. Brilho/boot/cores persistem depois de reiniciar; configuração ausente,
   JSON corrompido, LittleFS cheio e SD removido produzem defaults/erros úteis.
3. Busca Wi-Fi e BLE com zero, um e muitos resultados; cancelar e repetir;
   confirmar retorno ao menu, liberação de buffers e troca de rádio.
4. Salvar e abrir snapshots no Files, exportar por WebUI e comparar conteúdo
   com valores reais. Remover SD durante gravação e verificar erro/reinicialização.
5. Repetir 100 ciclos painel/menu/busca alternada e manter a placa por pelo menos
   uma hora. Comparar `[RAMLOG]` e snapshots (heap, mínimo e maior bloco) após
   cada stack ter sido encerrado. Não confundir consumo de stack ativo com vazamento.
6. Confirmar tamanho físico da flash antes de gravar e testar a recuperação ROM.

Esses procedimentos são instruções de teste, não resultados realizados.

## Licenças

Bruce e contribuições preservam [LICENSE](../../LICENSE) e
[THIRD_PARTY.md](../../THIRD_PARTY.md). DedSec OS é uma interpretação gráfica
original para estudo pessoal; não contém telas extraídas do jogo.
