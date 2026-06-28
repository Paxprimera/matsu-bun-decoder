#!/usr/bin/env python3
"""
decode_matsu.py — 松ワープロ（松86/松5/松6）.bun ファイル テキスト変換スクリプト

Decoder for Matsu word processor (Matsu86/5/6) .bun document files.

作者 / Author:    Atsushi Aoki (青木敦)
                  Professor of East Asian History,
                  Aoyama Gakuin University / The Oriental Library (Toyo Bunko)
実装 / Implemented by: Claude Opus 4.6 (Anthropic), June 2026
ライセンス / License: CC0 1.0 Universal (Public Domain)

技術的背景 / Technical background:
  File format reverse-engineered by Claude Opus 4.6 via hex dump analysis,
  subsequently verified against BUN2TX.C (Tomohide Muransky, 1986).
  See REFERENCE.md for full documentation.
"""

import sys


def decode_matsu_v56(filepath):
    """
    松86/松5/松6 形式の .bun ファイルをデコードする。

    Decode a .bun file in Matsu86/5/6 format.

    Args:
        filepath: Path to the .bun file

    Returns:
        (title, text): タイトル文字列と本文テキスト
                       A tuple of (title string, body text)
    """
    with open(filepath, 'rb') as f:
        data = f.read()

    # ヘッダからタイトルを取得（オフセット 0x07、Shift-JIS）
    # Extract title from header (offset 0x07, Shift-JIS encoded)
    try:
        title = data[0x07:0x2F].decode('cp932', errors='replace').rstrip('\x00')
    except Exception:
        title = ''

    def is_jis_pair(b0, b1):
        """JIS X 0208 の有効範囲かチェック"""
        return 0x21 <= b0 <= 0x7E and 0x21 <= b1 <= 0x7E

    # 本文開始オフセット（松86/5/6 固定値）
    # Body start offset (fixed for Matsu86/5/6)
    offset = 0x019E
    result = []

    while offset < len(data) - 1:
        b0 = data[offset]
        b1 = data[offset + 1]

        # 改行マーカ: 00 80
        # Newline marker
        if b0 == 0x00 and b1 == 0x80:
            result.append('\n')
            offset += 2
            continue

        # その他の制御コード（00 XX）: スキップ
        # Other control codes starting with 0x00: skip
        if b0 == 0x00:
            offset += 2
            continue

        # 全角文字: バイトスワップJIS X 0208
        # Full-width character: byte-swapped JIS X 0208
        # ファイル上は [JIS第2バイト, JIS第1バイト] の順で格納
        # File stores bytes as [JIS byte 2, JIS byte 1] (reversed order)
        if is_jis_pair(b0, b1):
            jis_hi = b1   # ファイルの2バイト目 → JISの第1バイト
            jis_lo = b0   # ファイルの1バイト目 → JISの第2バイト
            try:
                esc = b'\x1b$B' + bytes([jis_hi, jis_lo]) + b'\x1b(B'
                result.append(esc.decode('iso-2022-jp'))
            except Exception:
                result.append('?')

            # 属性バイト（書式情報）をスキップ
            # Skip attribute bytes (formatting info, typically 6 bytes)
            # 次の有効文字または改行マーカが来るまで走査
            # Scan forward until next valid character or newline
            offset += 2
            while offset < len(data) - 1:
                nb0 = data[offset]
                nb1 = data[offset + 1]
                if nb0 == 0x00 and nb1 == 0x80:   # 次が改行
                    break
                if is_jis_pair(nb0, nb1):          # 次の文字
                    break
                offset += 1
            continue

        # 不明バイト: スキップ
        # Unknown byte: skip
        offset += 1

    return title, ''.join(result)


def main():
    if len(sys.argv) < 2 or sys.argv[1] in ('-h', '--help'):
        print("使い方 / Usage: python decode_matsu.py <file.bun>")
        print()
        print("  松86/松5/松6 形式の .bun ファイルをテキストに変換して標準出力に出力します。")
        print("  Converts a Matsu86/5/6 .bun file to text and writes to stdout.")
        print()
        print("  例 / Example:")
        print("    python decode_matsu.py document.bun")
        print("    python decode_matsu.py document.bun > output.txt")
        print()
        print("  注意 / Note:")
        print("    新松形式には対応していません。BUN2TX (Muransky, 1986) を使用してください。")
        print("    Does not support Shin-Matsu (新松) format. Use BUN2TX for that.")
        sys.exit(0)

    filepath = sys.argv[1]

    try:
        title, text = decode_matsu_v56(filepath)
    except FileNotFoundError:
        print(f"エラー / Error: ファイルが見つかりません / File not found: {filepath}",
              file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"エラー / Error: {e}", file=sys.stderr)
        sys.exit(1)

    if title:
        print(f"# {title}")
        print()

    print(text)


if __name__ == '__main__':
    main()
