# cbtimer

Codebusters Timer - code for a custom built 7-segment LED device using addressable LED strips and an ESP-32 processor

## Hardware components

* [ESP32-S2-Saola-1](https://www.amazon.com/ESP32-S2-Saola-Dev-Featuring-WROVER/dp/B08PCPP729) development board.
* [YX5200](https://www.amazon.com/dp/B08H4NF3FV) MP3 Player board
* Speaker
* Button
* [Addressable LED strip](https://www.amazon.com/dp/B088FJF9XD).  This gets cut into 7 LED strips taped to the board and wires between the strips.

## Wiring

There is not a lot of wires between the components, the majority of the work is between the cut LED strips.

| **ESP-32** | **YX5200** | **LED Strip** | **Speaker** | **Button** |
|------------|------------|---------------|-------------|------------|
| 21 <span style="background:#000;color:#fff;padding:2px;">GND</span> | 7 <span style="background:#000;color:#fff;padding:2px;">GND</span> | <span style="background:#000;color:#fff;padding:2px;">GND</span> |             | One Side   |
| 20 <span style="background:#f00;color:#fff;padding:2px;">5V0</span> | 1 <span style="background:#f00;color:#fff;padding:2px;">VCC</span> | <span style="background:#f00;color:#fff;padding:2px;">+5</span>  |             |            |
| 11 <span style="background:#cfc;color:#000;padding:2px;">GPIO9</span> |            | <span style="background:#cfc;color:#000;padding:2px;">Data In</span> |             |            |
| 19 <span style="background:#cfc;color:#000;padding:2px;">GPIO17</span> | 3 <span style="background:#cfc;color:#000;padding:2px;">TX</span> |               |             |            |
| 18 <span style="background:#cfc;color:#000;padding:2px;">GPIO16</span> | 2 <span style="background:#cfc;color:#000;padding:2px;">RX</span> (through a 1K resistor) |   |             |            |
|            | 6 <span style="padding:2px;">SPK1</span> |               | One side    |            |
|            | 8 <span style="padding:2px;">SPK1</span> |               | Other side  |            |
| 17 <span style="background:#cfc;color:#000;padding:2px;">GPIO15</span> |            |               |             | Other Side |
