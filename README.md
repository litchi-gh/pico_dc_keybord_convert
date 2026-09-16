# Dreamcast HKT-4000 USBキーボード変換

[English](README.en.md) | 日本語

Dreamcast純正キーボード HKT-4000を、Raspberry Pi PicoまたはWaveshare
RP2040-Zero経由でPC用USB HIDキーボードとして使うファームウェアです。

Waveshare RP2040-Zeroと日本語版HKT-4000の実機で、単独キー、複数キー同時押し、
左右Shiftを含む修飾キーの動作を確認しています。

## ビルド済みUF2

| ボード | ファイル | 状態 |
|---|---|---|
| Waveshare RP2040-Zero | [dc_keyboard_usb_rp2040_zero.uf2](dist/dc_keyboard_usb_rp2040_zero.uf2) | 実機確認済み |
| Raspberry Pi Pico | [dc_keyboard_usb.uf2](dist/dc_keyboard_usb.uf2) | ビルド済み |

SHA-256:

- RP2040-Zero: `2B23B1155E3EBF05543E92A401153011341636D98E3A28F8BD07A501A2D2E676`
- Pico: `4ECC4EC69D912EEF195321348F3373D8D4D309B468073DA1D19DD5F591C4AFF3`

## 実機例

| HKT-4000 | RP2040-Zero配線 |
|---|---|
| <img src="docs/hkt-4000-keyboard.jpg" alt="Dreamcast HKT-4000 keyboard" width="520"> | <img src="docs/rp2040-zero-wiring.jpg" alt="RP2040-Zero wiring example" width="520"> |
| USB-Cポート加工例 | 型番ラベル |
| <img src="docs/hkt-4000-usb-c-port.jpg" alt="USB-C port installation example" width="520"> | <img src="docs/hkt-4000-label.jpg" alt="HKT-4000 model label" width="520"> |

写真は製作例です。写真内の中継配線色ではなく、次節の端子番号と信号名を基準に
接続してください。

## 必要なもの

- Raspberry Pi Pico、またはWaveshare RP2040-Zero
- Dreamcastキーボード HKT-4000
- Dreamcast延長ケーブルのメス側、または適合するコネクタ
- 33 Ω抵抗 2本（SDCKA/SDCKB信号線に直列）
- 0.1～0.25 A程度のリセッタブルヒューズ（5 V保護、推奨）
- 配線材、基板、データ通信対応USBケーブル
- 任意: 低容量3.3 V対応ESD保護素子

## 配線

配線図: [日本語](docs/wiring.svg) / [English](docs/wiring-en.svg)

| 端子番号 | HKT-4000信号 | 純正ケーブル線色 | Pico | RP2040-Zero | 備考 |
|---:|---|---:|---|---|---|
| 1 | SDCKA | 赤 | GPIO10（物理14番） | GP10 | 33 Ωを直列 |
| 2 | +5 V | 青 | VBUS（物理40番） | 5V | PTCヒューズを直列推奨 |
| 3 | GND | 黒 | GND | GND | GNDへ接続 |
| 4 | Sense | 緑 | GND | GND | キーボード内部でもGND接続 |
| 5 | SDCKB | 白 | GPIO11（物理15番） | GP11 | 33 Ωを直列 |

> [!WARNING]
> コネクタを見る向きによって端子1と5が左右反転します。線色だけを信用せず、電源を
> 入れる前に導通テスターで端子番号を確認してください。+5 VをGPIOや3V3端子へ接続
> するとボードを破損します。

Maple信号は3.3 Vです。待機中のGP10/GP11は内部プルアップにより約3.3 Vになります。
Dreamcast本体へ接続したまま、変換基板からも給電しないでください。

## UF2の書き込み

1. ボードをPCから外す。
2. BOOTSEL（RP2040-ZeroではBOOT）を押しながらUSBケーブルでPCへ接続する。
3. 表示された`RPI-RP2`ドライブへ、ボードに合うUF2をコピーする。
4. 自動再起動後、PCで「Dreamcast HKT-4000 Keyboard」として認識されることを確認する。
5. HKT-4000を接続し、テキストエディタで入力を確認する。

RP2040-ZeroのオンボードWS2812 RGB LEDは状態表示に使用していません。

## キーの扱い

- DreamcastのキーコードをUSB HIDキーコードとして送信します。
- S1/S2は左／右GUIキー（Windows/Commandキー）として扱います。
- S3はApplication/Menuキーとして扱います。
- 日本語固有キーはOS側を「日本語106/109」配列に設定して使用します。
- PCのNum/Caps/Scroll Lock LED状態はHKT-4000へ転送していません。
- 最大6個の通常キーと修飾キーの同時押しに対応します。

## ビルド

Pico SDK 2.3.xとARM GCCを使用します。初回構成時は固定コミットの
[DreamPicoPort](https://github.com/OrangeFox86/DreamPicoPort)を取得するため、
インターネット接続が必要です。

```sh
git clone --branch 2.3.0 https://github.com/raspberrypi/pico-sdk.git
git -C pico-sdk submodule update --init --recursive
export PICO_SDK_PATH="$PWD/pico-sdk"

# Raspberry Pi Pico
cmake -S . -B build -DPICO_BOARD=pico -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Waveshare RP2040-Zero
cmake -S . -B build-zero -DPICO_BOARD=waveshare_rp2040_zero -DCMAKE_BUILD_TYPE=Release
cmake --build build-zero -j
```

成果物:

- `build/dc_keyboard_usb.uf2`
- `build-zero/dc_keyboard_usb_rp2040_zero.uf2`

## トラブルシュート

[日本語トラブルシュート](docs/troubleshooting.ja.md)を参照してください。

## 仕組み

GPIO10/11をRP2040のPIOで駆動し、起動時にMapleのDevice Info RequestでHKT-4000を
列挙します。その後8 ms間隔でキーボード状態を取得し、8バイトのUSB HIDブート
キーボードレポートへ変換します。100 ms以上応答がない場合は押下状態を解除し、
デバイス列挙から再試行します。

## ライセンス

本プロジェクトは[MIT License](LICENSE)です。Mapleバス層にはMIT Licenseの
DreamPicoPortを使用しています。詳細は[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)
を参照してください。

## 参考資料

- [Waveshare RP2040-Zero](https://www.waveshare.com/wiki/RP2040-Zero)
- [Maple bus protocol](https://dreamcast.wiki/Maple_bus)
- [Dreamcast keyboard condition/key codes](https://mc.pp.se/dc/kbd.html)
- [KallistiOS keyboard definitions](https://kos-docs.dreamcast.wiki/keyboard_8h_source.html)
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [DreamPicoPort](https://github.com/OrangeFox86/DreamPicoPort)
