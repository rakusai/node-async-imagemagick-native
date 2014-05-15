#include <node.h>
#include <Magick++.h>

using namespace v8;

// libuv allows us to pass around a pointer to an arbitrary
// object when running asynchronous functions. We create a
// data structure to hold the data we need during and after
// the async work.
typedef struct ConvertData {
  int points;                    // estimation points
  double estimate;               // estimation result
  char srcPath[255];
  char outPath[255];
  unsigned int width;
  unsigned int height;
  unsigned int quality;
  char format[30];
  char resizeStyle[30];
  int debug;
  char errmsg[100];
  Persistent<Function> callback; // callback function
} ConvertData;

bool Convert (ConvertData* convertData);
ConvertData * fetchConvertOptions(const Arguments& args);
