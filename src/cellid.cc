#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s2cell.h"
#include "s2cellid.h"
#include "s2latlng.h"
#include "latlng.h"
#include "point.h"
#include "cellid.h"
#include "strutil.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> CellId::constructor;

void CellId::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(CellId::New));
    Local<String> name = String::NewSymbol("S2CellId");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    Nan::SetPrototypeMethod(constructor, "level", Level);
    Nan::SetPrototypeMethod(constructor, "toToken", ToToken);
    Nan::SetPrototypeMethod(constructor, "fromToken", FromToken);
    Nan::SetPrototypeMethod(constructor, "toPoint", ToPoint);
    Nan::SetPrototypeMethod(constructor, "toString", ToString);
    Nan::SetPrototypeMethod(constructor, "toLatLng", ToLatLng);
    Nan::SetPrototypeMethod(constructor, "parent", Parent);
    Nan::SetPrototypeMethod(constructor, "prev", Prev);
    Nan::SetPrototypeMethod(constructor, "next", Next);
    Nan::SetPrototypeMethod(constructor, "isFace", IsFace);
    Nan::SetPrototypeMethod(constructor, "rangeMin", RangeMin);
    Nan::SetPrototypeMethod(constructor, "rangeMax", RangeMax);
    Nan::SetPrototypeMethod(constructor, "id", Id);
    Nan::SetPrototypeMethod(constructor, "child", Child);
    Nan::SetPrototypeMethod(constructor, "contains", Contains);

    target->Set(name, constructor->GetFunction());
}

CellId::CellId()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> CellId::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        CellId* cellid = static_cast<CellId*>(ptr);
        cellid->Wrap(info.This());
        return info.This();
    }

    CellId* obj = new CellId();

    obj->Wrap(info.This());

    if (info.Length() == 1) {
        Handle<Object> fromObj = info[0]->ToObject();
        if (Nan::New(Point::constructor)->HasInstance( fromObj)) {
            S2Point p = Nan::ObjectWrap::Unwrap<Point>(fromObj)->get();
            obj->this_ = S2CellId::FromPoint(p);
        } else if (Nan::New(LatLng::constructor)->HasInstance( fromObj)) {
            S2LatLng ll = Nan::ObjectWrap::Unwrap<LatLng>(fromObj)->get();
            obj->this_ = S2CellId::FromLatLng(ll);
        } else if (info[0]->IsString()) {
            std::string strnum {*NanAsciiString(info[0])};
            obj->this_ = S2CellId(ParseLeadingUInt64Value(strnum, 0));
        } else {
            return Nan::ThrowError("Invalid input");
        }
    } else {
        obj->this_ = S2CellId();
    }

    return info.This();
}

Handle<Value> CellId::New(S2CellId s2cellid) {
    Nan::HandleScope scope;
    CellId* obj = new CellId();
    obj->this_ = s2cellid;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_METHOD(CellId::FromToken) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsString()) {
        return Nan::ThrowError("(str) required");
    }
    std::string strtoken {*NanAsciiString(info[0])};
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    obj->this_ = S2CellId::FromToken(strtoken);
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(CellId::Level) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.level()));
}

NAN_METHOD(CellId::ToToken) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(Nan::New<String>(obj->this_.ToToken().c_str())).ToLocalChecked();
}

NAN_METHOD(CellId::ToString) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(Nan::New<String>(obj->this_.ToString().c_str())).ToLocalChecked();
}

NAN_METHOD(CellId::ToPoint) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(Point::New(obj->this_.ToPoint()));
}

NAN_METHOD(CellId::Parent) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    if (info.Length() == 1 && info[0]->IsNumber()) {
        info.GetReturnValue().Set(CellId::New(obj->this_.parent(info[0]->ToNumber()->Value())));
    } else {
        info.GetReturnValue().Set(CellId::New(obj->this_.parent()));
    }
}
NAN_METHOD(CellId::Prev) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(CellId::New(obj->this_.prev()));
}

NAN_METHOD(CellId::Next) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(CellId::New(obj->this_.next()));
}

NAN_METHOD(CellId::IsFace) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(Nan::New<Boolean>(obj->this_.is_face()));
}

NAN_METHOD(CellId::RangeMin) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(CellId::New(obj->this_.range_min()));
}

NAN_METHOD(CellId::RangeMax) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(CellId::New(obj->this_.range_max()));
}

NAN_METHOD(CellId::Contains) {
    Nan::HandleScope scope;
    CellId* cellid = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    S2CellId other = Nan::ObjectWrap::Unwrap<CellId>(info[0]->ToObject())->get();
    info.GetReturnValue().Set(Nan::New<Boolean>(cellid->this_.contains(other)));
}

NAN_METHOD(CellId::Id) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(Number::New(obj->this_.id()));
}

NAN_METHOD(CellId::Child) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    if (info.Length() != 1) {
        return Nan::ThrowError("(number) required");
    }
    info.GetReturnValue().Set(CellId::New(obj->this_.child(info[0]->ToNumber()->Value())));
}

NAN_METHOD(CellId::ToLatLng) {
    Nan::HandleScope scope;
    CellId* obj = Nan::ObjectWrap::Unwrap<CellId>(info.This());
    info.GetReturnValue().Set(LatLng::New(obj->this_.ToLatLng()));
}
