# Logo e símbolos dos anexos — 2026.10.24-bw.2

Registro histórico desta revisão. A versão atual com os outros 12 símbolos
aprovados e suas medições está em [ICONES.md](ICONES.md).

Integração direta na base Bruce, para `DedSec-CYD-2432S028`.

## Resultado no código

* Primeira imagem anexada: logo desde o primeiro acesso ao display, antes de
  montar o armazenamento. Substitui a antiga tela textual de boot. Os marcos
  reais de inicialização continuam no log serial a 115200 baud.
* A logo permanece visível nas operações bloqueantes de armazenamento. Após
  inicializar a entrada, a animação no loop desloca pequenas faixas da mesma
  imagem e estabiliza. Até 720 ms, pulável por toque a partir de 120 ms. Não há
  uma tarefa extra desenhando concorrentemente com o display.
* Símbolos Wi-Fi, Bluetooth e engrenagem substituem os ícones padrão dos menus
  WiFi, Bluetooth e Config. Drivers, navegação e seletores foram preservados.
* Temas personalizados continuam tendo prioridade. Para restaurar os anexos,
  use `Config > Display & UI > DedSec UI > Preto e branco`. A opção
  `Animacao boot: ON` habilita a animação; aplicativos de início automático
  continuam tendo prioridade, como na revisão anterior.

Os PNGs fornecidos estão em `media/dedsec/source/`; prévias reais dos pixels
otimizados em `media/dedsec/preview/`. A autoria das artes não é atribuída ao
projeto. Mapeamento, avisos e reprodução do processo em
[media/dedsec/README.md](../../media/dedsec/README.md).

## Memória e compilação reais

Os quatro assets usam quatro níveis de luminância, dois bits por pixel:
logo 160 × 149 (5.960 B) e três símbolos 128 × 128 (4.096 B cada).
Total: **18.248 B de pixels na flash**. O renderer usa uma linha local RGB565
de até **320 B**, mais paleta de quatro cores e variáveis locais. Não carrega
PNG, framebuffer ou sprites em RAM e não precisa de SD para exibir os anexos.
Pillow 12.3.0 é necessário somente para regenerar assets; a compilação normal
usa os pixels já versionados e não acrescenta biblioteca à firmware.

| Etapa real | Resultado | Aplicação | RAM estática | Tempo |
| --- | --- | ---: | ---: | ---: |
| Símbolos dos menus e renderer | SUCCESS | 3.610.512 B | 92.320 B | 222,13 s |
| Logo desde o boot e versão final | SUCCESS | 3.615.840 B | 92.320 B | 182,38 s |

* Imagem completa: **3.681.376 B**, gravada em 0x0.
* Aplicação: **3.615.840 B**; factory de 3.932.160 B; **316.320 B livres**.
* RAM estática: 92.320 / 327.680 B, igual à revisão bw.1.
* IRAM: 113.268 / 131.072 B.
* Crescimento da aplicação sobre bw.1: 17.216 B. Heap em execução não foi
  medido na placa; os números de RAM estática não representam heap livre.

SHA-256 da imagem completa:
`a658ee8e4f3dd05be64c9767711f991cdf2b21aa53a8fb346e86af54f07ae808`.
Demais hashes em [DedSec-CYD-2432S028-image.json](DedSec-CYD-2432S028-image.json).
Intermediário: `artwork-icons-image.json`. Logs brutos reais das duas builds
e do esptool em `artwork-build-evidence.zip`.

## Validação executada

* 4/4 testes do empacotamento: ordem de bits conhecida, preenchimento de
  linhas, dimensões variadas e entradas inválidas.
* Regeneração determinística `encode_art.py --check`: passou; quatro prévias
  inspecionadas visualmente no ambiente de desenvolvimento.
* Check da publicação: hashes de fontes/pixels, dimensões e os quatro payloads
  de pixels encontrados na imagem compilada. Sem dependência de Pillow no CI.
* 6/6 testes host do verificador de partições e 5/5 testes de integridade do
  navegador passaram. O pacote final cabe antes do LittleFS e preserva os
  endereços de bootloader/tabela/aplicação.
* esptool offline: checksum 0x69 válido e validation hash válido.
* `git diff --check`: passou. Avisos herdados de bibliotecas/LTO permanecem;
  nenhum erro de compilação nos novos arquivos.

Não foi gravada nem testada uma CYD física nesta revisão. Boot real, aspecto
do display, fluidez, touch, duração sob carga e estabilidade na placa ainda
exigem validação. A prévia de um PNG não substitui esse teste.

## Instalar

Recarregue o [webflasher](https://enzogabriels.github.io/DedSec-OS-Bruce/),
confira `2026.10.24-bw.2` e grave pelo USB. Backup, escolha de porta, efeito
do apagamento e recuperação continuam documentados no [README](README.md).
Os registros bw.1 e da entrega inicial foram preservados como históricos.
