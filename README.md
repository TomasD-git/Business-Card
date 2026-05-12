# Business-Card

> A custom dev board built inside a business card — NFC, e-paper display and 16 exposed GPIO pins.

<img width="1377" height="810" alt="image" src="https://github.com/user-attachments/assets/9895b63e-7bb1-41f4-9194-0653230a006d" />


---

## Features

- E-paper display — Configure once image stays
- NFC — tap to share Contact info, a URL, or any custom NFC
- 16 exposed GPIO pins — Connector makes it easy to use for another project
- No power design - Once flashed and configured, nfc works with no external power and image stays on e-paper screen

---

## How It Works

The Business Card is a fully custom PCB designed. The microcontroller drives both the e-paper display and the NFC IC over SPI/I2C, while the 16 GPIO pins are broken out along one edge for easy prototyping.

On power-up, the firmware initialises the display and NFC tag, can be configured, the e-paper holds whatever was last written without needing a continuous refresh.

---

## Schematic Overview

The schematic is split into four main blocks:

<img width="1230" height="836" alt="image" src="https://github.com/user-attachments/assets/bb51ca10-9997-4cd6-8ee1-0b612fb8dcd1" />


**MCU**: Core microcontroller, configuration of nfc and e-paper, GPIO pins   
**E-paper interface**: Has 8 exposed pins to solder an E-paper onto the PCB  
**NFC**: NFC IC with antenna matching network, presents as a standard NDEF tag  
**Power**: Power regulation with decoupling  

---

## PCB

Designed in KiCad, manufactured as a standard 2-layer board with PCBA. Gerber files and BOM are in the repo if you want to order your own.

<img width="974" height="608" alt="image" src="https://github.com/user-attachments/assets/ba7ea24b-0097-4600-a364-39905848a561" />

---

<details>
  <summary>BOM</summary>

| Name | Purpose | Quantity | Total Cost (USD) | Link | Distributor |
|------|---------|----------|-----------------|------|-------------|
| locknut | secures the screen in place with M2 bolts | 1 | 4.88 | [Link](https://www.aliexpress.com/item/32963044301.html) | Aliexpress |
| PCB + PCBA | PCB with PCBA | 2 | 73.27 | [Link](https://jlcpcb.com) | jlcpcb |
| E-Paper, 2.13" | Screen to see my name | 1 | 12.44 | [Link](https://www.aliexpress.com/item/1005005183232092.html) | Aliexpress |

</details>

<details>
  <summary>For Future</summary>

## Planned improvements

**User interface**    
Change firmware so it could be configured with web or custom application.  

**Custom CAD**  
Some type of CAD to protect mcu.  

</details>

<details>
  <summary>License</summary>

### MIT License

**Copyright (c) [2026] [TomasD-git]**

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

</details>
