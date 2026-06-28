# matsu-bun-decoder

松ワープロ（管理工学研究所）の文書ファイル `.bun` をテキストに変換するPythonスクリプトです。
特に **松86・松5・松6** 形式を対象としています。

A Python decoder for `.bun` document files created by **Matsu** (松) word processor
(Kanri Kogaku Kenkyusho / Management Engineering Lab), targeting the **Matsu86/5/6** format.

---

## 背景 / Background

「松」は1983年から1990年代にかけてNEC PC-9801上で広く使われた日本語ワープロソフトです。
当時の研究者・出版社・企業が作成した大量のデジタル文書が `.bun` 形式で残されており、
現代の環境では読むことが困難になっています。

Matsu was a widely used Japanese word processor on NEC PC-9801 from 1983 through the 1990s.
A large volume of documents created by researchers, publishers, and companies in that era
survive as `.bun` files, but are now practically unreadable without special tools.

既存のコンバータ（xdoc2txt, BUN2TX）はWindowsバイナリ専用またはソース非公開であり、
クロスプラットフォームで動作するオープンなデコーダはこれまで存在しませんでした。

Existing converters (xdoc2txt, BUN2TX) are Windows-only binaries or closed-source,
and no open cross-platform decoder existed for this format.

---

## このプロジェクトについて / About this project

### 経緯

このデコーダは、**宋代史を専門とする歴史学者**（青木敦、青山学院大学・東洋文庫）が
1990年代に松ワープロで作成した自身の研究草稿 `.bun` ファイルを復元しようとしたことが
きっかけで生まれました。

著者はプログラミングの専門家ではありません。
**ファイル形式の解析・デコーダの実装はすべて、Anthropic社の大規模言語モデル
Claude（Opus 4.6）との対話を通じて行われました（2026年6月）。**

具体的には：

1. `.bun` ファイルのヘキサダンプをClaudeに渡し、構造を推論させた
2. Claudeが「バイトスワップされたJIS X 0208コード＋属性バイト」という構造を発見した
3. 後日、1986年に村井安雄氏が作成した `BUN2TX.C`（新松専用コンバータのソースコード）が
   見つかり、Claudeの解析を検証・補完することができた
4. 両者の照合により、「新松」と「松86/松5/松6」でフォーマットが異なることも判明した

このプロジェクトは「AIを使ったデジタルアーカイブ復元」の一例でもあります。

### How it was made

This decoder was created by **Atsushi Aoki** (Professor of East Asian History,
Aoyama Gakuin University / The Oriental Library, Toyo Bunko), who needed to recover
his own research manuscripts written in the 1990s on Matsu word processor.

The author is not a programmer.
**All file format analysis and decoder implementation were carried out through dialogue
with Claude (Opus 4.6), a large language model by Anthropic, in June 2026.**

The process:

1. A hex dump of a `.bun` file was provided to Claude, which inferred the file structure
2. Claude discovered the key encoding rule: **byte-swapped JIS X 0208 + attribute bytes**
3. Later, source code of `BUN2TX.C` (by Tomohide Muransky, 1986) was found and used
   to verify Claude's analysis — this also revealed that "Shin-Matsu" (新松) and
   "Matsu86/5/6" use fundamentally different formats
4. The reconciled findings are documented in `REFERENCE.md`

This project is also an example of AI-assisted digital archive recovery.

---

## 使い方 / Usage

### 必要環境 / Requirements

- Python 3.6 以上 / Python 3.6 or later
- 標準ライブラリのみ / Standard library only (no external dependencies)

### 実行方法 / Run

```bash
python decode_matsu.py input.bun
```

テキストを標準出力に出力します。ファイルに保存するには：

To save to a file:

```bash
python decode_matsu.py input.bun > output.txt
```

### コード例 / Example

```python
from decode_matsu import decode_matsu_v56

title, text = decode_matsu_v56("document.bun")
print(f"タイトル: {title}")
print(text)
```

---

## 対応フォーマット / Supported formats

| フォーマット | 対応 | 備考 |
|-------------|------|------|
| 松86 | ✅ | 本スクリプトの主対象 |
| 松5 / 松6 | ✅ | 同一フォーマットと推定 |
| 新松 (Shin-Matsu) | ❌ | BUN2TX（村井氏）を使用してください |
| 松85以前 | ❌ | 未調査 |

---

## 技術仕様 / Technical notes

詳細は [`REFERENCE.md`](REFERENCE.md) を参照してください。

See [`REFERENCE.md`](REFERENCE.md) for full technical documentation.

### 要点 / Key findings

- 本文開始オフセット: `0x019E`（414バイト）
- 文字エンコーディング: **バイトスワップJIS X 0208**
  - ファイル上は `[JIS第2バイト, JIS第1バイト]` の順で格納
  - 標準のJISコード表と**バイト順が逆**
- レコード構造: 8バイト（文字コード2バイト＋書式属性6バイト）
- 改行マーカ: `00 80`

---

## 既知の制限 / Known limitations

- JIS X 0208 の範囲外の文字（補助漢字・外字）は復元できません
- 罫線・表組みは無視されます
- 脚注・注釈の格納構造は未解明です
- 新松形式には対応していません

---

## 参考文献・謝辞 / References & Acknowledgements

- **村井安雄氏 (Tomohide Muransky)**「BUN2TX.C」(1986, ver 0.80286)
  新松文書コンバータのソースコード。本プロジェクトの技術検証に不可欠でした。
  氏のコードおよびコメントはPDSとして公開されています。

- **showさん**「MTOT.C」
  BUN2TX.Cの原型。詳細不明ですが、このプロジェクトの遠い源流です。

- **Anthropic / Claude Opus 4.6**
  ファイル形式の解析・デコーダ実装・ドキュメント作成のすべてを担いました。

- **Wikipedia「松（ワープロ）」**
  歴史的背景の確認に使用しました。

---

## ライセンス / License

デコーダスクリプト本体（`decode_matsu.py`）および技術ドキュメントは
**CC0 1.0 Universal（パブリックドメイン）** で提供します。
自由に使用・改変・再配布してください。

The decoder script (`decode_matsu.py`) and technical documentation are released under
**CC0 1.0 Universal (Public Domain)**. Use, modify, and redistribute freely.
