#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s1angle.h"
#include "latlng.h"
#include "point.h"
#include "interval.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> Interval::constructor;

void Interval::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Interval::New));
    Local<String> name = String::NewSymbol("S1Interval");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    // Add all prototype methods, getters and setters here.
    Nan::SetPrototypeMethod(constructor, "length", GetLength);
    Nan::SetPrototypeMethod(constructor, "hi", GetHi);
    Nan::SetPrototypeMethod(constructor, "lo", GetLo);
    Nan::SetPrototypeMethod(constructor, "center", GetCenter);
    Nan::SetPrototypeMethod(constructor, "complementLength", GetComplementCenter);
    Nan::SetPrototypeMethod(constructor, "contains", Contains);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript.
    target->Set(name, constructor->GetFunction());
}

Interval::Interval()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> Interval::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        Interval* ll = static_cast<Interval*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    if (info.Length() != 1) {
        return Nan::ThrowError("(number) required");
    }

    Interval* obj = new Interval();

    obj->Wrap(info.This());

    obj->this_ = S1Interval::FromPoint(info[0]->ToNumber()->Value());

    return info.This();
}

Handle<Value> Interval::New(S1Interval s1angle) {
    Nan::HandleScope scope;
    Interval* obj = new Interval();
    obj->this_ = s1angle;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_METHOD(Interval::GetLength) {
    Nan::HandleScope scope;
    Interval* obj = Nan::ObjectWrap::Unwrap<Interval>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.GetLength()));
}

NAN_METHOD(Interval::GetCenter) {
    Nan::HandleScope scope;
    Interval* obj = Nan::ObjectWrap::Unwrap<Interval>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.GetCenter()));
}

NAN_METHOD(Interval::GetComplementCenter) {
    Nan::HandleScope scope;
    Interval* obj = Nan::ObjectWrap::Unwrap<Interval>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.GetComplementCenter()));
}

NAN_METHOD(Interval::GetHi) {
    Nan::HandleScope scope;
    Interval* obj = Nan::ObjectWrap::Unwrap<Interval>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.hi()));
}

NAN_METHOD(Interval::GetLo) {
    Nan::HandleScope scope;
    Interval* obj = Nan::ObjectWrap::Unwrap<Interval>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.lo()));
}

NAN_METHOD(Interval::Contains) {
    Nan::HandleScope scope;
    Interval* obj = Nan::ObjectWrap::Unwrap<Interval>(info.This());
    if (info.Length() != 1) {
        Nan::ThrowError("(number) required");
    }
    if (!info[0]->IsNumber()) {
        Nan::ThrowError("(number) required");
    }
    info.GetReturnValue().Set(Nan::New<Boolean>(obj->this_.Contains(info[0]->ToNumber()->Value())));
}
