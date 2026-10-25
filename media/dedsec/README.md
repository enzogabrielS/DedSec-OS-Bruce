# Artes aprovadas para a DedSec OS

Os quatro PNGs `logo.png`, `wifi.png`, `bluetooth.png` e `config.png` em
`source/` foram anexados pelo usuário e são preservados sem
alterar seus pixels. Eles não são uma criação visual original deste projeto.
Este registro não atribui ao projeto a autoria ou direitos exclusivos sobre
as marcas/elementos fornecidos, nem declara que esses elementos são AGPL.
Os avisos de licença do código Bruce e de terceiros continuam preservados.

Mapeamento: primeira imagem → logo de boot; símbolo Wi-Fi → menu WiFi;
símbolo Bluetooth → menu Bluetooth; engrenagem → menu Config.

Os outros 12 PNGs foram gerados com image_gen usando a engrenagem como
referência de estilo e aprovados pelo usuário antes desta integração:
`rf.png`, `nrf24.png`, `lora.png`, `fm.png`, `ir.png`, `ethernet.png`,
`gps.png`, `rfid.png`, `files.png`, `scripts.png`, `clock.png`, `others.png`.
Os originais aprovados também são preservados. Categorias, ferramenta e
prompts estão registrados em `generated-icons.json`. A representação gráfica
de um periférico não indica que esse hardware esteja presente na CYD.

`tools/dedsec/encode_art.py` reduz as imagens com Lanczos e uma curva de
luminância fixa para preservar legibilidade, quantiza para quatro tons e
empacota quatro pixels por byte, da esquerda para a direita (bits altos
primeiro). Cada linha recebe preenchimento independente. As proporções e a
composição dos originais são mantidas. A conversão não redesenha os símbolos.

* Logo: 160 × 149, 5.960 bytes.
* Cada ícone: 128 × 128, 4.096 bytes.
* Total dos pixels compilados (logo + 15 ícones): **67.400 bytes na flash**.
* Acréscimo dos 12 ícones aprovados: **49.152 bytes de pixels na flash**.
* Renderização: buffer local RGB565 de uma linha de até 320 bytes, mais uma
  paleta de quatro cores. Sem framebuffer, sprites, PNGs em RAM ou acesso ao SD.

As prévias em `preview/` mostram exatamente os quatro níveis empacotados,
antes da conversão para RGB565. `assets.json` registra dimensões e hashes.
Os originais/PNG de prévia não são incorporados à firmware; somente o arquivo
gerado `src/core/dedsec_art_data.inc` é utilizado pelo renderizador.

Para regenerar, use Python e Pillow 12.3.0 (dependência só da ferramenta):

```powershell
python -m pip install -r tools/dedsec/requirements-assets.txt
python tools/dedsec/encode_art.py
python tools/dedsec/encode_art.py --check
python -m unittest discover -s tools/dedsec -p test_art.py
```

O check de publicação `tools/dedsec/check_art_image.py` usa só a biblioteca
padrão e confere hashes/fontes, limites de tamanho e a presença dos 16
payloads de pixels na imagem compilada. Ele também é executado pelo verificador
do pacote e pelo workflow do webflasher, sem instalar Pillow no CI.

A compilação PlatformIO normal não precisa instalar Pillow, pois os pixels
gerados ficam versionados no repositório.
