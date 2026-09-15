# Dreamcast HKT-4000 → USB keyboard converter (Raspberry Pi Pico)

Dreamcast keyboard HKT-4000を、Raspberry Pi Pico（RP2040）経由でPC用USB
HIDキーボードとして使うためのファームウェアです。

ビルド済みUF2: [dist/dc_keyboard_usb.uf2](dist/dc_keyboard_usb.uf2)

MapleバスをGPIO 10/11のPIOでポーリングし、受け取った8バイトの
キーボード状態をPicoのmicro-USB端子からUSB HIDとして送ります。Dreamcastの
修飾キービットとキーコードはUSB HIDキーボードと同じ配置なので、文字コードへの
変換は行いません。日本語版／英語版の物理配列はPC側のキーボード配列設定で選びます。

## 必要なもの

- Raspberry Pi Pico（通常版RP2040。Pico Wでは未確認）
- Dreamcastキーボード HKT-4000
- Dreamcast延長ケーブルのメス側、または適合するメスコネクタ
- 33 Ω抵抗 2本（信号線直列）
- 0.1～0.25 A程度のリセッタブルヒューズ 1個（5 V電源保護、推奨）
- ブレッドボード、ユニバーサル基板、配線材など
- データ通信対応micro-USBケーブル
- 任意: 低容量3.3 V対応ESD保護素子（信号線からGNDへ）

## 配線

詳細図: [docs/wiring.svg](docs/wiring.svg)

| HKT-4000側 | 一般的な線色 | Pico側 | 備考 |
|---|---:|---|---|
| +5 V | 赤 | VBUS（物理40番） | 0.1～0.25 A PTCヒューズを直列 |
| SDCKA | 緑 | GPIO10（物理14番） | 33 Ωを直列 |
| SDCKB | 青 | GPIO11（物理15番） | 33 Ωを直列 |
| GND / Sense | 白 | GND | GNDへ接続 |
| GND | 黒 | GND | GNDへ接続 |

**線色だけを信用せず、必ず導通テスターでコネクタ端子との対応を確認してください。**
ケーブルのメーカーや切断位置によって線色が異なります。Maple信号は3.3 V TTLなので
レベル変換器は不要です。+5 VをGPIOまたは3V3ピンへ接続するとPicoを破損します。

PicoのVBUSはPCのUSB 5 Vです。この構成ではDreamcast本体と同時接続しません。
HKT-4000をDreamcastへ接続したまま、Pico側からも給電しないでください。

## ビルド

Pico SDK 2.3.xとARM GCCを用意します。WindowsではRaspberry Pi Pico用VS Code
Extensionを使うのが簡単です。コマンドラインでは次のようにビルドします。

```sh
git clone --branch 2.3.0 https://github.com/raspberrypi/pico-sdk.git
git -C pico-sdk submodule update --init --recursive
export PICO_SDK_PATH="$PWD/pico-sdk"

cmake -S . -B build -DPICO_BOARD=pico
cmake --build build -j
```

初回構成時にMaple PIO実装
[DreamPicoPort](https://github.com/OrangeFox86/DreamPicoPort) の固定コミットを
取得するため、インターネット接続が必要です。成果物は
`build/dc_keyboard_usb.uf2` です。

検証済みのビルド済みファイルは`dist/dc_keyboard_usb.uf2`にも置いてあります。

## Picoへの書き込み

1. PicoをPCから外す。
2. PicoのBOOTSELボタンを押したままmicro-USBケーブルでPCへ接続する。
3. `RPI-RP2`ドライブへ`dist/dc_keyboard_usb.uf2`（または自分でビルドした
   `build/dc_keyboard_usb.uf2`）をコピーする。
4. Picoが自動再起動したらHKT-4000をメスコネクタへ接続する。
5. PCのキーボードテスト画面で入力を確認する。

HKT-4000とのMaple通信が成立するとPicoのオンボードLEDが点灯します。100 ms以上
応答が途絶えると、押下中のキーをすべて解除してLEDを消灯します。

## キーの扱い

- 英数、ファンクション、カーソル、テンキーなどはHIDキーコードをそのまま送信します。
- S1/S2はそれぞれ左／右GUIキー（Windowsキー、Commandキー）として扱われます。
- S3はApplication/Menuキーとして扱われます。
- 日本語固有キーもHID usageをそのまま送ります。OS側を日本語キーボード配列に設定します。
- HKT-4000側のLED状態バイトはUSB予約バイトに置換します。PCからのLock LED出力は
  HKT-4000へ転送していません。

## トラブルシュート

- LEDが点灯しない: SDCKA/SDCKBの入れ替わり、GND、5 V、コネクタ向きを確認。
- USBキーボードとして認識しない: データ対応USBケーブルを使用し、UF2を書き直す。
- 一部キーの印字と入力が違う: OSの配列を「日本語106/109」または「US」に合わせる。
- 入力が不安定: 配線を短くし、信号抵抗を10～33 Ωの範囲で試す。100 Ω以上は避ける。
- Pico W: オンボードLED制御が通常Picoと異なるため、本コードは通常版Picoを対象とする。

## 技術資料とライセンス

- [Maple bus protocol overview](https://dreamcast.wiki/Maple_bus)
- [Dreamcast keyboard condition/key codes](https://mc.pp.se/dc/kbd.html)
- [KallistiOS keyboard definitions](https://kos-docs.dreamcast.wiki/keyboard_8h_source.html)
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [DreamPicoPort](https://github.com/OrangeFox86/DreamPicoPort)

このプロジェクトはMIT Licenseです。Mapleバス層はMIT LicenseのDreamPicoPortを
ビルド時に使用します。詳細は[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)を参照してください。
