node-qrc
========
QR Code generation in Node.js using libqrencode and libpng

Requirements
------------
- [libpng](http://www.libpng.org/pub/png/libpng.html)
- [libqrencode](http://fukuchi.org/works/qrencode/)

Installation
------------
1) Install libpng(-dev) and libqrencode(-dev) using the package manager of your
choice.

2) `npm install qrc`

Usage
-----

    var qrc = require('qrc');

    var qrBuffer = qrc.encode('Some text to put in a QR Code');
    // or:
    var qrPngBuffer = qrc.encodePng('Some text to put in a QR Code PNG');

    // of course there are some options:
    var qrPngBuffer = qrc.encodePng('Test', {
      version: 4,
      ecLevel: qrc.EC_H,
      dotSize: 5,
      margin: 2,
      foregroundColor: 0xFF0000,
      backgroundColor: 0x00FF00
    });

### Options

`version` – *Minimum* version of QR Code, valid values: 1-41, 0 = auto
[default]

`ecLevel` – error correction level, valid values: EC_L (lowest [default]) –
EC_M – EC_Q - EC_H (highest)

`mode` – QR code mode, valid values: MODE_NUM (numeral)– MODE_AN (alphanumeric)–
MODE_8 (8-bit binary [default])- MODE_KANJI (kanji)

`dotSize`* – Size of one ‚dot‘ in pixels, valid values: 1-50
(default: 3)

`margin`* – Size of margin (in dots with background color),
valid values: 0-10 (default: 4)

`foregroundColor`* – Foreground color, valid values:
0x0-0xFFFFFF (default: 0x0 [= black])

`backgroundColor`* – Background color, valid values: 0x0-0xFFFFFF (default:
0xFFFFFF [= white])

\* = PNG encoding only

Legal
-----
QR Code is a registered trademark of
[DENSO WAVE INCORPORATED](http://www.denso-wave.com/en/).

License
-------
Copyright (C) 2013 Tobias Muellerleile <muellerleile@hrz.uni-marburg.de>

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 2.1 of the License, or any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this library; if not, write to the Free Software Foundation, Inc., 51
Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
