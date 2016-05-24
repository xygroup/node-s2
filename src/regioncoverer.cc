#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s2cell.h"
#include "s2cellid.h"
#include "s2latlng.h"
#include "s2regioncoverer.h"
#include "latlng.h"
#include "cell.h"
#include "point.h"
#include "regioncoverer.h"
#include "cap.h"
#include "cellid.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> RegionCoverer::constructor;

void RegionCoverer::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(RegionCoverer::New));
    Local<String> name = String::NewSymbol("S2RegionCoverer");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    // Nan::SetPrototypeMethod(constructor, "getCenter", GetCenter);

    target->Set(name, constructor->GetFunction());
}

RegionCoverer::RegionCoverer()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> RegionCoverer::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        RegionCoverer* ll = static_cast<RegionCoverer*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    RegionCoverer* obj = new RegionCoverer();

    obj->Wrap(info.This());

    Handle<Object> ll = info[0]->ToObject();

    S2RegionCoverer cov;

    obj->this_ = cov;

    return info.This();
}

Handle<Value> RegionCoverer::New(S2RegionCoverer rc) {
    Nan::HandleScope scope;
    RegionCoverer* obj = new RegionCoverer();
    obj->this_ = rc;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}
