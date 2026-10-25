# Fundo alternativo e boot longo — 2026.10.25-bw.4

A imagem anexada foi integrada como fundo alternativo dos cartões de categorias
WiFi, Bluetooth, RF e demais menus principais existentes do Bruce.
A animação com a logo DedSec passa de 720 ms para **4 segundos por padrão**.

## Usar

Abra `Config > Display & UI > DedSec UI`:

* `Fundo: imagem DedSec` alterna entre a imagem e o fundo sólido do tema
  (preto na paleta padrão).
* `Duracao boot: 4 s` percorre **4 → 7 → 1 → 4 segundos**.
* `Animacao boot: ON/OFF` continua habilitando ou desabilitando a sequência.

O fundo vem ativado quando a configuração ainda não tem essa opção.
As preferências são salvas no `/bruce.conf` do Bruce somente ao alterar opções.
Temas externos continuam tendo prioridade: para usar o fundo incorporado,
restaure a paleta com `Preto e branco` e habilite `Fundo: imagem DedSec`.
Aplicativos configurados para iniciar automaticamente e InstaBoot mantêm o
comportamento anterior e podem suprimir a animação.

A fase animada de 4 s ocorre após a inicialização dos periféricos/entrada.
Durante a montagem dos arquivos, a logo continua visível como antes.
O toque pula a sequência após os primeiros 120 ms. Os frames são agendados
por `millis()` no loop existente, sem nova tarefa ou sequência de `delay()`.

## Integração e memória

O JPEG fornecido foi preservado em `media/dedsec/source/wallpaper.jpg`.
A ferramenta existente aplica recorte central para preencher 320 × 240,
sem esticar, e quantiza para **16 níveis escuros de 0 a 48**, quatro bits por
pixel. Prévia real em
[wallpaper.png](../../media/dedsec/preview/wallpaper.png).
Não foi gerada uma imagem substituta.

* Fundo empacotado: **38.400 B na flash**.
* Conjunto completo: 17 artes, **105.800 B de pixels na flash**.
* O desenho do fundo usa uma linha RGB565 local de até **640 B** e variáveis.
* Os ícones mantêm a linha local de até **320 B**; seus pixels escuros agora
  recebem a amostra correspondente do fundo, evitando retângulos pretos.
* Sem framebuffer, JPEG/PNG decodificado em RAM, nova dependência de firmware
  ou necessidade de cartão SD.
* A imagem é desenhada na troca de categoria pelo callback de renderização do
  Bruce. Não há atualização contínua do fundo durante leituras do touch.
* Barras de estado, título e controles continuam usando os componentes
  existentes para preservar legibilidade e interação.

As configurações novas são exclusivas de `DEDSEC_OS`:
`dedsecWallpaper` (booleano) e `dedsecBootDuration` (milissegundos).
Arquivos antigos recebem imagem ligada e 4000 ms. Tipos inválidos e durações
fora de 1000–10000 ms recebem os padrões válidos e sinalizam falha de leitura.
A interface oferece os três tempos acima.
Drivers, pinos, módulos e partições não foram alterados.

## Compilação e verificações reais

* **SUCCESS**, `DedSec-CYD-2432S028`, **487,13 s**.
* Aplicação: **3.699.984 B**; factory: **3.932.160 B**;
  livres: **232.176 B**.
* Imagem completa: **3.765.520 B**, endereço de gravação **0x0**.
* Aumento da aplicação sobre bw.3: **41.632 B**.
* RAM estática: **92.336 / 327.680 B** (bw.3: 92.344 B).
  Este número não é uma medição de heap livre.
* Região IRAM: **113.268 / 131.072 B**.

SHA-256 da imagem completa:
`fa4f837f30bbe2e48ad185fa91487c970ad87c6313e41e03a7d03a58a382b16b`.

Validação de partições e hashes:
[DedSec-CYD-2432S028-image.json](DedSec-CYD-2432S028-image.json).
Logs reais:
[wallpaper-build-evidence.zip](wallpaper-build-evidence.zip).
Registro anterior:
[release-2026.10.24-bw.3.json](release-2026.10.24-bw.3.json).

* **6/6** testes de empacotamento: ordem de bits/nibbles, preenchimento de linhas,
  ida e volta de 320 × 240 e entradas inválidas.
* **6/6** testes do verificador de partições: passaram.
* **5/5** testes de integridade do navegador: passaram.
* Encoder `--check`: saída determinística conferida.
* Verificador do pacote: **17/17 artes** presentes no binário, incluindo o fundo,
  com dimensões e hashes de fontes/pixels conferidos.
* Os pixels dos 16 assets anteriores continuam com os mesmos hashes.
* Prévia de 320 × 240 inspecionada visualmente no computador.
* esptool offline: checksum **0xe1 válido** e hash interno válido.
* `git diff --check`: passou.

Avisos herdados de bibliotecas e LTO permanecem; não houve erro de compilação.
Não foi gravada/testada uma CYD física. Aspecto do fundo no TFT, tempo total
de boot, touch para pular, retorno dos submenus, persistência após desligar e
estabilidade/heap durante uso precisam ser validados na placa.

## Instalar e recuperar

Recarregue o [webflasher](https://enzogabriels.github.io/DedSec-OS-Bruce/) e
confira **2026.10.25-bw.4** antes de gravar no Chrome/Edge com cabo USB de dados.
Na atualização, deixar `Erase device` desmarcado preserva o LittleFS e
`/bruce.conf`; a imagem completa ainda regrava o prefixo, incluindo NVS.
Apagar o dispositivo elimina configurações e arquivos internos.
Instruções reproduzíveis de compilação, backup, gravação e reinstalação do
Bruce original estão no [README](README.md).

Arquivos: `include/MenuItemInterface.h`, `src/core/config.h/.cpp`,
`src/core/dedsec_art.h/.cpp`, dados gerados, `src/modules/others/dedsec.cpp`,
encoder/verificador/testes em `tools/dedsec/`, JPEG e prévia em
`media/dedsec/`, relatório/logs/documentação e pacote em `webflasher/`.
Inventário acumulado em [FILES.txt](FILES.txt).
