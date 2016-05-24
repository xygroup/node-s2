#include <node.h>
#include <nan.h>
#include <memory>
#include <vector>
#include "node_object_wrap.h"  // for Nan::ObjectWrap
#include "v8.h"                // for Handle, String, Integer, etc

#include "s2latlng.h"
#include "s2cellid.h"
#include "s2cell.h"
#include "s2polygon.h"
#include "s2loop.h"
#include "s2polygonbuilder.h"
#include "s2regioncoverer.h"
#include "s2polyline.h"
#include "latlng.h"
#include "latlngrect.h"
#include "cell.h"
#include "cellid.h"
#include "interval.h"
#include "point.h"
#include "polyline.h"
#include "cap.h"
#include "regioncoverer.h"
#include "angle.h"

using namespace v8;

struct CoverConfiguration {
  CoverConfiguration()
      : min_level(std::numeric_limits<int>::min()),
        max_level(std::numeric_limits<int>::min()),
        level_mod(std::numeric_limits<int>::min()),
        max_cells(std::numeric_limits<int>::min()),
        type("polygon"),
        result_type("cell") {}
  int min_level;
  int max_level;
  int level_mod;
  int max_cells;
  std::string type;
  std::string result_type;
  S2Polygon polygon;
  std::shared_ptr<S2Polyline> polyline;
  S2LatLngRect rect;
  S2Cell cell;
  S2Cap cap;
};

class CoverWorker : public Nan::AsyncWorker {
 private:
  std::vector<S2CellId> cellids_vector;

 public:
  CoverWorker(Nan::Callback *callback,
              std::shared_ptr<CoverConfiguration> coverConfiguration)
      : Nan::AsyncWorker(callback), coverConfiguration(coverConfiguration) {}
  ~CoverWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute() {
    S2RegionCoverer coverer;
    if (coverConfiguration->min_level != std::numeric_limits<int>::min()) {
      coverer.set_min_level(coverConfiguration->level_mod);
    }
    if (coverConfiguration->max_level != std::numeric_limits<int>::min()) {
      coverer.set_max_level(coverConfiguration->max_level);
    }
    if (coverConfiguration->level_mod != std::numeric_limits<int>::min()) {
      coverer.set_level_mod(coverConfiguration->level_mod);
    }
    if (coverConfiguration->max_cells != std::numeric_limits<int>::min()) {
      coverer.set_max_cells(coverConfiguration->max_cells);
    }

    // check for types and call appropriate getCovering function
    if ("polygon" == coverConfiguration->type ||
        "multipolygon" == coverConfiguration->type) {
      coverer.GetCovering(coverConfiguration->polygon, &cellids_vector);
    } else if ("polyline" == coverConfiguration->type) {
      coverer.GetCovering((*coverConfiguration->polyline), &cellids_vector);
    } else if ("rect" == coverConfiguration->type) {
      coverer.GetCovering(coverConfiguration->rect, &cellids_vector);
    } else if ("cell" == coverConfiguration->type) {
      coverer.GetCovering(coverConfiguration->cell, &cellids_vector);
    } else if ("cap" == coverConfiguration->type) {
      coverer.GetCovering(coverConfiguration->cap, &cellids_vector);
    } else {
    }
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback() {
    Nan::HandleScope scope;

    Local<Array> out = Array::New(cellids_vector.size());
    if (coverConfiguration->result_type == "cell") {
      for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
        out->Set(i, Cell::New(cellids_vector.at(i)));
      }
    } else if (coverConfiguration->result_type == "cellId") {
      for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
        out->Set(i, CellId::New(cellids_vector.at(i)));
      }
    } else if (coverConfiguration->result_type == "string") {
      for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
        out->Set(i, Nan::New<String>(cellids_vector.at(i).ToString())).ToLocalChecked();
      }
    } else if (coverConfiguration->result_type == "token") {
      for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
        out->Set(i, Nan::New<String>(cellids_vector.at(i).ToToken())).ToLocalChecked();
      }
    } else if (coverConfiguration->result_type == "point") {
            for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
        out->Set(i, Point::New(cellids_vector.at(i).ToPoint()));
      }
    }
    if (coverConfiguration->type == "undefined") {
      v8::Local<v8::Value> argv[] = {Exception::Error(String::New("cover type not specified")), Nan::Null()};
      callback->Call(2, argv);
    } else {
      v8::Local<v8::Value> argv[] = {Nan::Null(), out};
      callback->Call(2, argv);
    }
  }

  void HandleErrorCallback() {
    Nan::HandleScope scope;
    Local<Value> argv[] = {Exception::Error(String::New("fail!"))};
    callback->Call(1, argv);
  };

 private:
  std::shared_ptr<CoverConfiguration> coverConfiguration;
};

NAN_METHOD(GetCover) {
  Nan::HandleScope scope;

  if (info.Length() < 1) {
    return Nan::ThrowError("(array, [min, max, mod]) required");
  }

  typedef std::vector<pair<S2Point, S2Point>> EdgeList;
  std::shared_ptr<CoverConfiguration> coverConfiguration =
      std::make_shared<CoverConfiguration>();

  if (info.Length() > 2) {
    Handle<Object> opt = info[1]->ToObject();
    if (opt->Has(Nan::New<String>("min").ToLocalChecked())) {
      coverConfiguration->min_level =
          opt->Get(Nan::New<String>("min").ToLocalChecked())->ToInteger()->Value();
    }
    if (opt->Has(Nan::New<String>("max").ToLocalChecked())) {
      coverConfiguration->max_level =
          opt->Get(Nan::New<String>("max").ToLocalChecked())->ToInteger()->Value();
    }
    if (opt->Has(Nan::New<String>("mod").ToLocalChecked())) {
      coverConfiguration->level_mod =
          opt->Get(Nan::New<String>("mod").ToLocalChecked())->ToInteger()->Value();
    }
    if (opt->Has(Nan::New<String>("max_cells").ToLocalChecked())) {
      coverConfiguration->max_cells =
          opt->Get(Nan::New<String>("max_cells").ToLocalChecked())->ToInteger()->Value();
    }
    if (opt->Has(Nan::New<String>("type").ToLocalChecked())) {
      coverConfiguration->type =
          *NanAsciiString(opt->Get(Nan::New<String>("type").ToLocalChecked()));
    }
    if (opt->Has(Nan::New<String>("result_type").ToLocalChecked())) {
      coverConfiguration->result_type =
          *NanAsciiString(opt->Get(Nan::New<String>("result_type").ToLocalChecked()));
    }
  }

  // Check the number of arguments. we need size-1 as the callback always goes
  // last
  Nan::Callback *callback =
      new Nan::Callback(info[info.Length() - 1].As<Function>());
  if (info[0]->IsArray()) {
    Handle<Array> array = Handle<Array>::Cast(info[0]);

    if (coverConfiguration->type == "polygon") {
      S2PolygonBuilderOptions polyOptions;
      polyOptions.set_validate(true);
      // Don't silently eliminate duplicate edges.
      polyOptions.set_xor_edges(false);
      S2PolygonBuilder builder(polyOptions);

      std::vector<S2Point> rings;
      for (std::size_t i = 0; i < array->Length(); ++i) {
        std::vector<S2Point> points;
        Handle<Array> pointArray = Handle<Array>::Cast(array->Get(i));

        for (std::size_t ii = 0; ii < pointArray->Length(); ++ii) {
          Local<Object> obj = pointArray->Get(ii)->ToObject();
          if (Nan::New(Point::constructor)->HasInstance( obj)) {
            S2Point p = Nan::ObjectWrap::Unwrap<Point>(obj)->get();
            points.push_back(p);
          } else {
            return Nan::ThrowError("array must contain only points");
          }
        }
        // construct polygon loop
        S2Loop loop(points);
        loop.Normalize();

        // outer ring should not be a hole
        if (i == 0 && loop.is_hole()) {
          loop.Invert();
        }
        // inner rings should be holes
        else if (i == 1 && !loop.is_hole()) {
          loop.Invert();
        }
        if (!loop.IsValid()) {
          return Nan::ThrowError("invalid loop");
        }
        builder.AddLoop(&loop);
      }
      // get cover
      EdgeList edgeList;
      builder.AssemblePolygon(&coverConfiguration->polygon, &edgeList);
    } else if (coverConfiguration->type == "polyline") {
      std::vector<S2Point> points;
      for (std::size_t i = 0; i < array->Length(); ++i) {
        Local<Object> obj = array->Get(i)->ToObject();
        if (Nan::New(Point::constructor)->HasInstance( obj)) {
          S2Point p =
              Nan::ObjectWrap::Unwrap<Point>(array->Get(i)->ToObject())->get();
          points.push_back(p);
        } else {
          return Nan::ThrowError("array must contain only points");
        }
      }
      coverConfiguration->polyline = std::make_shared<S2Polyline>(points);
    } else if (coverConfiguration->type == "multipolygon") {
      S2PolygonBuilderOptions polyOptions;
      polyOptions.set_validate(true);
      // Don't silently eliminate duplicate edges.
      polyOptions.set_xor_edges(false);
      S2PolygonBuilder builder(polyOptions);
      std::vector<S2Point> rings;

      for (std::size_t k = 0; k < array->Length(); ++k) {
        Handle<Array> ringArray = Handle<Array>::Cast(array->Get(k));
        for (std::size_t i = 0; i < ringArray->Length(); ++i) {
          std::vector<S2Point> points;
          Handle<Array> pointArray = Handle<Array>::Cast(ringArray->Get(i));
          for (std::size_t ii = 0; ii < pointArray->Length(); ++ii) {
            Local<Object> obj = pointArray->Get(ii)->ToObject();
            if (Nan::New(Point::constructor)->HasInstance( obj)) {
              S2Point p = Nan::ObjectWrap::Unwrap<Point>(obj)->get();
              points.push_back(p);
            } else {
              return Nan::ThrowError("array must contain only points");
            }
          }
          // construct polygon loop
          S2Loop loop(points);
          loop.Normalize();

          // outer ring should not be a hole
          if (i == 0 && loop.is_hole()) {
            loop.Invert();
          }
          // inner rings should be holes
          else if (i == 1 && !loop.is_hole()) {
            loop.Invert();
          }
          if (!loop.IsValid()) {
            return Nan::ThrowError("invalid loop");
          }
          builder.AddLoop(&loop);
        }
      }
      // get cover
      EdgeList edgeList;
      builder.AssemblePolygon(&coverConfiguration->polygon, &edgeList);
    }
  } else if (Nan::New(LatLngRect::constructor)->HasInstance( info[0])) {
    coverConfiguration->type = "rect";
    coverConfiguration->rect =
        Nan::ObjectWrap::Unwrap<LatLngRect>(info[0]->ToObject())->get();
  } else if (Nan::New(Cell::constructor)->HasInstance( info[0])) {
    coverConfiguration->type = "cell";
    coverConfiguration->cell =
        Nan::ObjectWrap::Unwrap<Cell>(info[0]->ToObject())->get();
  } else if (Nan::New(Cap::constructor)->HasInstance( info[0])) {
    coverConfiguration->type = "cap";
    coverConfiguration->cap =
        Nan::ObjectWrap::Unwrap<Cap>(info[0]->ToObject())->get();
  } else {
    coverConfiguration->type = "undefined";
  }

  Nan::AsyncQueueWorker(new CoverWorker(callback, coverConfiguration));
  return;
}

NAN_METHOD(GetCoverSync) {
  Nan::HandleScope scope;

  if (info.Length() < 1) {
    return Nan::ThrowError("(array, [min, max, mod]) required");
  }

  typedef std::vector<pair<S2Point, S2Point>> EdgeList;
  std::vector<S2CellId> cellids_vector;
  S2RegionCoverer coverer;

  std::string type{"polygon"};
  std::string result_type{"cell"};

  if (info.Length() > 1) {
    Handle<Object> opt = info[1]->ToObject();
    if (opt->Has(Nan::New<String>("min").ToLocalChecked())) {
      coverer.set_min_level(
          opt->Get(Nan::New<String>("min").ToLocalChecked())->ToInteger()->Value());
    }
    if (opt->Has(Nan::New<String>("max").ToLocalChecked())) {
      coverer.set_max_level(
          opt->Get(Nan::New<String>("max").ToLocalChecked())->ToInteger()->Value());
    }
    if (opt->Has(Nan::New<String>("mod").ToLocalChecked())) {
      coverer.set_level_mod(
          opt->Get(Nan::New<String>("mod").ToLocalChecked())->ToInteger()->Value());
    }
    if (opt->Has(Nan::New<String>("max_cells").ToLocalChecked())) {
      coverer.set_max_cells(
          opt->Get(Nan::New<String>("max_cells").ToLocalChecked())->ToInteger()->Value());
    }
    if (opt->Has(Nan::New<String>("type").ToLocalChecked())) {
      type = *NanAsciiString(opt->Get(Nan::New<String>("type").ToLocalChecked()));
    }
    if (opt->Has(Nan::New<String>("result_type").ToLocalChecked())) {
      result_type = *NanAsciiString(opt->Get(Nan::New<String>("result_type").ToLocalChecked()));
    }
  }

  if (info[0]->IsArray()) {
    Handle<Array> array = Handle<Array>::Cast(info[0]);

    if (type == "polygon") {
      S2PolygonBuilderOptions polyOptions;
      polyOptions.set_validate(true);
      // Don't silently eliminate duplicate edges.
      polyOptions.set_xor_edges(false);
      S2PolygonBuilder builder(polyOptions);
      S2Polygon polygon;
      std::vector<S2Point> rings;

      for (std::size_t i = 0; i < array->Length(); ++i) {
        std::vector<S2Point> points;
        Handle<Array> pointArray = Handle<Array>::Cast(array->Get(i));

        for (std::size_t ii = 0; ii < pointArray->Length(); ++ii) {
          Local<Object> obj = pointArray->Get(ii)->ToObject();
          if (Nan::New(Point::constructor)->HasInstance( obj)) {
            S2Point p = Nan::ObjectWrap::Unwrap<Point>(obj)->get();
            points.push_back(p);
          } else {
            return Nan::ThrowError("array must contain only points");
          }
        }
        // construct polygon loop
        S2Loop loop(points);
        loop.Normalize();

        // outer ring should not be a hole
        if (i == 0 && loop.is_hole()) {
          loop.Invert();
        }
        // inner rings should be holes
        else if (i == 1 && !loop.is_hole()) {
          loop.Invert();
        }

        if (!loop.IsValid()) {
          return Nan::ThrowError("invalid loop");
        }

        builder.AddLoop(&loop);
      }
      // get cover
      EdgeList edgeList;
      builder.AssemblePolygon(&polygon, &edgeList);
      coverer.GetCovering(polygon, &cellids_vector);
    } else if (type == "polyline") {
      std::vector<S2Point> points;
      for (std::size_t i = 0; i < array->Length(); ++i) {
        Local<Object> obj = array->Get(i)->ToObject();
        if (Nan::New(Point::constructor)->HasInstance( obj)) {
          S2Point p =
              Nan::ObjectWrap::Unwrap<Point>(array->Get(i)->ToObject())->get();
          points.push_back(p);
        } else {
          return Nan::ThrowError("array must contain only points");
        }
      }
      S2Polyline polyline(points);
      coverer.GetCovering(polyline, &cellids_vector);
    } else if (type == "multipolygon") {
      S2PolygonBuilderOptions polyOptions;
      polyOptions.set_validate(true);
      // Don't silently eliminate duplicate edges.
      polyOptions.set_xor_edges(false);
      S2PolygonBuilder builder(polyOptions);
      S2Polygon polygon;
      std::vector<S2Point> rings;

      for (std::size_t k = 0; k < array->Length(); ++k) {
        Handle<Array> ringArray = Handle<Array>::Cast(array->Get(k));
        for (std::size_t i = 0; i < ringArray->Length(); ++i) {
          std::vector<S2Point> points;
          Handle<Array> pointArray = Handle<Array>::Cast(ringArray->Get(i));
          for (std::size_t ii = 0; ii < pointArray->Length(); ++ii) {
            Local<Object> obj = pointArray->Get(ii)->ToObject();
            if (Nan::New(Point::constructor)->HasInstance( obj)) {
              S2Point p = Nan::ObjectWrap::Unwrap<Point>(obj)->get();
              points.push_back(p);
            } else {
              return Nan::ThrowError("array must contain only points");
            }
          }
          // construct polygon loop
          S2Loop loop(points);
          loop.Normalize();

          // outer ring should not be a hole
          if (i == 0 && loop.is_hole()) {
            loop.Invert();
          }
          // inner rings should be holes
          else if (i == 1 && !loop.is_hole()) {
            loop.Invert();
          }

          if (!loop.IsValid()) {
            return Nan::ThrowError("invalid loop");
          }

          builder.AddLoop(&loop);
        }
      }
      // get cover
      EdgeList edgeList;
      builder.AssemblePolygon(&polygon, &edgeList);
      coverer.GetCovering(polygon, &cellids_vector);
    }
  } else if (Nan::New(LatLngRect::constructor)->HasInstance( info[0])) {
    S2LatLngRect rect =
        Nan::ObjectWrap::Unwrap<LatLngRect>(info[0]->ToObject())->get();
    coverer.GetCovering(rect, &cellids_vector);
  } else if (Nan::New(Cell::constructor)->HasInstance( info[0])) {
    S2Cell cell = Nan::ObjectWrap::Unwrap<Cell>(info[0]->ToObject())->get();
    coverer.GetCovering(cell, &cellids_vector);
  } else if (Nan::New(Cap::constructor)->HasInstance( info[0])) {
    S2Cap cap = Nan::ObjectWrap::Unwrap<Cap>(info[0]->ToObject())->get();
    coverer.GetCovering(cap, &cellids_vector);
  } else {
    return Nan::ThrowError("incompatible object to cover");
  }

  Local<Array> out = Array::New(cellids_vector.size());
  if (result_type == "cell") {
    for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
      out->Set(i, Cell::New(cellids_vector.at(i)));
    }
  } else if (result_type == "cellId") {
    for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
      out->Set(i, CellId::New(cellids_vector.at(i)));
    }
  } else if (result_type == "string") {
    for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
      out->Set(i, Nan::New<String>(cellids_vector.at(i).ToString())).ToLocalChecked();
    }
  } else if (result_type == "token") {
    for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
      out->Set(i, Nan::New<String>(cellids_vector.at(i).ToToken())).ToLocalChecked();
    }
  } else if (result_type == "point") {
    for (std::size_t i = 0; i < cellids_vector.size(); ++i) {
      out->Set(i, Point::New(cellids_vector.at(i).ToPoint()));
    }
  }

  info.GetReturnValue().Set(out);
}

void RegisterModule(Handle<Object> exports) {
  LatLng::Init(exports);
  LatLngRect::Init(exports);
  Cap::Init(exports);
  Angle::Init(exports);
  Cell::Init(exports);
  CellId::Init(exports);
  Point::Init(exports);
  Interval::Init(exports);
  Polyline::Init(exports);
  exports->Set(Nan::New<String>("getCoverSync").ToLocalChecked(),
               Nan::New<FunctionTemplate>(GetCoverSync)->GetFunction());
  exports->Set(Nan::New<String>("getCover").ToLocalChecked(),
               Nan::New<FunctionTemplate>(GetCover)->GetFunction());
}

NODE_MODULE(_s2, RegisterModule);
