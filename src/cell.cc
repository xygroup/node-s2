#include <node.h>
#include <nan.h>
#include "node_object_wrap.h"           // for Nan::ObjectWrap
#include "v8.h"                         // for Handle, String, Integer, etc

#include "s2.h"
#include "s2cell.h"
#include "s2cellid.h"
#include "s2latlng.h"
#include "latlng.h"
#include "cell.h"
#include "point.h"
#include "cap.h"
#include "cellid.h"

using namespace v8;

Nan::Persistent<FunctionTemplate> Cell::constructor;

void Cell::Init(Handle<Object> target) {
    Nan::HandleScope scope;

    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(Cell::New));
    Local<String> name = String::NewSymbol("S2Cell");

    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    Nan::SetPrototypeMethod(constructor, "approxArea", ApproxArea);
    Nan::SetPrototypeMethod(constructor, "exactArea", ExactArea);
    Nan::SetPrototypeMethod(constructor, "averageArea", AverageArea);
    Nan::SetPrototypeMethod(constructor, "face", Face);
    Nan::SetPrototypeMethod(constructor, "level", Level);
    Nan::SetPrototypeMethod(constructor, "orientation", Orientation);
    Nan::SetPrototypeMethod(constructor, "isLeaf", IsLeaf);
    Nan::SetPrototypeMethod(constructor, "getCapBound", GetCapBound);
    Nan::SetPrototypeMethod(constructor, "getVertex", GetVertex);
    Nan::SetPrototypeMethod(constructor, "getCenter", GetCenter);
    Nan::SetPrototypeMethod(constructor, "id", Id);
    Nan::SetPrototypeMethod(constructor, "toString", ToString);

    target->Set(name, constructor->GetFunction());
}

Cell::Cell()
    : Nan::ObjectWrap(),
      this_() {}

Handle<Value> Cell::New(const Arguments& args) {
    Nan::HandleScope scope;

    if (!info.IsConstructCall()) {
        return Nan::ThrowError("Use the new operator to create instances of this object.");
    }

    if (info[0]->IsExternal()) {
        Local<External> ext = Local<External>::Cast(info[0]);
        void* ptr = ext->Value();
        Cell* ll = static_cast<Cell*>(ptr);
        ll->Wrap(info.This());
        return info.This();
    }

    if (info.Length() != 1) {
        return Nan::ThrowError("(latlng) required");
    }

    Cell* obj = new Cell();

    obj->Wrap(info.This());

    Handle<Object> ll = info[0]->ToObject();

    if (Nan::New(LatLng::constructor)->HasInstance( ll)) {
        obj->this_ = S2Cell(
            S2CellId::FromLatLng(Nan::ObjectWrap::Unwrap<LatLng>(ll)->get()));
    } else if (Nan::New(CellId::constructor)->HasInstance( ll)) {
        obj->this_ = S2Cell(Nan::ObjectWrap::Unwrap<CellId>(ll)->get());
    } else {
        return Nan::ThrowError("(latlng) or (cellid) required");
    }

    return info.This();
}

Handle<Value> Cell::New(S2Cell s2cell) {
    Nan::HandleScope scope;
    Cell* obj = new Cell();
    obj->this_ = s2cell;
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

Handle<Value> Cell::New(S2CellId s2cellid) {
    Nan::HandleScope scope;
    Cell* obj = new Cell();
    obj->this_ = S2Cell(s2cellid);
    Handle<Value> ext = External::New(obj);
    Handle<Object> handleObject = constructor->GetFunction()->NewInstance(1, &ext);
    return scope.Close(handleObject);
}

NAN_METHOD(Cell::ApproxArea) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.ApproxArea()));
}

NAN_METHOD(Cell::ExactArea) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.ExactArea()));
}

NAN_METHOD(Cell::AverageArea) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.AverageArea(info[0]->ToNumber()->Value())));
}

NAN_METHOD(Cell::Face) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.face()));
}

NAN_METHOD(Cell::Level) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.level()));
}

NAN_METHOD(Cell::Orientation) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Nan::New<Number>(obj->this_.orientation()));
}

NAN_METHOD(Cell::IsLeaf) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Nan::New<Boolean>(obj->this_.is_leaf()));
}

NAN_METHOD(Cell::GetCapBound) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Cap::New(obj->this_.GetCapBound()));
}

NAN_METHOD(Cell::GetCenter) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Point::New(obj->this_.GetCenterRaw()));
}

NAN_METHOD(Cell::Id) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(CellId::New(obj->this_.id()));
}

NAN_METHOD(Cell::ToString) {
    Nan::HandleScope scope;
    Cell* obj = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Nan::New<String>(obj->this_.id().ToString().c_str())).ToLocalChecked();
}

NAN_METHOD(Cell::GetVertex) {
    Nan::HandleScope scope;
    Cell* cell = Nan::ObjectWrap::Unwrap<Cell>(info.This());
    info.GetReturnValue().Set(Point::New(cell->this_.GetVertex(info[0]->ToNumber()->Value())));
}
