#include "functions.h"

using v8::FunctionTemplate;

// NativeExtension.cc represents the top level of the module.
// C++ constructs that are exposed to javascript are exported here

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("DistanceBetweenLocations").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(DistanceBetweenLocations)).ToLocalChecked());
  Nan::Set(target, Nan::New("CellAtLocationAndLevel").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(CellAtLocationAndLevel)).ToLocalChecked());
  Nan::Set(target, Nan::New("GetClosestLevel").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GetClosestLevel)).ToLocalChecked());
  Nan::Set(target, Nan::New("SearchRanges").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(SearchRanges)).ToLocalChecked());
}

NODE_MODULE(NativeExtension, InitAll)
