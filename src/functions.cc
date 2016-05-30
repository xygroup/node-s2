#include "functions.h"

#include "s2.h"
#include "s2cap.h"
#include "s2cellid.h"
#include "s2latlng.h"
#include "s2regioncoverer.h"

namespace {

const double kEarthCircumferenceMeters = 1000 * 40075.017;

double EarthMetersToRadians(double meters) {
  return (2 * M_PI) * (meters / kEarthCircumferenceMeters);
}

double RadiansToEarthMeters(double radians) {
  return (radians * kEarthCircumferenceMeters) / (2 * M_PI);
}

}  // namespace

NAN_METHOD(DistanceBetweenLocations) {
    bool bad_args = false;

    if (info[0]->IsUndefined()) bad_args = true;
    if (info[1]->IsUndefined()) bad_args = true;
    if (info[2]->IsUndefined()) bad_args = true;
    if (info[3]->IsUndefined()) bad_args = true;

    if (bad_args) {
        Nan::ThrowError("Invalid arguments: function requires four double arguments.");
        return;
    }

    double lat1 = Nan::To<double>(info[0]).FromJust();
    double lng1 = Nan::To<double>(info[1]).FromJust();
    double lat2 = Nan::To<double>(info[2]).FromJust();
    double lng2 = Nan::To<double>(info[3]).FromJust();

    S2LatLng latlng1 = S2LatLng::FromDegrees(lat1, lng1);
    S2LatLng latlng2 = S2LatLng::FromDegrees(lat2, lng2);

    double ret = RadiansToEarthMeters(latlng1.GetDistance(latlng2).radians());

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(CellAtLocationAndLevel) {
    bool bad_args = false;

    if (info[0]->IsUndefined()) bad_args = true;
    if (info[1]->IsUndefined()) bad_args = true;
    if (info[2]->IsUndefined()) bad_args = true;

    if (bad_args) {
        Nan::ThrowError("Invalid arguments: function requires three arguments, two doubles and an integer.");
        return;
    }

    double lat = Nan::To<double>(info[0]).FromJust();
    double lng = Nan::To<double>(info[1]).FromJust();
    uint32_t level = Nan::To<uint32_t>(info[2]).FromJust();

    S2CellId id(S2CellId::FromLatLng(S2LatLng::FromDegrees(lat, lng)));
    S2CellId ret = id.parent(level);

    auto buf = Nan::CopyBuffer(
        (char*)&ret,
        sizeof(ret)
    ).ToLocalChecked();

    info.GetReturnValue().Set(buf);
}

NAN_METHOD(GetClosestLevel) {
    bool bad_args = false;

    if (info[0]->IsUndefined()) bad_args = true;

    if (bad_args) {
        Nan::ThrowError("Invalid arguments: function requires one argument, a double.");
        return;
    }

    double meters = Nan::To<double>(info[0]).FromJust();

    uint32_t ret = S2::kAvgEdge.GetClosestLevel(EarthMetersToRadians(meters));

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(SearchRanges) {
    bool bad_args = false;

    if (info[0]->IsUndefined()) bad_args = true;
    if (info[1]->IsUndefined()) bad_args = true;
    if (info[2]->IsUndefined()) bad_args = true;
    if (info[3]->IsUndefined()) bad_args = true;
    if (info[4]->IsUndefined()) bad_args = true;

    if (bad_args) {
        Nan::ThrowError("Invalid arguments: function requires four arguments, three doubles and two ints.");
        return;
    }

    double lat = Nan::To<double>(info[0]).FromJust();
    double lng = Nan::To<double>(info[1]).FromJust();
    double radius_meters = Nan::To<double>(info[2]).FromJust();
    uint32_t min_level = Nan::To<uint32_t>(info[3]).FromJust();
    uint32_t max_level = Nan::To<uint32_t>(info[4]).FromJust();

    const double radius_radians = EarthMetersToRadians(radius_meters);
    const S2Cap region = S2Cap::FromAxisHeight(
        S2LatLng::FromDegrees(lat, lng).Normalized().ToPoint(),
        (radius_radians * radius_radians) / 2);
    S2RegionCoverer coverer;
    coverer.set_min_level(min_level);
    coverer.set_max_level(max_level);
    coverer.set_max_cells(20);

    vector<S2CellId> covering;
    coverer.GetCovering(region, &covering);

    v8::Local<v8::Array> arr = Nan::New<v8::Array>(covering.size() * 2);

    for (size_t i = 0; i < covering.size(); ++i) {
        S2CellId cell = covering[i];
        S2CellId min_cell = cell.range_min();
        S2CellId max_cell = cell.range_max();

        auto min_buf = Nan::CopyBuffer(
            (char*)&min_cell,
            sizeof(min_cell)
        ).ToLocalChecked();

        auto max_buf = Nan::CopyBuffer(
            (char*)&max_cell,
            sizeof(max_cell)
        ).ToLocalChecked();

        Nan::Set(arr, (i * 2) + 0, min_buf);
        Nan::Set(arr, (i * 2) + 1, max_buf);
    }

    info.GetReturnValue().Set(arr);
}
