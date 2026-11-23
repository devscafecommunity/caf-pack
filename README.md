# caf-pack — Caffeine Asset Packer

<div class="doc-meta">
  <span class="doc-badge doc-badge--stable">Stable</span>
  <span class="doc-badge">Tooling</span>
  <span>Atualizado em: <time datetime="2026-11-18">2026-11-18</time></span>
  <span>Fonte: <a href="https://github.com/devscafecommunity/caf-pack"><code>devscafecommunity/caf-pack</code></a></span>
</div>

<p class="doc-lede">Converte assets brutos (PNG, WAV, OBJ) em ficheiros
<code>.caf</code> alinhados e empacota-os num <code>.cap</code> pronto para memory
mapping. A engine lê o binário diretamente; não há parsing de texto em runtime.</p>

## Filosofia

| Princípio | O que significa |
|---|---|
| **Zero-parsing** | A CLI entrega a struct binária no layout C++ de `include/caffeine/CafTypes.hpp`. |
| **Alinhamento** | Cada payload começa num limite de 32 bytes (`Packer::Config::alignment`). |
| **Hash-based** | Assets são referenciados por `u64` (MurmurHash3), sem procuras por string em runtime. |
| **Pack** | Processa cada asset para `.caf` em memória e agrega tudo num `.cap`. |

## Estrutura real

```text
caf-pack/
├── include/
│   ├── caffeine/CafTypes.hpp      # Formatos CAF e CAP partilhados com a engine
│   └── caf-pack/
│       ├── Packer.hpp             # Descoberta, processamento e container CAP
│       ├── Registry.hpp           # Registo de processors por extensão
│       ├── Reader.hpp             # Ler .caf e .cap
│       ├── HeaderGenerator.hpp    # IDs C++ a partir dos assets
│       └── *Processor.hpp         # PNG, WAV, OBJ → .caf
├── src/
│   ├── main.cpp                   # CLI
│   ├── Packer.cpp
│   ├── Registry.cpp
│   ├── Reader.cpp
│   ├── AssetProcessor.cpp
│   ├── TextureProcessor.cpp
│   ├── AudioProcessor.cpp
│   ├── MeshProcessor.cpp
│   └── HeaderGenerator.cpp
└── CMakeLists.txt
```

## CLI

```bash
caf-pack --input ./assets_raw --output game.cap
caf-pack --input ./assets_raw --output game.cap --gen-ids assets_ids.hpp
caf-pack --input ./assets_raw --output game.cap --compress
```

| Opção | Efeito |
|---|---|
| `--input <dir>` | Diretório de assets brutos (obrigatório). |
| `--output <file>` | Ficheiro `.cap` de saída (padrão `assets.cap`). |
| `--gen-ids <file>` | Gera um header C++ com constantes de id. |
| `--compress` | Ativa compressão opcional. |
| `--convert-terrain <ficheiro.cterrain>` | Converte um terreno legado num `.terrain` nativo (contentor `caf` por tiles), sem perdas: lê o resultado de volta e compara bit a bit. `--output` define o ficheiro (padrão: mesmo nome, `.terrain`). Só existe quando o caf-pack é compilado dentro da árvore da engine. |
| `--terrain-size <x> <z>` | Extensão em metros gravada no `.terrain` (padrão 512 512; o `.cterrain` não a guarda). |
| `--terrain-max-height <m>` | Escala de altura em metros gravada no `.terrain` (padrão 80). |
| `--no-verify` | Salta a verificação bit a bit da conversão de terreno. |
| `--help` | Mostra a ajuda. |

O Doppio usa a mesma CLI no assistente de Assets (Create/Import/Pack); o caminho do
binário é descoberto por `CAF_PACK`, `CAFFEINE_ENGINE_ROOT`, `caf-pack/build/src` ou
`PATH`.

## Biblioteca (`caf-pack-lib`)

A CLI é uma camada fina sobre a biblioteca partilhada:

```cpp
#include "caf-pack/Packer.hpp"
#include "caf-pack/Reader.hpp"
#include "caf-pack/Registry.hpp"

CafPack::Packer::Config config;
config.inputDir = "assets/raw";
config.outputFile = "build/game.cap";
config.generateHeader = true;
config.headerPath = "build/game_assets.hpp";

CafPack::Packer packer(config);
if (!packer.pack()) {
    // packer.getError()
}

auto assets = CafPack::Reader::loadCap("build/game.cap");
CafPack::CafAsset one;
CafPack::Reader::loadCaf("assets/raw/Textures/brick.caf", one);
```

| Classe | Responsabilidade |
|---|---|
| `Packer` | Descobre assets, processa e escreve o container CAP. |
| `Registry` | Registo extensível de processors; `makeDefaultRegistry()` inclui textura, áudio e mesh. |
| `Reader` | `loadCap`, `loadCaf` e `readCafHeader` para consumidores C++ e tools. |
| `HeaderGenerator` | Gera constantes C++ com os hashes dos assets. |

### Registar um novo tipo

1. Adicione o valor em `CafAssetType` (`include/caffeine/CafTypes.hpp`).
2. Defina a metadata no mesmo header, se o asset precisar de campos próprios.
3. Implemente `AssetProcessor` (`canProcess` + `process`).
4. Registe-o em `Registry` (ou em `makeDefaultRegistry()` para um processor built-in).
5. Consumidores usam `Reader`/`AssetManager`; não fazem parsing do `.caf` à mão.

## Formato CAF

`CafHeader` tem 32 bytes: magic `CAF!`, versão, tipo de asset, tamanho do payload,
flags e CRC64. Metadados específicos seguem o header:

| Tipo | Metadados |
|---|---|
| Textura | `width`, `height`, `format` (RGBA8/BC1/BC4/BC5), `mipLevels` |
| Áudio | `sampleRate`, `sampleCount`, `channels`, `format` (PCM16/PCM32) |
| Mesh | `vertexCount`, `indexCount`, `vertexStride`, `indexFormat` |

Os dados começam no próximo limite de 32 bytes.

## Formato CAP

| Secção | Conteúdo |
|---|---|
| **Header** | `CapHeader` com magic `CAP/`, versão e contagem de assets. |
| **Table** | `CapEntry[]` com `hashID`, `offset` e `size`. |
| **Padding** | Alinhamento a 32 bytes. |
| **Blobs** | Payloads `.caf` concatenados. |

O runtime mapeia o `.cap` e resolve assets pelo hash, sem varrer diretórios.

## Build

Requisitos: C++17, CMake 3.20+, libpng, zlib e zstd.

```bash
cmake -S caf-pack -B caf-pack/build -DCAFFEINE_ROOT="$PWD"
cmake --build caf-pack/build -j
```

O binário fica em `caf-pack/build/src/caf-pack` num build standalone, ou em
`build/caf-pack/src/caf-pack` no monorepo. A partir do repositório da engine, o build normal também o compila
quando o submódulo está presente.

## Testes

Os testes do caf-pack vivem em `tests/` no repositório do submódulo. No monorepo, a
engine compila a biblioteca partilhada e os testes de integração usam o mesmo
`CafTypes.hpp`; execute `ctest` a partir de `build/`.

## Relação com a engine

- `include/caffeine/CafTypes.hpp` é a fonte de verdade dos layouts.
- O asset manager carrega `.caf`/`.cap` por hash e faz memory mapping.
- O pipeline de import do Doppio chama a CLI e atualiza a cache de assets.
