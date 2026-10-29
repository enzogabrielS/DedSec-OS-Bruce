# DedSec OS Grid — 2026.10.29-grid.1

Segunda edição da firmware baseada no mesmo Bruce e no mesmo hardware CYD
ESP32-2432S028 padrão. A edição anterior continua no instalador como
**DedSec OS Classic**, com o binário bw.4 preservado.

## Interface implementada

* Menu principal: seis módulos por página, cartões de 152 × 48 px na
  orientação padrão de 320 × 240. Títulos principais de 16 px.
* Submenus: três opções por página, seleção por toque direto e marcadores
  de opções selecionadas/desabilitadas.
* Controles inferiores com 40 px de altura: página anterior, abrir/voltar,
  próxima página. Teclas, controle serial e índice de retorno continuam
  passando pelo `loopOptions` original.
* Preto, branco e destaques vermelhos; ícones DedSec já aprovados e identidade
  Grid no boot e na barra de estado.
* Heap livre real atualizado em uma pequena região a cada segundo. Wi-Fi
  continua usando a informação real do Bruce; sem bateria ou sensores fictícios.
* Fundo alternativo e duração do boot continuam nas configurações DedSec.
* Temas externos do Bruce têm prioridade: ao usar um tema externo, o menu
  principal usa o renderizador original. Para ver a grade, restaure
  `Config > Display & UI > DedSec UI > Preto e branco`.
* Menus com callbacks próprios de prévia e menus regulares internos mantêm
  a apresentação nativa. Isso preserva recursos como prévias de brilho/cor.

A edição Grid modifica a apresentação e o tratamento dos toques nas áreas
dos cartões. Não substitui módulos por versões simuladas. As ações usam os
mesmos `Option.operation`, categorias, filtros de menus ocultos, serviços,
drivers, armazenamento e inicialização da base existente. A proteção contra
toque mantido e os 600 ms iniciais de seleção do Bruce são preservados.

As 15 categorias continuam presentes conforme os flags originais: WiFi,
BLE, RF, NRF24, LoRa, FM, IR, Ethernet, GPS, RFID, arquivos, scripts, relógio,
outros e configuração. Não foram ativados LITE ou DISABLE_INTERPRETER.
Os requisitos de hardware permanecem: RF/NRF24/LoRa/FM/IR/Ethernet/GPS/RFID
dependem dos periféricos correspondentes. O firmware não cria esses recursos
na CYD nem considera o ícone prova de hardware presente.

## Duas escolhas no webflasher

| Edição | Versão | Imagem completa | SHA-256 |
| --- | --- | ---: | --- |
| Classic | 2026.10.25-bw.4 | 3.765.520 B | fa4f837f30bbe2e48ad185fa91487c970ad87c6313e41e03a7d03a58a382b16b |
| Grid | 2026.10.29-grid.1 | 3.767.568 B | 59660dfd6cab795a236f804e98017454a76779d1f623067883b6ba4fed35637c |

O seletor atualiza nome, descrição, versão, tamanho, download e hash.
Trocar a edição limpa a confirmação de substituição e bloqueia o botão até
verificar o pacote escolhido. Uma seleção atrasada não pode sobrescrever
a mais recente. O diálogo do ESP Web Tools bloqueia a troca de edição até
ser fechado. Os dois pares de URLs Blob ficam válidos durante a instalação;
o componente recebe os mesmos bytes que passaram pela verificação.

Manifestos e relatórios são separados. O verificador confere o nome, ambiente,
edição, caminho exato do binário, versão, tamanho, SHA-256, cabeçalhos e
partições. Misturar manifestos/relatórios das edições é rejeitado.
A Classic teve somente o nome de apresentação do manifesto atualizado.
Seu binário e seu relatório de publicação não foram substituídos.

## Compilação e memória

Ambiente novo: `DedSec-Grid-CYD-2432S028`, herdando o ambiente DedSec Classic,
que herda a CYD padrão. Mesmos pinos ILI9341/touch, flash de 4 MiB e partição
factory de 0x3C0000. Nenhuma dependência de firmware foi adicionada.

| Compilação real | Resultado | Aplicação | Tempo |
| --- | --- | ---: | ---: |
| Grid inicial | SUCCESS | 3.700.928 B | 725,40 s |
| Grid com ajuste de texto/buffers | SUCCESS | 3.700.880 B | 179,32 s |
| Grid definitiva, com prioridade para temas/prévias | SUCCESS | 3.702.032 B | 179,95 s |
| Classic, regressão do código compartilhado | SUCCESS | 3.698.576 B | 225,67 s |

Grid definitiva: **230.128 B livres** na factory de 3.932.160 B.
RAM estática: **92.328 / 327.680 B**. IRAM: **113.268 / 131.072 B**.
A imagem completa Grid acrescenta **2.048 B** sobre a Classic publicada.
Não há framebuffer ou sprite de tela inteira; são reutilizados o renderer
de pixels na flash e buffers locais. Heap/fragmentação sob carga não foram
medidos na placa. A recompilação Classic de regressão é apenas evidência de
compatibilidade do código: o instalador continua oferecendo o bw.4 original.

Relatório Grid: [DedSec-Grid-CYD-2432S028-image.json](DedSec-Grid-CYD-2432S028-image.json).
Regressão Classic: [classic-grid-regression-image.json](classic-grid-regression-image.json).
Logs reais: [grid-build-evidence.zip](grid-build-evidence.zip).

## Verificações realizadas

* Três compilações Grid e uma regressão Classic bem-sucedidas.
* esptool offline na Grid definitiva: checksum **0x26 válido** e hash interno
  de validação válido.
* Verificação dos dois pacotes: partições, SHA-256 e os 17 payloads de arte
  na flash conferidos em cada binário.
* **9/9 testes Node passaram**: integridade, corrupção, truncamento,
  incompatibilidade de chip/endereço, pacote Grid, mistura de edições,
  identificadores inválidos e conclusão assíncrona fora de ordem.
* Navegador real: Classic e Grid com versões e downloads correspondentes;
  botão habilitado somente após as confirmações; troca rápida entre edições
  e limpeza da confirmação; pacote ausente bloqueado com erro compreensível.
  Nenhuma porta USB foi solicitada ou firmware gravada durante esse teste.
* `git diff --check`: passou. Avisos herdados das dependências/LTO continuam.

Não há teste físico de display, toque nos cartões, retorno de todas as
ferramentas, rádio, configurações após desligar ou estabilidade prolongada.
Essa validação precisa de uma CYD real. Compilar e testar o website não
substitui testar a firmware no hardware.

## Compilar, instalar e recuperar

Prepare o ambiente conforme [README.md](README.md). Na raiz do repositório:

```powershell
./tools/dedsec/build.ps1 -Environment DedSec-Grid-CYD-2432S028
./.venv/Scripts/python.exe tools/dedsec/prepare_webflasher.py --variant grid --version 2026.10.29-grid.1
./.venv/Scripts/python.exe tools/dedsec/prepare_webflasher.py --check
node --test tools/dedsec/test_webflasher.mjs
```

No [webflasher](https://enzogabriels.github.io/DedSec-OS-Bruce/), escolha
**DedSec OS Grid** ou **DedSec OS Classic**, confira a versão, marque as
confirmações e use Chrome/Edge no computador com cabo USB de dados.
As duas imagens completas são gravadas em **0x0**, para o mesmo modelo de CYD.
Não é uma atualização OTA.

As duas edições compartilham `/bruce.conf` e os arquivos internos. Na troca,
desmarcar `Erase device` preserva o LittleFS; a imagem completa ainda regrava
o prefixo da flash, incluindo NVS. Fazer backup continua recomendado.
Para retornar à versão anterior, selecione Classic no mesmo site.
Para reinstalar Bruce original e recuperar a placa, siga [README.md](README.md).

## Arquivos e integração

Novos: ambiente Grid, `src/core/dedsec_grid.h/.cpp`, catálogo do site,
manifesto/relatório/binário Grid, relatório JSON de validação e evidências.
Alterados: `display.cpp` (hooks condicionados a DEDSEC_GRID),
`dedsec_art.h/.cpp` (composição sólida dos ícones dos cartões),
`dedsec.cpp` (marca no boot), script de build/empacotamento, testes,
HTML/CSS/JS do instalador e workflow que valida as duas edições.
Inventário acumulado em [FILES.txt](FILES.txt). A arquitetura e os avisos
de licença da base Bruce permanecem preservados.
