#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s1angle.h"
#include "latlng.h"
#include "point.h"
#include "angle.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> Angle::constructor;

void Angle::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Angle::New));
    Local<String> name = String::NewSymbol("S1Angle");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    // Add all prototype methods, getters and setters here.
    Nan::SetPrototypeMethod(constructor, "normalize", Normalize);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript.
    target->Set(name, constructor->GetFunction());
}

Angle::Angle()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> Angle::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        Angle* ll = static_cast<Angle*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    if (info.Length() != 2) {
        return Nan::ThrowError("(point, point) required");
    }

    Angle* obj = new Angle();

    obj->Wrap(info.This());

    Handle<Object> a = info[0]->ToObject();
    Handle<Object> b = info[1]->ToObject();

    if (!Nan::New(Point::constructor)->HasInstance( a) ||
        !Nan::New(Point::constructor)->HasInstance( a)) {
        return Nan::ThrowError("(point, point) required");
    }

    obj->this_ = S1Angle(
        Nan::ObjectWrap::Unwrap<Point>(a)->get(),
        Nan::ObjectWrap::Unwrap<Point>(b)->get());

    return info.This();
}

Handle<Value> Angle::New(S1Angle s1angle) {
    Nan::HandleScope scope;
    Angle* obj = new Angle();
    obj->this_ = s1angle;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_METHOD(Angle::Normalize) {
    Nan::HandleScope scope;
    Angle* obj = Nan::ObjectWrap::Unwrap<Angle>(info.This());
    obj->this_.Normalize();
    return scope.Close(info.This());
}
