#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s2cell.h"
#include "s2latlngrect.h"
#include "latlngrect.h"
#include "latlng.h"
#include "cell.h"
#include "cap.h"
#include "cellid.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> LatLngRect::constructor;

void LatLngRect::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(LatLngRect::New));
    Local<String> name = String::NewSymbol("S2LatLngRect");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    Nan::SetPrototypeMethod(constructor, "center", GetCenter);
    Nan::SetPrototypeMethod(constructor, "size", GetSize);
    Nan::SetPrototypeMethod(constructor, "area", GetArea);
    Nan::SetPrototypeMethod(constructor, "getVertex", GetVertex);
    Nan::SetPrototypeMethod(constructor, "getCapBound", GetCapBound);
    Nan::SetPrototypeMethod(constructor, "contains", Contains);
    Nan::SetPrototypeMethod(constructor, "isValid", IsValid);
    Nan::SetPrototypeMethod(constructor, "isEmpty", IsEmpty);
    Nan::SetPrototypeMethod(constructor, "isPoint", IsPoint);
    Nan::SetPrototypeMethod(constructor, "union", Union);
    Nan::SetPrototypeMethod(constructor, "intersection", Intersection);
    Nan::SetPrototypeMethod(constructor, "approxEquals", ApproxEquals);

    target->Set(name, constructor->GetFunction());
}

LatLngRect::LatLngRect()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> LatLngRect::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        LatLngRect* ll = static_cast<LatLngRect*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    if (info.Length() == 0) {
        return Nan::ThrowError("(latlng) or (latlng, latlng) required");
    }

    LatLngRect* obj = new LatLngRect();

    obj->Wrap(info.This());


    Handle<Object> ll = info[0]->ToObject();

    if (!Nan::New(LatLng::constructor)->HasInstance( ll)) {
        return Nan::ThrowError("(latlng) required");
    }

    if (info.Length() == 1) {
        obj->this_ = S2LatLngRect(
            S2LatLngRect::FromPoint(Nan::ObjectWrap::Unwrap<LatLng>(ll)->get()));
    } else if (info.Length() == 2) {
        Handle<Object> llb = info[1]->ToObject();

        if (!Nan::New(LatLng::constructor)->HasInstance( llb)) {
            return Nan::ThrowError("(latlng, latlng) required");
        }

        obj->this_ = S2LatLngRect(
            S2LatLngRect::FromPointPair(
                Nan::ObjectWrap::Unwrap<LatLng>(ll)->get(),
                Nan::ObjectWrap::Unwrap<LatLng>(llb)->get()));
    }

    return info.This();
}

Handle<Value> LatLngRect::New(S2LatLngRect s2cell) {
    Nan::HandleScope scope;
    LatLngRect* obj = new LatLngRect();
    obj->this_ = s2cell;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_METHOD(LatLngRect::GetCenter) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    info.GetReturnValue().Set(LatLng::New(latlngrect->this_.GetCenter()));
}

NAN_METHOD(LatLngRect::GetCapBound) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    info.GetReturnValue().Set(Cap::New(latlngrect->this_.GetCapBound()));
}

NAN_METHOD(LatLngRect::GetSize) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    info.GetReturnValue().Set(LatLng::New(latlngrect->this_.GetSize()));
}

NAN_METHOD(LatLngRect::GetArea) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(latlngrect->this_.Area()));
}

NAN_METHOD(LatLngRect::GetVertex) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    info.GetReturnValue().Set(LatLng::New(latlngrect->this_.GetVertex(info[0]->ToNumber()->Value())));
}

NAN_METHOD(LatLngRect::Contains) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    S2LatLng other = Nan::ObjectWrap::Unwrap<LatLng>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(latlngrect->this_.Contains(other)));
}

NAN_METHOD(LatLngRect::IsValid) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    info.GetReturnValue().Set(Nan::New<Boolean>(latlngrect->this_.is_valid()));
}

NAN_METHOD(LatLngRect::IsEmpty) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    info.GetReturnValue().Set(Nan::New<Boolean>(latlngrect->this_.is_empty()));
}

NAN_METHOD(LatLngRect::IsPoint) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    info.GetReturnValue().Set(Nan::New<Boolean>(latlngrect->this_.is_point()));
}

NAN_METHOD(LatLngRect::Union) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    S2LatLngRect other = Nan::ObjectWrap::Unwrap<LatLngRect>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(LatLngRect::New(latlngrect->this_.Union(other)));
}

NAN_METHOD(LatLngRect::Intersection) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    S2LatLngRect other = Nan::ObjectWrap::Unwrap<LatLngRect>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(LatLngRect::New(latlngrect->this_.Intersection(other)));
}

NAN_METHOD(LatLngRect::ApproxEquals) {
    Nan::HandleScope scope;
    LatLngRect* latlngrect = Nan::ObjectWrap::Unwrap<LatLngRect>(info.This());
    S2LatLngRect other = Nan::ObjectWrap::Unwrap<LatLngRect>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(latlngrect->this_.ApproxEquals(other)));
}
