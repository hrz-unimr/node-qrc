var qrc = require('bindings')('qrc.node');

// some convenience consts:
qrc.EC_L = 0;
qrc.EC_M = 1;
qrc.EC_Q = 2;
qrc.EC_H = 3;

module.exports = qrc;
