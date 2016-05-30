var nativeExtension = require('../');
var assert = require('assert');

describe('native extension', function() {
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

  it("should export a function that calculates the cell ranges to search for a given lat/long, distance, and min/max level combination", function() {
    var los_angeles = { lat: 34.0522, long: 118.2437 }
    var min_level = nativeExtension.GetClosestLevel(100000)
    var max_level = nativeExtension.GetClosestLevel(1000)
    // console.log('min_level', min_level)
    // console.log('max_level', max_level)
    var distance = 5000
    var ranges = nativeExtension.SearchRanges(los_angeles.lat, los_angeles.long, distance, min_level, max_level)
    // console.log(ranges.length)
    // console.log(ranges)
    assert(Array.isArray(ranges))
    assert(ranges.length <= 40)
    ranges.forEach(function(cell) {
      assert(Buffer.isBuffer(cell))
      assert(cell.length === 8)
    })
  });

  it("should throw when calling SearchRanges with missing arguments", function() {
    try {
      nativeExtension.SearchRanges()
    } catch (e) {
      assert(true)
      return
    }
    assert(false)
  })

});
