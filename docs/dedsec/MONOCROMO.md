# Revisão preto e branco — 24/10/2026

Versão do pacote: `2026.10.24-bw.1`. A base continua sendo o Bruce oficial e o
ambiente continua `DedSec-CYD-2432S028`, sem mudanças de pinos ou partições.

## Alterações

* Paleta padrão preto, branco e cinzas, incluindo mensagens de estado. Erros
  usam letras pretas em faixa branca para manter contraste.
* Animação original de caveira geométrica, olhos cruzados e nome DedSec OS,
  com revelação e deslocamentos curtos. Até 720 ms, frames de 60 ms, cancelável
  por toque a partir de 120 ms. Executada incrementalmente no loop existente,
  com temporização por millis(), sem sprite, arquivo de imagem ou tarefa nova.
* Menu principal original abre após a animação. A classe e a aba adicional
  System foram removidas. Dashboard, snapshot e informação de hardware ficam
  em Config; busca passiva fica em WiFi; descoberta BLE continua em Bluetooth.
* Removida a confirmação adicional `Alvo proprio/autorizado` e a interceptação
  genérica de seleção de opções. As ferramentas originais continuam usando
  seus próprios menus e seletores de rede/dispositivo quando disponíveis.
  Nenhum algoritmo de ataque, alvo automático ou método de transmissão novo.
* Confirmações de requisitos de expansão e interrupção de Wi-Fi/BLE continuam
  ligadas à disponibilidade de recursos. Confirmações destrutivas originais
  do Bruce, como restauração de configurações, foram preservadas.

A animação respeita `instantBoot` e não atrasa um aplicativo configurado para
iniciar automaticamente. Em `Config > Display & UI > DedSec UI`, use a opção
de animação e `Preto e branco`. A antiga paleta vermelha padrão migra uma vez
ao carregar `/bruce.conf`, usando `dedsecUiRevision`; arquivos de tema e
paletas personalizados são preservados. Não é necessário apagar configurações
somente para instalar esta revisão sobre a versão DedSec anterior.

## Validação

Os números atuais de aplicação, imagem completa, espaço livre e SHA-256 estão
em [DedSec-CYD-2432S028-image.json](DedSec-CYD-2432S028-image.json).
O relatório inicial em [VALIDACAO.md](VALIDACAO.md) é histórico; o seu JSON
original está em `release-2026.10.24-6de6ad3f.json`.

| Compilação real | Resultado | Aplicação | RAM estática | Tempo |
| --- | --- | ---: | ---: | ---: |
| Paleta, animação e remoção de System | SUCCESS | 3.600.176 B | 92.320 B | 478,58 s |
| Fluxo original das ferramentas e versão final | SUCCESS | 3.598.624 B | 92.320 B | 185,48 s |

A imagem completa final tem **3.664.160 bytes**, com **333.536 bytes livres**
na partição factory de 3.932.160 bytes. Em relação à primeira versão DedSec,
a aplicação ficou 1.056 bytes menor e a RAM estática caiu 40 bytes. IRAM:
113.268 / 131.072 bytes. Não houve medição física de heap ou fragmentação.

SHA-256 da imagem completa:
`1ba00fef0163e264bc34a6786b9d9889044c186b9b0e1b17834e7d4182650b57`.

Verificações executadas:

* 6/6 testes host de limites e endereços da imagem passaram. A primeira
  execução encontrou bloqueio do diretório temporário pelo sandbox; a
  repetição fora dessa restrição passou.
* 5/5 testes do verificador do navegador passaram usando o novo pacote real.
* Check de pacote: SHA-256, imagem completa/aplicação, cabeçalhos e tabela de
  partições correspondem ao build. esptool offline: checksum e hash válidos.
* A imagem final contém a nova rotina de boot/textos e não contém o texto da
  confirmação removida. `wifi_atks.cpp` e `deauther.cpp` continuam idênticos
  à base oficial utilizada; os seletores não foram reimplementados.
* `git diff --check` passou. Avisos herdados de bibliotecas gráficas/NimBLE e
  compilação LTO continuam presentes; não houve erro de compilação.

Logs das duas compilações e do esptool: `monochrome-build-evidence.zip`.
Metadados intermediários: `monochrome-phase1-image.json`.

Nenhum teste físico de boot, toque, display, rádio ou estabilidade foi realizado
nesta revisão. A animação precisa ser conferida na CYD; a compilação não mede
fluidez nem prova a calibração física do touch. Os roteiros de instalação e
recuperação continuam em [README.md](README.md).

## Atualizar a placa

Abra o [webflasher](https://enzogabriels.github.io/DedSec-OS-Bruce/), confira a
versão `2026.10.24-bw.1` e instale pelo USB. Ao atualizar uma DedSec anterior,
deixe `Erase device` desmarcado para preservar o LittleFS e o `/bruce.conf`.
A imagem completa também regrava a região NVS no prefixo da flash, mesmo sem
apagamento integral. Faça backup dos dados importantes. Na troca de outra firmware/partição, siga a instalação
limpa documentada. Após gravar, o menu começa em WiFi; diagnósticos estão em
Config. Temas personalizados podem ser trocados por `Preto e branco` em DedSec UI.
