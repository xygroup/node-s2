#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s2latlng.h"
#include "latlng.h"
#include "point.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> LatLng::constructor;

void LatLng::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LatLng::New));
    Local<String> name = String::NewSymbol("S2LatLng");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    // Add all prototype methods, getters and setters here.
    Nan::SetPrototypeMethod(constructor, "normalized", Normalized);
    Nan::SetPrototypeMethod(constructor, "isValid", IsValid);
    Nan::SetPrototypeMethod(constructor, "toPoint", ToPoint);
    Nan::SetPrototypeMethod(constructor, "distance", Distance);
    Nan::SetPrototypeMethod(constructor, "toString", ToString);

    Local<ObjectTemplate> proto = constructor->PrototypeTemplate();
    Nan::SetAccessor(proto, Nan::New<String>("lat").ToLocalChecked(), Lat);
    Nan::SetAccessor(proto, Nan::New<String>("lng").ToLocalChecked(), Lng);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript.
    target->Set(name, constructor->GetFunction());
}

LatLng::LatLng()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> LatLng::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        LatLng* ll = static_cast<LatLng*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    LatLng* obj = new LatLng();
    obj->Wrap(info.This());

    if (info.Length() == 2) {
        if (info[0]->IsNumber() &&
            info[1]->IsNumber()) {
            obj->this_ = S2LatLng::FromDegrees(
                info[0]->ToNumber()->Value(),
                info[1]->ToNumber()->Value());
        }
    } else if (info.Length() == 1) {
        Handle<Object> fromObj = info[0]->ToObject();
        if (Nan::New(Point::constructor)->HasInstance( fromObj)) {
            S2Point p = Nan::ObjectWrap::Unwrap<Point>(fromObj)->get();
            obj->this_ = S2LatLng(p);
        } else {
            return Nan::ThrowError("Use the new operator to create instances of this object.");
        }
    }

    return info.This();
}

Handle<Value> LatLng::New(S2LatLng s2latlng) {
    Nan::HandleScope scope;
    LatLng* obj = new LatLng();
    obj->this_ = s2latlng;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_GETTER(LatLng::Lat) {
    Nan::HandleScope scope;
    LatLng* obj = Nan::ObjectWrap::Unwrap<LatLng>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.lat().degrees()));
}

NAN_GETTER(LatLng::Lng) {
    Nan::HandleScope scope;
    LatLng* obj = Nan::ObjectWrap::Unwrap<LatLng>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.lng().degrees()));
}

NAN_METHOD(LatLng::IsValid) {
    Nan::HandleScope scope;
    LatLng* obj = Nan::ObjectWrap::Unwrap<LatLng>(info.This());
    info.GetReturnValue().Set(Nan::New<Boolean>(obj->this_.is_valid()));
}

NAN_METHOD(LatLng::Normalized) {
    Nan::HandleScope scope;
    LatLng* obj = Nan::ObjectWrap::Unwrap<LatLng>(info.This());
    return scope.Close(LatLng::New(obj->this_.Normalized()));
}

NAN_METHOD(LatLng::ToPoint) {
    Nan::HandleScope scope;
    LatLng* obj = Nan::ObjectWrap::Unwrap<LatLng>(info.This());
    return scope.Close(Point::New(obj->this_.ToPoint()));
}

NAN_METHOD(LatLng::Distance) {
    Nan::HandleScope scope;
    LatLng* latlng = Nan::ObjectWrap::Unwrap<LatLng>(info.This());
    S2LatLng other = Nan::ObjectWrap::Unwrap<LatLng>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Number>(latlng->this_.GetDistance(other).degrees()));
}

NAN_METHOD(LatLng::ToString) {
    Nan::HandleScope scope;
    LatLng* latlng = Nan::ObjectWrap::Unwrap<LatLng>(info.This());
    info.GetReturnValue().Set(Nan::New<String>(latlng->this_.ToStringInDegrees().c_str())).ToLocalChecked();
}
