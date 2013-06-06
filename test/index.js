/* global suite: false, test: false */
var assert = require('assert');
var crypto = require('crypto');
var qrc = require('../');

suite('Validation of input params', function () {
  test('version', function () {
    assert.throws(
      function () {
        qrc.encode('TEST', { version: -1 });
      },
      /Version out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { version: 41 });
      },
      /Version out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { version: 1.5 });
      },
      /Wrong type for version/
    );
    assert.doesNotThrow(
      function () {
        qrc.encode('TEST', { version: 1 });
      }
    );
  });

  test('EC level', function () {
    assert.throws(
      function () {
        qrc.encode('TEST', { ecLevel: qrc.EC_L - 1 });
      },
      /EC level out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { ecLevel: qrc.EC_H + 1 });
      },
      /EC level out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { ecLevel: qrc.EC_L + 0.5 });
      },
      /Wrong type for EC level/
    );
    assert.doesNotThrow(
      function () {
        qrc.encode('TEST', { ecLevel: qrc.EC_H });
      }
    );
  });

  test('dot size', function () {
    assert.throws(
      function () {
        qrc.encode('TEST', { dotSize: 0 });
      },
      /Dot size out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { dotSize: 51 });
      },
      /Dot size out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { dotSize: 10.5 });
      },
      /Wrong type for dot size/
    );
    assert.doesNotThrow(
      function () {
        qrc.encode('TEST', { dotSize: 10 });
      }
    );
  });

  test('margin', function () {
    assert.throws(
      function () {
        qrc.encode('TEST', { margin: -1 });
      },
      /Margin size out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { margin: 11 });
      },
      /Margin size out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { margin: 5.5 });
      },
      /Wrong type for margin size/
    );
    assert.doesNotThrow(
      function () {
        qrc.encode('TEST', { margin: 5 });
      }
    );
  });

  test('foreground & background color', function () {
    assert.throws(
      function () {
        qrc.encode('TEST', { foregroundColor: -1 });
      },
      /Wrong type for foreground color/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { foregroundColor: 0xFFFFFF + 1 });
      },
      /Foreground color out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { foregroundColor: 5.5 });
      },
      /Wrong type for foreground color/
    );
    assert.doesNotThrow(
      function () {
        qrc.encode('TEST', { foregroundColor: 0x000000 });
      }
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { backgroundColor: -1 });
      },
      /Wrong type for background color/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { backgroundColor: 0xFFFFFF + 1 });
      },
      /Background color out of range/
    );
    assert.throws(
      function () {
        qrc.encode('TEST', { backgroundColor: 5.5 });
      },
      /Wrong type for background color/
    );
    assert.doesNotThrow(
      function () {
        qrc.encode('TEST', { backgroundColor: 0xFFFFFF });
      }
    );
  });
});

suite('Encode to buffer', function () {
  test('baseline', function() {
    var act = qrc.encode('TEST');
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 1);
    assert.strictEqual(act.width, 21);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      '3a9bf8d82bbfd0f826371d0decbbbacc32e20c40');
  });

  test('explicitly set version', function() {
    var act = qrc.encode('TEST', { version: 4 });
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 4);
    assert.strictEqual(act.width, 33);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      '861b0aa7dae66323e005a55771e8555cfed694cd');
  });

  test('explicitly set EC level', function() {
    var act = qrc.encode('TEST', { ecLevel: qrc.EC_H });
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 1);
    assert.strictEqual(act.width, 21);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      'dc645b387c5c8189bde0606ef5c6d85880d67fbc');
  });
});

suite('Encode to PNG buffer', function () {
  test('baseline', function() {
    var act = qrc.encodePng('TEST');
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 1);
    assert.strictEqual(act.width, 21);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      '39e9a557be36ad5643f542ad5ec6e316e8b9d6bb');
  });

  test('explicitly set version', function() {
    var act = qrc.encodePng('TEST', { version: 4 });
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 4);
    assert.strictEqual(act.width, 33);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      '1eeea344b6dc8ff975eeab7a661a383a10757748');
  });

  test('explicitly set EC level', function() {
    var act = qrc.encodePng('TEST', { ecLevel: qrc.EC_H });
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 1);
    assert.strictEqual(act.width, 21);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      '907541542df208453ad40e357fa7d9ed98d208ae');
  });

  test('explicitly set dot size', function() {
    var act = qrc.encodePng('TEST', { dotSize: 10 });
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 1);
    assert.strictEqual(act.width, 21);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      'db7f7aac2af9bb25b2e46e638ce853fdee926e60');
  });

  test('explicitly set margin', function() {
    var act = qrc.encodePng('TEST', { margin: 2 });
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 1);
    assert.strictEqual(act.width, 21);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      'f7577f482a1ce2fe2f8a3630a847d82729c4c635');
  });

  test('explicitly set colors', function() {
    var act = qrc.encodePng('TEST', {
      foregroundColor: 0xFF0000,
      backgroundColor: 0x00FF00
    });
    var hash = crypto.createHash('sha1');
    assert.strictEqual(act.version, 1);
    assert.strictEqual(act.width, 21);
    hash.update(act.data);
    assert.strictEqual(hash.digest('hex'),
      '25e59cae0949a01cd4d7651dc4b41c9ab8891a06');
  });
});
