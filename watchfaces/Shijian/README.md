# Shijian for Watchy

![Shijian screenshot](/screenshot/Shijian.gif?raw=true)

## About
**Shijian** is a simple aesthetic Chinese watchface for [Watchy](https://watchy.sqfmi.com/). It is modified from [Stationary Text](https://github.com/BraininaBowl/Stationary-Text-for-Watchy).

* It only tells the time, so only NTP settings are sufficient for syncing.
* The characters for the hour are in uppercase (大写) to fit with the style.
* Only the files in the `src` folder are required for deployment.
* The font only supports *Simplified Chinese*, so there won't be a *Traditional Chinese* version.

## Customisation
Modifying the logic is very simple:
* To switch numbers between uppercase and lowercase, just replace `uc` in the array names with `lc` or vice versa.
* To use light mode, just swap all instances of `GxEPD_WHITE` with `GxEPD_BLACK`.

Modifying the glyphs can be pretty complicated:
* All fonts are in the `font` folder.
* Most uppercase English letters were replaced with Chinese glyphs during the conversion to GFX.
* If you need those uppercase characters back, you'll need to convert the fonts yourself.

To convert the glyphs, [fontconvert](https://github.com/adafruit/Adafruit-GFX-Library/tree/master/fontconvert) was implemented in Python with [freetype-py](https://pypi.org/project/freetype-py/). If there's interest, I'll open-source the script.

### Modified Glyphs
| ASCII | Original | Replaced | Translation |
| - | - | - | - |
| `0x41` | A | 零 | 0 |
| `0x42` | B | 一 | 1 (lower) |
| `0x43` | C | 二 | 2 (lower) |
| `0x44` | D | 三 | 3 (lower) |
| `0x45` | E | 四 | 4 (lower) |
| `0x46` | F | 五 | 5 (lower) |
| `0x47` | G | 六 | 6 (lower) |
| `0x48` | H | 七 | 7 (lower) |
| `0x49` | I | 八 | 8 (lower) |
| `0x4A` | J | 九 | 9 (lower) |
| `0x4B` | K | 十 | 10 (lower) |
| `0x4C` | L | 壹 | 1 (upper) |
| `0x4D` | M | 贰 | 2 (upper) |
| `0x4E` | N | 叁 | 3 (upper) |
| `0x4F` | O | 肆 | 4 (upper) |
| `0x50` | P | 伍 | 5 (upper) |
| `0x51` | Q | 陆 | 6 (upper) |
| `0x52` | R | 柒 | 7 (upper) |
| `0x53` | S | 捌 | 8 (upper) |
| `0x54` | T | 玖 | 9 (upper) |
| `0x55` | U | 拾 | 10 (upper) |
| `0x56` | V | 时 | Hours |
| `0x57` | W | 分 | Minutes |
| `0x58` | X | 整 | Sharp |

## Other
Special thanks to [Pconti31](https://github.com/Pconti31) for his [TTF2GFX](https://github.com/Pconti31/TTF2GFX) library.
