#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"
#include "v8.h"

#include "s2.h"
#include "point.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> Point::constructor;

void Point::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Point::New));
    Local<String> name = String::NewSymbol("S2Point");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    Nan::SetPrototypeMethod(constructor, "x", X);
    Nan::SetPrototypeMethod(constructor, "y", Y);
    Nan::SetPrototypeMethod(constructor, "z", Z);

    target->Set(name, constructor->GetFunction());
}

Point::Point()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> Point::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        Point* p = static_cast<Point*>(ptr);
        p->Wrap(info.This());
        return info.This();
    }

    if (info.Length() != 3) {
        return Nan::ThrowError("(number, number, number) required");
    }

    Point* obj = new Point();

    obj->Wrap(info.This());

    obj->this_ = S2Point(
        info[0]->ToNumber()->Value(),
        info[1]->ToNumber()->Value(),
        info[2]->ToNumber()->Value());

    return info.This();
}

Handle<Value> Point::New(S2Point s2cell) {
    Nan::HandleScope scope;
    Point* obj = new Point();
    obj->this_ = s2cell;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_METHOD(Point::X) {
    Nan::HandleScope scope;
    Point* obj = Nan::ObjectWrap::Unwrap<Point>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.x()));
}

NAN_METHOD(Point::Y) {
    Nan::HandleScope scope;
    Point* obj = Nan::ObjectWrap::Unwrap<Point>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.y()));
}

NAN_METHOD(Point::Z) {
    Nan::HandleScope scope;
    Point* obj = Nan::ObjectWrap::Unwrap<Point>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.z()));
}
