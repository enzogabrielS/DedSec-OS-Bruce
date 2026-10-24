# Webflasher DedSec OS

Página: https://enzogabrielS.github.io/DedSec-OS-Bruce/

O diretório `webflasher/` contém uma página estática, manifesto ESP Web Tools,
metadados e a imagem completa validada. Não modifica nem recompila a firmware.
O pacote inicial corresponde à versão `2026.10.24+6de6ad3f`; não é uma indicação
de teste físico. A gravação por USB ainda exige validação na CYD real.

## Atualizar o pacote

1. Compile com `tools/dedsec/build.ps1`. Revise o relatório JSON de validação.
2. Execute `python tools/dedsec/prepare_webflasher.py --version SUA_VERSAO`.
   O script exige que o build local corresponda ao relatório revisado; copia
   somente a imagem completa e gera manifesto/metadados.
3. Execute os checks abaixo, revise e envie os arquivos por Git.

```powershell
python tools/dedsec/prepare_webflasher.py --check
node --test tools/dedsec/test_webflasher.mjs
python -m http.server 8765 --bind 127.0.0.1 --directory webflasher
```

Abra `http://127.0.0.1:8765`. Não abra `index.html` por `file://`.
O check funciona em checkout limpo sem PlatformIO: compara SHA-256 com o
relatório versionado, imagem completa/aplicação, cabeçalhos ESP32/DIO/40 MHz,
tabela e limite da partição factory, endereço 0x0 e política explícita de erase.
O navegador verifica também o download antes de liberar o botão e passa os
bytes conferidos ao ESP Web Tools por URLs Blob locais.

## Publicação

Em Settings > Pages, a fonte deve ser **GitHub Actions**. O workflow
`DedSec Webflasher` valida o pacote e os testes de integridade antes de publicar
somente `webflasher/`. Pode ser executado manualmente em Actions. A matriz
original de compilação de todas as placas do Bruce continua disponível
manualmente no fork; seus disparos automáticos ficam restritos ao upstream.

O ESP Web Tools é fixado em 10.4.0 e carregado do CDN unpkg. Não há analytics,
backend, armazenamento de senhas ou acesso automático à porta serial. É
necessário HTTPS ou localhost e um navegador que ofereça Web Serial. O
diálogo nativo do instalador pode usar textos em inglês. Configurar Wi-Fi via
Improv foi desativado porque a firmware não implementa esse protocolo.

## Validação e limites

Testes de host: pacote real íntegro; rejeição de download alterado/truncado,
imagem de aplicação isolada, chip/endereço incorretos, binário remoto e
política de erase incompatível. Teste local de navegador: carregamento real do
componente, SHA-256 conferido e botão bloqueado/liberado pelas confirmações.
Nenhuma gravação física, formatação ou apagamento de dispositivo foi realizado.

Referências: [ESP Web Tools](https://esphome.github.io/esp-web-tools/),
[boot serial ESP32](https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html).
Instruções de backup, gravação manual e recuperação: [README.md](README.md#gravar).
