#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "angle.h"
#include "cap.h"
#include "cell.h"
#include "latlng.h"
#include "latlngrect.h"
#include "point.h"
#include "polyline.h"
#include "s2.h"
#include "s1angle.h"
#include "s2cell.h"
#include "s2latlngrect.h"
#include "s2polyline.h"

#include <string>
#include <vector>

using namespace v8;

Nan::Persistent<FunctionTemplate> Polyline::constructor;

void Polyline::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Polyline::New));
    Local<String> name = String::NewSymbol("S2Polyline");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    Nan::SetPrototypeMethod(constructor, "getCapBound", GetCapBound);
    Nan::SetPrototypeMethod(constructor, "getLength", GetLength);
    Nan::SetPrototypeMethod(constructor, "getCentroid", GetCentroid);
    Nan::SetPrototypeMethod(constructor, "interpolate", Interpolate);
    Nan::SetPrototypeMethod(constructor, "getSuffix", GetSuffix);
    Nan::SetPrototypeMethod(constructor, "intersects", Intersects);
    Nan::SetPrototypeMethod(constructor, "reverse", Reverse);
    Nan::SetPrototypeMethod(constructor, "approxEquals", ApproxEquals);
    Nan::SetPrototypeMethod(constructor, "nearlyCoversPolyline", NearlyCoversPolyline);
    Nan::SetPrototypeMethod(constructor, "getRectBound", GetRectBound);
    Nan::SetPrototypeMethod(constructor, "contains", Contains);
    Nan::SetPrototypeMethod(constructor, "size", Size);

    target->Set(name, constructor->GetFunction());
}

Polyline::Polyline()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> Polyline::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        Polyline* ll = static_cast<Polyline*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    if (info.Length() != 1) {
        return Nan::ThrowError("array<LatLng> required");
    }

    if (!info[0]->IsArray()) {
       return Nan::ThrowError("array<LatLng> required");
    }

    // unwrap array into std::vector
    v8::Handle<v8::Array> array = v8::Handle<v8::Array>::Cast(info[0]);
    std::vector<S2LatLng> coordinate_vector;
    for (unsigned i = 0; i < array->Length(); i++) {
        v8::Handle<v8::Value> value = array->Get(i);
        if (value->IsUndefined() || value->IsNull()) {
            return Nan::ThrowError("array entry undefined");
        }
        LatLng* ll = Nan::ObjectWrap::Unwrap<LatLng>(value->ToObject());
        coordinate_vector.emplace_back(ll->get());
        // TODO: complete

        // std::cout << "[" << i << "] " << coordinate_vector.at(i).lat() << "," << coordinate_vector.at(i).lng() << std::endl;
    }
    Polyline* obj = new Polyline();
    obj->Wrap(info.This());

    // pass std::vector<S2LatLng> to c'tor
    obj->this_ = S2Polyline(coordinate_vector);
    return info.This();
}

Handle<Value> Polyline::New(S2Polyline s2cap) {
    Nan::HandleScope scope;
    Polyline* obj = new Polyline();
    obj->this_ = s2cap;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_METHOD(Polyline::GetCapBound) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    info.GetReturnValue().Set(Cap::New(polyline->this_.GetCapBound()));
}

NAN_METHOD(Polyline::GetLength) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    info.GetReturnValue().Set(Angle::New(polyline->this_.GetLength()));
}

NAN_METHOD(Polyline::GetCentroid) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    info.GetReturnValue().Set(Point::New(polyline->this_.GetCentroid()));
}

NAN_METHOD(Polyline::Interpolate) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    info.GetReturnValue().Set(Point::New(polyline->this_.Interpolate(info[0]->ToNumber()->Value())));
}

NAN_METHOD(Polyline::GetSuffix) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    int next_vertex = info[1]->ToInteger()->Value();
    info.GetReturnValue().Set(Point::New(polyline->this_.GetSuffix(info[0]->ToNumber()->Value(),
                                                          &next_vertex)));
    // TODO: set argv[1] to next_vertex
}

NAN_METHOD(Polyline::Intersects) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    S2Polyline other = Nan::ObjectWrap::Unwrap<Polyline>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(polyline->this_.Intersects(&other)));
}

NAN_METHOD(Polyline::Reverse) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    polyline->this_.Reverse();
    return;
}

NAN_METHOD(Polyline::ApproxEquals) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    S2Polyline other = Nan::ObjectWrap::Unwrap<Polyline>(info[0]->ToObject())->get();
    double max_error = info[1]->ToNumber()->Value();
    info.GetReturnValue().Set(Nan::New<Boolean>(polyline->this_.ApproxEquals(&other, max_error)));
}

NAN_METHOD(Polyline::NearlyCoversPolyline) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    S2Polyline other = Nan::ObjectWrap::Unwrap<Polyline>(info[0]->ToObject())->get();
    S1Angle max_error =  Nan::ObjectWrap::Unwrap<Angle>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(polyline->this_.NearlyCoversPolyline(other, max_error)));
}

NAN_METHOD(Polyline::GetRectBound) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    info.GetReturnValue().Set(LatLngRect::New(polyline->this_.GetRectBound()));
}

NAN_METHOD(Polyline::Contains) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    S2Cell other = Nan::ObjectWrap::Unwrap<Cell>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(polyline->this_.Contains(other)));
}

NAN_METHOD(Polyline::Size) {
    Nan::HandleScope scope;
    Polyline* polyline = Nan::ObjectWrap::Unwrap<Polyline>(info.This());
    info.GetReturnValue().Set(Nan::New<Integer>(polyline->this_.num_vertices()));
}
