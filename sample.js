var magick = require('./build/Release/imagemagick.node');

// for each batch of work, request an async Estimate() for
// a portion of the total number of calculations
option = {
    srcPath: "/Users/rakusai/Work/Gyazo/node-imagemagick-native/test/test.jpg",
    outPath: "/Users/rakusai/Work/Gyazo/node-imagemagick-native/test/out.jpg",
    width: 48,
    height: 48,
    resizeStyle: 'aspectfit',
    quality: 80,
    format: 'JPEG',
    debug: 1
};

magick.convert(option, function (err) {
  if (err) {
    console.log(err);
    return;
  }
  console.log("success!");

});
console.log("end");
