#include <node.h>
#include <cstdlib>

#include "async.h"
#include "convert.h"
#include "nan.h"

using namespace v8;

// Function to execute inside the worker-thread.
// It is not safe to access V8, or V8 data structures
// here, so everything we need for input and output
// should go on our req->data object.
void AsyncWork(uv_work_t *req) {
  // fetch our data structure
  ConvertData *options = (ConvertData *)req->data;
  // run Convert() and assign the result to our data structure
  Convert(options);
}

// Function to execute when the async work is complete
// this function will be run inside the main event loop
// so it is safe to use V8 again
void AsyncAfter(uv_work_t *req) {
  HandleScope scope;

  // fetch our data structure
  ConvertData *options = (ConvertData *)req->data;
  // create an arguments array for the callback
  Handle<Value> argv[] = {
    options->errmsg ? String::New(options->errmsg) : Null()
  };

  // surround in a try/catch for safety
  TryCatch try_catch;
  // execute the callback function
  options->callback->Call(Context::GetCurrent()->Global(), 1, argv);
  if (try_catch.HasCaught())
    node::FatalException(try_catch);

  // dispose the Persistent handle so the callback
  // function can be garbage-collected
  options->callback.Dispose();
  // clean up any memory we allocated
  delete options;
  delete req;
}

// Asynchronous access to the `Convert()` function
Handle<Value> ConvertAsync(const Arguments& args) {
  HandleScope scope;

  // check argument
  if (args.Length() != 2) {
    ThrowException(Exception::TypeError(String::New("convert() requires one option argument and one callback argument!")));
    return scope.Close(Undefined());
  }

  if (!args[0]->IsObject()) {
    ThrowException(Exception::TypeError(String::New("convert()'s 1st argument should be an object")));
    return scope.Close(Undefined());
  }

  if (!args[1]->IsFunction()) {
    ThrowException(Exception::TypeError(String::New("convert()'s 2nd argument should be a callback")));
    return scope.Close(Undefined());
  }

  // create an async work token
  uv_work_t *req = new uv_work_t;

  // assign our data structure that will be passed around
  ConvertData *options = fetchConvertOptions(args);
  req->data = options;

  // expect a function as the second argument
  // we create a Persistent reference to it so
  // it won't be garbage-collected
  options->callback = Persistent<Function>::New(
      Local<Function>::Cast(args[1]));

  // pass the work token to libuv to be run when a
  // worker-thread is available to
  uv_queue_work(
    uv_default_loop(),
    req,                          // work token
    AsyncWork,                    // work function
    (uv_after_work_cb)AsyncAfter  // function to run when complete
  );

  return scope.Close(Undefined());
}
