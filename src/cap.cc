#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s2cap.h"
#include "s2latlngrect.h"
#include "point.h"
#include "cap.h"
#include "latlngrect.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> Cap::constructor;

void Cap::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Cap::New));
    Local<String> name = String::NewSymbol("S2Cap");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    Nan::SetPrototypeMethod(constructor, "getRectBound", GetRectBound);
    Nan::SetPrototypeMethod(constructor, "intersects", Intersects);
    Nan::SetPrototypeMethod(constructor, "interiorIntersects", InteriorIntersects);
    Nan::SetPrototypeMethod(constructor, "contains", Contains);
    Nan::SetPrototypeMethod(constructor, "complement", Complement);

    target->Set(name, constructor->GetFunction());
}

Cap::Cap()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> Cap::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        Cap* ll = static_cast<Cap*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    Cap* obj = new Cap();

    obj->Wrap(info.This());

    if (info.Length() == 2 && info[1]->IsNumber()) {
        Handle<Object> fromObj = info[0]->ToObject();
        if (Nan::New(Point::constructor)->HasInstance( fromObj)) {
            S2Point p = Nan::ObjectWrap::Unwrap<Point>(fromObj)->get();
            obj->this_ = S2Cap::FromAxisHeight(p, info[1]->ToNumber()->Value());
        } else {
            return Nan::ThrowError("S2Cap requires arguments (S2Point, number)");
        }
    } else {
        return Nan::ThrowError("S2Cap requires arguments (S2Point, number)");
    }

    return info.This();
}

Handle<Value> Cap::New(S2Cap s2cap) {
    Nan::HandleScope scope;
    Cap* obj = new Cap();
    obj->this_ = s2cap;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_METHOD(Cap::GetRectBound) {
    Nan::HandleScope scope;
    Cap* cap = Nan::ObjectWrap::Unwrap<Cap>(info.This());
    info.GetReturnValue().Set(LatLngRect::New(cap->this_.GetRectBound()));
}

NAN_METHOD(Cap::Intersects) {
    Nan::HandleScope scope;
    Cap* cap = Nan::ObjectWrap::Unwrap<Cap>(info.This());
    S2Cap other = Nan::ObjectWrap::Unwrap<Cap>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(cap->this_.Intersects(other)));
}

NAN_METHOD(Cap::InteriorIntersects) {
    Nan::HandleScope scope;
    Cap* cap = Nan::ObjectWrap::Unwrap<Cap>(info.This());
    S2Cap other = Nan::ObjectWrap::Unwrap<Cap>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(cap->this_.Intersects(other)));
}

NAN_METHOD(Cap::Contains) {
    Nan::HandleScope scope;
    Cap* cap = Nan::ObjectWrap::Unwrap<Cap>(info.This());
    S2Cap other = Nan::ObjectWrap::Unwrap<Cap>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(cap->this_.Contains(other)));
}

NAN_METHOD(Cap::Complement) {
    Nan::HandleScope scope;
    Cap* cap = Nan::ObjectWrap::Unwrap<Cap>(info.This());
    info.GetReturnValue().Set(Cap::New(cap->this_.Complement()));
}
