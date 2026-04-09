# 📦 caf-pack (Caffeine Asset Packer)

O **caf-pack** é o motor de processamento de dados da **Caffeine Engine**. Ele transforma arquivos brutos (PNG, OBJ, JSON, WAV) em arquivos binários otimizados `.caf` e os agrupa em um super-pacote `.cap` pronto para **Memory Mapping (mmap)**.

## 🎯 Filosofia
* **Zero-Parsing:** A Engine não interpreta texto; o caf-pack já entrega o binário no formato da struct de C++.
* **Memory-Ready:** Todo asset é alinhado em 32-bytes para acesso direto via CPU/GPU.
* **Hash-Based:** Arquivos são referenciados por `u64` (MurmurHash3), eliminando buscas por strings em runtime.

---

## 🛠️ Estrutura do Repositório

```text
/
├── bin/                # Binários compilados (caf-pack CLI)
├── src/
│   ├── core/           # Lógica de empacotamento e alinhamento
│   ├── processors/     # Conversores (Image2Caf, Mesh2Caf, Scribe2Caf)
│   └── utils/          # Hashing (MurmurHash3) e IO
├── include/
│   └── Caffeine/       # Shared Headers (CafTypes.hpp) entre Pack e Engine
├── tests/              # Assets de teste e validação de saída
└── build.py            # Script de build (ou CMakeLists.txt)
```

---

## 🏗️ O Formato .CAP (Caffeine Asset Pack)

O arquivo `.cap` gerado segue o layout de memória abaixo:

| Seção | Conteúdo | Descrição |
| :--- | :--- | :--- |
| **Header** | `CapHeader` | Magic (`CAP!`), Versão e Contagem de Assets. |
| **Table** | `CapEntry[]` | Array de busca rápida: `HashID | Offset | Size`. |
| **Padding** | `0x00...` | Alinhamento para garantir que o primeiro blob comece em 32-bytes. |
| **Blobs** | `[.caf, .caf, ...]` | O payload real dos arquivos processados. |

---

## 🚀 Como Usar (CLI)

### 1. Empacotar um diretório
```bash
./caf-pack --input ./assets_raw --output game_data.cap
```

### 2. Gerar Header de IDs (Opcional)
Gera um arquivo `.hpp` com constantes para que você não precise usar strings nem no código:
```bash
./caf-pack --input ./assets_raw --gen-ids assets_ids.hpp
```

---

## ⚙️ Processadores Integrados

1.  **Scribe (Localization):** Converte arquivos `.lang` (chave-valor) em uma `StringTable` binária de acesso $O(1)$.
2.  **Mesh (3D):** Converte `.obj` ou `.fbx` para buffers de vértices intercalados (*interleaved*).
3.  **Texture:** Converte imagens para formatos brutos (RGBA8/BCn) com suporte a mipmaps pré-gerados.

---

## 🛠️ Build & Requisitos

* **Linguagem:** C++ 17 ou superior.
* **Dependências:** * `stb_image` (Processamento de imagem).
    * `fast_obj` (Parser rápido de OBJ).
* **Plataformas:** Windows (MSVC), Linux (GCC/Clang).

---

## 📝 Exemplo de Implementação de Header (`CafTypes.hpp`)

```cpp
namespace Caffeine {
    struct CapHeader {
        uint32_t magic = 0x43415021; // "CAP!"
        uint32_t assetCount;
    };

    struct CapEntry {
        uint64_t hashID;   // MurmurHash3 do path original
        uint64_t offset;   // Posição absoluta no arquivo
        uint64_t size;     // Tamanho total do asset
    };
}
```

> **Nota:** Este projeto é parte integrante do ecossistema **Caffeine Engine**.
