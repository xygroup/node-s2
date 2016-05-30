var nativeExtension = require('../');
var assert = require('assert');


describe('native extension', function() {
  it('should export a wrapped object', function() {
    var obj = new nativeExtension.MyObject(0);
    assert.equal(obj.plusOne(), 1);
    assert.equal(obj.plusOne(), 2);
    assert.equal(obj.plusOne(), 3);
  });

  it('should export a function that returns nothing', function() {
    assert.equal(nativeExtension.nothing(), undefined);
  });

  it('should export a function that returns a string', function() {
    assert.equal(typeof nativeExtension.aString(), 'string');
  });

  it('should export a function that returns a boolean', function() {
    assert.equal(typeof nativeExtension.aBoolean(), 'boolean');
  });

  it('should export a function that returns a number', function() {
    assert.equal(typeof nativeExtension.aNumber(), 'number');
  });

  it('should export a function that returns an object', function() {
    assert.equal(typeof nativeExtension.anObject(), 'object');
  });

  it('should export a function that returns an object with a key, value pair', function() {
    assert.deepEqual(nativeExtension.anObject(), {'key': 'value'});
  });

  it('should export a function that returns an array', function() {
    assert.equal(Array.isArray(nativeExtension.anArray()), true);
  });

  it('should export a function that returns an array with some values', function() {
    assert.deepEqual(nativeExtension.anArray(), [1, 2, 3]);
  });

  it('should export a function that calls a callback', function(done) {
    nativeExtension.callback(done);
  });
  
  it('should export a function that returns a Buffer', function() {
    assert.ok(Buffer.isBuffer(nativeExtension.aBuffer()))
  });
  
  it("should export a function that calculates the distance between two points on the earth's surface", function() {
    var los_angeles = { lat: 34.0522, long: 118.2437 }
    var new_york = { lat: 40.7128, long: 74.0059 }
    var dist = nativeExtension.DistanceBetweenLocations(los_angeles.lat, los_angeles.long, new_york.lat, new_york.long)
    var actual = 3940163 // floor of actual
    // console.log(dist)
    assert(typeof dist === 'number')
    assert(dist >= actual && dist <= actual + 1)
  });

  it("should throw when calling DistanceBetweenLocations with missing arguments", function() {
    try {
      nativeExtension.DistanceBetweenLocations()
    } catch (e) {
      assert(true)
      return
    }
    assert(false)
  })

  it("should export a function that calculates the level given a desired edge length in meters", function() {
    var level = nativeExtension.GetClosestLevel(1000)
    // console.log(level)
    assert(typeof level === 'number')
    assert(level === 13)
  });

  it("should throw when calling GetClosestLevel with missing arguments", function() {
    try {
      nativeExtension.GetClosestLevel()
    } catch (e) {
      assert(true)
      return
    }
    assert(false)
  })

  it("should export a function that calculates the cell at a given location and level", function() {
    var los_angeles = { lat: 34.0522, long: 118.2437 }
    var level = nativeExtension.GetClosestLevel(1000)
    var cell = nativeExtension.CellAtLocationAndLevel(los_angeles.lat, los_angeles.long, level)
    // console.log(cell)
    assert(Buffer.isBuffer(cell))
    assert(cell.length === 8)
    assert(cell.equals(new Buffer('000000001cc3c735', 'hex')))
  });

  it("should throw when calling CellAtLocationAndLevel with missing arguments", function() {
    try {
      nativeExtension.CellAtLocationAndLevel()
    } catch (e) {
      assert(true)
      return
    }
    assert(false)
  })

});
