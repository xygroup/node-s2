#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s2cell.h"
#include "s2cellid.h"
#include "s2latlng.h"
#include "s2polygon.h"
#include "latlng.h"
#include "cell.h"
#include "polygon.h"
#include "point.h"
#include "cap.h"
#include "cellid.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> Polygon::constructor;

void Polygon::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Polygon::New));
    Local<String> name = String::NewSymbol("S2Polygon");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    // Nan::SetPrototypeMethod(constructor, "approxArea", ApproxArea);

    target->Set(name, constructor->GetFunction());
}

Polygon::Polygon()
    : Nan::ObjectWrap() {}

Handle<Value> Polygon::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        Polygon* ll = static_cast<Polygon*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    if (info.Length() != 1) {
        return Nan::ThrowError("(latlng) required");
    }

    Polygon* obj = new Polygon();
    obj->Wrap(info.This());

    return info.This();
}

// NAN_METHOD(Polygon::ApproxArea) {
//     Nan::HandleScope scope;
//     Polygon* obj = Nan::ObjectWrap::Unwrap<Polygon>(info.This());
//     info.GetReturnValue().Set(Nan::New<Number>(obj->this_.ApproxArea()));
// }
