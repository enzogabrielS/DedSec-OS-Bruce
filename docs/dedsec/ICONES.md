# Ícones aprovados — 2026.10.24-bw.3

Os 12 símbolos aprovados pelo usuário foram incorporados aos menus existentes
do Bruce para `DedSec-CYD-2432S028`.

## Alterações e integração

| Categoria | Original aprovado em media/dedsec/source/ | Menu |
| --- | --- | --- |
| RF | rf.png | RFMenu.cpp |
| NRF24 | nrf24.png | NRF24.cpp |
| LoRa | lora.png | LoRaMenu.cpp |
| FM | fm.png | FMMenu.cpp |
| Infravermelho | ir.png | IRMenu.cpp |
| Ethernet | ethernet.png | EthernetMenu.cpp |
| GPS | gps.png | GpsMenu.cpp |
| RFID | rfid.png | RFIDMenu.cpp |
| Arquivos | files.png | FileMenu.cpp |
| Scripts | scripts.png | ScriptsMenu.cpp |
| Relógio | clock.png | ClockMenu.cpp |
| Outros | others.png | OthersMenu.cpp |

Cada menu chama o mesmo `dedsecDrawArt` já usado por WiFi, Bluetooth e Config,
com o tamanho limitado à área de ícone calculada pelo Bruce e com as cores
da configuração atual. A alteração é condicionada a `DEDSEC_OS`; o desenho
original continua disponível para os ambientes Bruce sem essa opção.
Temas externos mantêm a prioridade do `MenuItemInterface` original.
Os seletores, ferramentas, touch, drivers, pinos e partições não foram alterados.

A logo de inicialização e os três símbolos anteriores mantêm os mesmos hashes
dos PNGs e dos pixels empacotados. A animação de boot e as opções de tema da
revisão anterior continuam disponíveis em
`Config > Display & UI > DedSec UI`.

As artes novas foram geradas com image_gen a partir da referência visual
fornecida e aprovadas antes da integração. Os originais são preservados em
`media/dedsec/source/`; os prompts estão em
`media/dedsec/generated-icons.json`. O encoder não redesenha essas imagens.

## Memória e compilação verificadas

* Compilação PlatformIO: **SUCCESS**, ambiente `DedSec-CYD-2432S028`, 204,24 s.
* Cada novo ícone: 128 × 128, quatro tons, dois bits por pixel, **4.096 B**.
* Novos pixels: **49.152 B** na flash; conjunto completo: **67.400 B**.
* Aplicação: **3.658.352 B**, aumento de **42.512 B** sobre bw.2.
* Partição factory: **3.932.160 B**; livres: **273.808 B**.
* Imagem completa: **3.723.888 B**, endereço de gravação **0x0**.
* RAM estática: **92.344 / 327.680 B**, aumento de **24 B** sobre bw.2.
* Região IRAM: **113.268 / 131.072 B**.

O renderizador continua usando um buffer local de uma linha RGB565 de até
320 B e uma paleta de quatro cores. Os pixels permanecem na flash. Não há
framebuffer, decodificação de PNG no dispositivo, tarefas novas ou alocações
dinâmicas novas no caminho de renderização.
Heap livre, fragmentação e estabilidade durante uso não foram medidos na placa.
Pillow é usado somente para regenerar os assets; nenhuma biblioteca nova foi
adicionada à compilação da firmware.

SHA-256 da imagem completa:
`3148038c341226e2ad24ed6f5afe3b7a2ee1a8b260b8da65ddfd1b99de9d6031`.

Tamanhos e hashes de aplicação/imagem completa:
[DedSec-CYD-2432S028-image.json](DedSec-CYD-2432S028-image.json).
Logs reais da build e esptool:
[approved-icons-build-evidence.zip](approved-icons-build-evidence.zip).
A publicação bw.2 anterior está preservada em
[release-2026.10.24-bw.2.json](release-2026.10.24-bw.2.json).

## Verificações executadas

* Regeneração determinística `encode_art.py --check`: passou.
* 4/4 testes de empacotamento e 6/6 testes de limites/endereços das partições:
  passaram.
* 12 prévias efetivas de 128 × 128 inspecionadas visualmente.
* Comparação dos quatro assets anteriores: fontes e pixels inalterados.
* Verificador de artes: **16/16 payloads encontrados** no binário, com hashes,
  dimensões, tamanho empacotado e fontes conferidos.
* Verificador do pacote: versão, manifesto, SHA-256 e limite antes do LittleFS
  conferidos.
* 5/5 testes de integridade do navegador: passaram.
* esptool offline: checksum **0xb6 válido** e hash interno de validação válido.
* `git diff --check`: passou.

Avisos herdados de bibliotecas NimBLE/Chameleon e do LTO permanecem no log.
Não houve erro de compilação. Esta revisão adiciona símbolos visuais;
não adiciona capacidades de rádio ou periféricos à CYD.

## Instalar e validar na placa

Abra o [webflasher](https://enzogabriels.github.io/DedSec-OS-Bruce/), recarregue
e confira **2026.10.24-bw.3**. Use Chrome ou Edge no computador, cabo USB de
dados e a porta serial correspondente à CYD padrão ILI9341 resistiva de 4 MiB.
Siga [instalação, backup e recuperação](README.md).

Ao atualizar a DedSec existente, desmarcar `Erase device` preserva o LittleFS
e `/bruce.conf`; a imagem completa ainda regrava o prefixo, incluindo NVS.
Apagar o dispositivo elimina os arquivos/configurações internos.
Para ver os ícones incorporados, selecione o tema preto e branco caso um tema
externo esteja configurado.

Não foi gravada nem testada uma CYD física nesta revisão. Valide na placa:
boot/animação, os 15 ícones e seus títulos, navegação/toque em todas as
categorias, retorno dos submenus e comportamento após reiniciar.
RF/NRF24/LoRa/FM/IR/Ethernet/GPS/RFID continuam dependendo dos módulos e das
configurações descritas na auditoria; o símbolo não significa hardware presente.
A reinstalação do Bruce original continua documentada no [README](README.md).

## Arquivos

Alterações no enum e seleção de arte em `src/core/dedsec_art.h/.cpp`, no
arquivo gerado `dedsec_art_data.inc` e nos 12 menus da tabela acima.
Encoder e verificador em `tools/dedsec/encode_art.py` e `check_art_image.py`.
Mais 12 originais, 12 prévias, metadados e registro de prompts em
`media/dedsec/`. Pacote/binário, manifesto e versão em `webflasher/`.
Documentação, relatório JSON e logs em `docs/dedsec/`.
Inventário acumulado em [FILES.txt](FILES.txt).
