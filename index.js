var qrc = require('bindings')('qrc.node');

// some convenience consts:

// consts for EC level
qrc.EC_L = 0;
qrc.EC_M = 1;
qrc.EC_Q = 2;
qrc.EC_H = 3;

// consts for mode
qrc.MODE_NUM   = 0; // numeric
qrc.MODE_AN    = 1; // alphanumeric
qrc.MODE_8     = 2; // 8-bit bytes (binary)
qrc.MODE_KANJI = 3; // Kanji

module.exports = qrc;
