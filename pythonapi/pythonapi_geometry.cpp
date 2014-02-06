#include "../../IlwisCore/core/kernel.h"
#include "../../IlwisCore/core/ilwisobjects/ilwisdata.h"

#include "../../IlwisCore/core/util/geometries.h"
#include "../../IlwisCore/core/ilwisobjects/geometry/coordinatesystem/coordinatesystem.h"
#include "geos/io/ParseException.h"
#include "../../IlwisCore/core/ilwisobjects/coverage/geometryhelper.h"
#include "../../IlwisCore/core/util/angle.h"

#include "geos/geom/Geometry.h"
#include "geos/geom/GeometryFactory.h"
#include "geos/geom/Coordinate.h"
#include "../../IlwisCore/core/util/location.h"
#include "../../IlwisCore/core/util/coordinate.h"
#include "../../IlwisCore/core/util/box.h"

#include "../../IlwisCore/core/ilwisobjects/geometry/coordinatesystem/coordinatesystem.h"
#include "../../IlwisCore/core/ilwisobjects/coverage/geometryhelper.h"

#include "../../IlwisCore/core/ilwisobjects/domain/domain.h"
#include "../../IlwisCore/core/ilwisobjects/domain/datadefinition.h"
#include "../../IlwisCore/core/ilwisobjects/table/columndefinition.h"
#include "../../IlwisCore/core/ilwisobjects/table/table.h"
#include "../../IlwisCore/core/ilwisobjects/table/attributerecord.h"

#include "../../IlwisCore/core/ilwisobjects/coverage/feature.h"

#include "pythonapi_geometry.h"
#include "pythonapi_feature.h"

namespace pythonapi{

Geometry::Geometry(std::string wkt, const CoordinateSystem& csy):
    _standalone(true),
    _feature(nullptr),
    _index(-1),
    _ilwisGeometry(Ilwis::GeometryHelper::fromWKT(wkt)){
    Ilwis::GeometryHelper::setCoordinateSystem(this->_ilwisGeometry.get(),csy.ptr()->get<Ilwis::CoordinateSystem>().ptr());
}

Geometry::Geometry(geos::geom::Geometry* geometry, const Ilwis::ICoordinateSystem& csy):
    _standalone(true),
    _feature(nullptr),
    _index(-1),
    _ilwisGeometry(geometry){
    Ilwis::GeometryHelper::setCoordinateSystem(this->_ilwisGeometry.get(),csy.ptr());
}

Geometry::Geometry(Feature* feature, int index):
    _standalone(false),
    _feature(feature),
    _index(index),
    _ilwisGeometry(nullptr){
}

Geometry::~Geometry(){
}

bool Geometry::__bool__() const{
    if (this->_standalone){
        return this->_ilwisGeometry != nullptr && (bool)this->_ilwisGeometry && this->_ilwisGeometry->isValid();
    }else{
        return this->_feature != nullptr && (bool)this->_feature && this->_feature->__bool__() && (bool)this->_feature->ptr()->geometry(this->_index) && this->_feature->ptr()->geometry(this->_index)->isValid();
    }
}

std::string Geometry::__str__(){
    if (this->__bool__())
        return this->toWKT();
    else
        if (this->_standalone)
            return "invalid Geometry!";
        else
            return "invalid standalone Geometry!";
}

IlwisTypes Geometry::ilwisType(){
    return Ilwis::GeometryHelper::geometryType(this->ptr().get());
}

bool Geometry::fromWKT(const std::string& wkt){
    if (!this->__bool__()){
        if (this->_standalone){
            this->_standalone = true;
            this->_feature = nullptr;
            this->_index = -1;
            this->_ilwisGeometry.reset(Ilwis::GeometryHelper::fromWKT(wkt));
            return this->_ilwisGeometry->isValid();
        }else{
            if (this->_feature != nullptr && (bool)this->_feature && this->_feature->__bool__()){
                this->_feature->ptr()->set(Ilwis::GeometryHelper::fromWKT(wkt), this->_index);
                return this->_feature->ptr()->geometry(this->_index)->isValid();
            }else
                throw InvalidObject("invalid referenc to hosting feature of non-standalone geometry!");
        }
    }else{
        if (_standalone){
            this->_ilwisGeometry.reset(Ilwis::GeometryHelper::fromWKT(wkt));
            return this->_ilwisGeometry->isValid();
        }else{
            this->_feature->ptr()->set(Ilwis::GeometryHelper::fromWKT(wkt), this->_index);
            return this->_feature->ptr()->geometry(this->_index)->isValid();
        }
    }
}

std::string Geometry::toWKT(){
    return Ilwis::GeometryHelper::toWKT(this->ptr().get());
}

CoordinateSystem Geometry::coordinateSystem(){
    return CoordinateSystem(new Ilwis::ICoordinateSystem(Ilwis::GeometryHelper::getCoordinateSystem(this->ptr().get())));
}

void Geometry::setCoordinateSystem(const CoordinateSystem &cs){
    Ilwis::GeometryHelper::setCoordinateSystem(this->ptr().get(), cs.ptr()->get<Ilwis::CoordinateSystem>().ptr());
}

Geometry* Geometry::transform(const CoordinateSystem &cs){
    geos::geom::Geometry* copy = this->ptr()->clone();
    Ilwis::GeometryHelper::transform(copy,coordinateSystem().ptr()->get<Ilwis::CoordinateSystem>(), cs.ptr()->get<Ilwis::CoordinateSystem>());
    return new Geometry(copy, cs.ptr()->get<Ilwis::CoordinateSystem>());
}

Envelope Geometry::envelope(){
    return Envelope(this->ptr()->getEnvelopeInternal());
}

bool Geometry::isSimple() const{
    return this->ptr()->isSimple();
}

bool Geometry::within(const Geometry &geometry) const{
    return this->ptr()->within(geometry.ptr().get());
}

bool Geometry::contains(const Geometry &geometry) const{
    return this->ptr()->contains(geometry.ptr().get());
}

bool Geometry::disjoint(const Geometry &geometry) const{
    return this->ptr()->disjoint(geometry.ptr().get());
}

bool Geometry::touches(const Geometry &geometry) const{
    return this->ptr()->touches(geometry.ptr().get());
}

bool Geometry::intersects(const Geometry &geometry) const{
    return this->ptr()->intersects(geometry.ptr().get());
}

bool Geometry::crosses(const Geometry &geometry) const{
    return this->ptr()->crosses(geometry.ptr().get());
}

bool Geometry::overlaps(const Geometry &geometry) const{
    return this->ptr()->overlaps(geometry.ptr().get());
}

bool Geometry::equals(const Geometry &geometry) const{
    return this->ptr()->equals(geometry.ptr().get());
}

bool Geometry::covers(const Geometry &geometry) const{
    return this->ptr()->covers(geometry.ptr().get());
}

bool Geometry::coveredBy(const Geometry &geometry) const{
    return this->ptr()->coveredBy(geometry.ptr().get());
}

bool Geometry::relate(const Geometry &geometry, const std::string& intersectionPattern) const{
    return this->ptr()->relate(geometry.ptr().get(), intersectionPattern);
}

double Geometry::distance(const Geometry &geometry) const{
    return this->ptr()->distance(geometry.ptr().get());
}

double Geometry::getArea() const{
    return this->ptr()->getArea();
}

double Geometry::getLength() const{
    return this->ptr()->getLength();
}

bool Geometry::isWithinDistance(const Geometry &geometry, double cDistance) const{
    return this->ptr()->isWithinDistance(geometry.ptr().get(), cDistance);
}

const std::unique_ptr<geos::geom::Geometry>& Geometry::ptr() const{
    if (this->_standalone){
        if (!this->__bool__())
            throw InvalidObject("invalid standalone Geometry!");
        return this->_ilwisGeometry;
    }else{
        if (!this->__bool__())
            throw InvalidObject("invalid Geometry!");
        return this->_feature->ptr()->geometry(this->_index);
    }
}

}//namespace pythonapi
