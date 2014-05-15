#include <Magick++.h>
#include <unistd.h>
#include <exception>

#include "convert.h"
#include "nan.h"

const char* getStringFromObject(Local<Object> obj, const char* key, const char* defaultString) {

  if (obj->Get(String::New(key))->IsUndefined())
    return defaultString;

  size_t size;
  return NanCString(obj->Get(String::New(key))->ToString(), &size);
}

ConvertData * fetchConvertOptions(const Arguments& args){

  ConvertData *options = new ConvertData();

  // expect a number as the first argument
  Local<Object> obj = Local<Object>::Cast(args[0]);

  options->debug = obj->Get(String::New("debug"))->Uint32Value();

  options->width = obj->Get(String::New("width"))->Uint32Value();
  options->height = obj->Get(String::New("height"))->Uint32Value();
  options->quality = obj->Get(String::New("quality"))->Uint32Value();

  strcpy(options->srcPath, getStringFromObject(obj, "srcPath", ""));
  strcpy(options->outPath, getStringFromObject(obj, "outPath", ""));

  strcpy(options->format, getStringFromObject(obj, "format", ""));
  strcpy(options->resizeStyle, getStringFromObject(obj, "resizeStyle", "aspectfit"));

  if (options->debug){
    printf( "srcPath: %s\n", options->srcPath );
    printf( "outPath: %s\n", options->outPath );
    printf( "width: %d\n", options->width );
    printf( "height: %d\n", options->height );
    printf( "quality: %d\n", options->quality );
    printf( "format: %s\n", options->format );
    printf( "resizeStyle: %s\n", options->resizeStyle );
  }
  return options;
}

bool Convert (ConvertData* option) {

  sleep(1);
  if (option->debug) printf("Convert\n");

  Magick::Image image;
  try {
    image.read(option->srcPath);
  } catch (std::exception& err) {
    std::string message = "image.read failed with error: ";
    message            += err.what();
    strcpy(option->errmsg, message.c_str());
    return false;
  } catch (...) {
    strcpy(option->errmsg, "image.read failed with unhandled error");
    return false;
  }

  if (option->format && strcmp( option->format, "" ) != 0)
    image.magick(option->format);
  if (option->debug)
    printf( "format: %s\n", option->format );

  if (option->debug)
    printf("original width,height: %d, %d\n", (int) image.columns(), (int) image.rows());

  int width = option->width;
  int height = option->height;

  if (option->width || option->height) {
    if (!width)
      width  = image.columns();
    if (!height)
      height = image.rows();

    // do resize
    if ( strcmp( option->resizeStyle, "aspectfill" ) == 0 ) {
      // ^ : Fill Area Flag ('^' flag)
      // is not implemented in Magick++
      // and gravity: center, extent doesnt look like working as exptected
      // so we do it ourselves

      // keep aspect ratio, get the exact provided size, crop top/bottom or left/right if necessary
      double aspectratioExpected = (double)height / (double)width;
      double aspectratioOriginal = (double)image.rows() / (double)image.columns();
      unsigned int xoffset = 0;
      unsigned int yoffset = 0;
      unsigned int resizewidth;
      unsigned int resizeheight;

      if ( aspectratioExpected > aspectratioOriginal ) {
        // expected is taller
        resizewidth  = (unsigned int)( (double)height / (double)image.rows() * (double)image.columns() + 1. );
        resizeheight = height;
        xoffset      = (unsigned int)( (resizewidth - width) / 2. );
        yoffset      = 0;
      } else {
        // expected is wider
        resizewidth  = width;
        resizeheight = (unsigned int)( (double)width / (double)image.columns() * (double)image.rows() + 1. );
        xoffset      = 0;
        yoffset      = (unsigned int)( (resizeheight - height) / 2. );
      }

      if (option->debug)
        printf("resize to: %d, %d\n", resizewidth, resizeheight);
      Magick::Geometry resizeGeometry(resizewidth, resizeheight, 0, 0, 0, 0);
      image.resize(resizeGeometry);

      // limit canvas size to cropGeometry
      if (option->debug)
        printf( "crop to: %d, %d, %d, %d\n", width, height, xoffset, yoffset );
      Magick::Geometry cropGeometry( width, height, xoffset, yoffset, 0, 0 );

      Magick::Color transparent("white");
      if (option->format) {
        // make background transparent for PNG
        // JPEG background becomes black if set transparent here
        transparent.alpha(1.);
      }
      image.extent( cropGeometry, transparent );
    } else if (strcmp (option->resizeStyle, "aspectfit") == 0 ) {
      // keep aspect ratio, get the maximum image which fits inside specified size
      char geometryString[32];
      sprintf( geometryString, "%dx%d", width, height );
      if (option->debug)
        printf( "resize to: %s\n", geometryString );
      image.resize(geometryString);
    } else if (strcmp (option->resizeStyle, "fill") == 0) {
      // change aspect ratio and fill specified size
      char geometryString[32];
      sprintf( geometryString, "%dx%d!", width, height );
      if (option->debug)
        printf( "resize to: %s\n", geometryString );
      image.resize(geometryString);
    } else {
      //this->errmsg = "resizeStyle not supported";
      return false;
    }

    if (option->debug)
      printf( "resized to: %d, %d\n", (int)image.columns(), (int)image.rows() );
  }

  if (option->quality) {
    if (option->debug)
      printf("quality: %d\n", option->quality);
    image.quality(option->quality);
  }

  image.write(option->outPath);

  return true;
}
