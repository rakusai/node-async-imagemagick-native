#include <node.h>

#include "async.h"

using namespace v8;

// Expose synchronous and asynchronous access to our
// Estimate() function
void InitAll(Handle<Object> exports) {
  exports->Set(String::NewSymbol("convert"),
      FunctionTemplate::New(ConvertAsync)->GetFunction());
}

NODE_MODULE(imagemagick, InitAll)
