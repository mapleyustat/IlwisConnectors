#include "../../IlwisCore/core/kernel.h"
#include "../../IlwisCore/core/ilwisobjects/ilwisobject.h"
#include "../../IlwisCore/core/ilwisobjects/ilwisdata.h"

#include "../../IlwisCore/core/util/geometries.h"
#include "../../IlwisCore/core/ilwisobjects/geometry/coordinatesystem/coordinatesystem.h"

#include "../../IlwisCore/core/ilwisobjects/geometry/coordinatesystem/ellipsoid.h"
#include "../../IlwisCore/core/ilwisobjects/geometry/coordinatesystem/geodeticdatum.h"
#include "../../IlwisCore/core/ilwisobjects/geometry/coordinatesystem/projection.h"
#include "../../IlwisCore/core/ilwisobjects/geometry/coordinatesystem/proj4parameters.h"

#include "../../IlwisCore/core/ilwisobjects/geometry/coordinatesystem/conventionalcoordinatesystem.h"

#include "pythonapi_coordinatesystem.h"
#include "pythonapi_error.h"

using namespace pythonapi;

CoordinateSystem::CoordinateSystem(Ilwis::ICoordinateSystem *cs): IlwisObject(new Ilwis::IIlwisObject(*cs)){
    delete cs;
}

CoordinateSystem::CoordinateSystem(const std::string& resource){
    Ilwis::ICoordinateSystem cs(QString::fromStdString(resource), itCOORDSYSTEM);
    if (cs.isValid())
        this->_ilwisObject = std::shared_ptr<Ilwis::IIlwisObject>(new Ilwis::IIlwisObject(cs));
}

Envelope CoordinateSystem::envelope(){
    return Envelope(this->ptr()->as<Ilwis::CoordinateSystem>()->envelope());
}

void CoordinateSystem::setEnvelope(const Envelope& env){
    Ilwis::Envelope ilwEnv = env.data();
    this->ptr()->as<Ilwis::CoordinateSystem>()->envelope(ilwEnv);
}

bool CoordinateSystem::operator==(const CoordinateSystem &csy){
    return (this->ptr()->ptr()->ilwisType() == itCONVENTIONALCOORDSYSTEM) && this->ptr()->as<Ilwis::ConventionalCoordinateSystem>()->isEqual(csy.ptr()->ptr());
}

bool CoordinateSystem::operator!=(const CoordinateSystem &csy){
    return !this->operator==(csy);
}

std::string CoordinateSystem::toWKT(quint32 spaces) const{
    QString ilwStr = this->ptr()->as<Ilwis::CoordinateSystem>()->toWKT();
    return ilwStr.toStdString();
}

std::string CoordinateSystem::toProj4() const{
    if(const_cast<CoordinateSystem*>(this)->type() == "ConventionalCoordinateSystem"){
        QString ilwStr = this->ptr()->as<Ilwis::ConventionalCoordinateSystem>()->toProj4();
        return ilwStr.toStdString();
    }else{
        throw InvalidObject("No proj4 definition for this kind of coordinatesystem available");
    }
}

Envelope CoordinateSystem::convertEnvelope(const CoordinateSystem& pyCsy, const Envelope& pyEnv){
    Ilwis::ICoordinateSystem ilwCsy = pyCsy.ptr()->as<Ilwis::CoordinateSystem>();
    Ilwis::Envelope ilwEnv = pyEnv.data();
    return Envelope(this->ptr()->as<Ilwis::CoordinateSystem>()->convertEnvelope(ilwCsy, ilwEnv));
}

Coordinate CoordinateSystem::coord2coord(const CoordinateSystem &pyCsy, const Coordinate &pyCoord){
    Ilwis::ICoordinateSystem ilwCsy = pyCsy.ptr()->as<Ilwis::CoordinateSystem>();
    Ilwis::Coordinate ilwCoord = pyCoord.data();
    return Coordinate(this->ptr()->as<Ilwis::CoordinateSystem>()->coord2coord(ilwCsy, ilwCoord));
}

CoordinateSystem *CoordinateSystem::toCoordinateSystem(Object *obj){
    CoordinateSystem* ptr = dynamic_cast<CoordinateSystem*>(obj);
    if(!ptr)
        throw InvalidObject("cast to CoordinateSystem not possible");
    return ptr;
}
