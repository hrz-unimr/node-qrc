var qrc = require('../');
for (var i = 0; i < 100000; i++) {
  qrc.encode('abcdefghijklmnopqrstuvwxyz' + i, { ecLevel: EC_H });
  if (i % 1000 === 0) {
    console.log(i);
  }
}
