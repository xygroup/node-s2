#ifndef NODES2POINT_HPP
#define NODES2POINT_HPP

#include <node.h>
#include <nan.h>
#include "s2.h"

class Point : public Nan::ObjectWrap {
public:
    static v8::Persistent<v8::FunctionTemplate> constructor;
    static void Init(v8::Handle<v8::Object> target);
    inline S2Point get() { return this_; }
    static v8::Handle<v8::Value> New(const S2Point c);

protected:
    Point();

    static v8::Handle<v8::Value> New(Nan::NAN_METHOD_ARGS_TYPE args);
    static NAN_METHOD(X);
    static NAN_METHOD(Y);
    static NAN_METHOD(Z);

    S2Point this_;
};

#endif
