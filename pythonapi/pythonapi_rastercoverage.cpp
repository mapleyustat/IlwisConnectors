#include "../../IlwisCore/core/kernel.h"
#include "../../IlwisCore/core/ilwisobjects/ilwisobject.h"

#include "../../IlwisCore/core/ilwisobjects/ilwisdata.h"
#include "../../IlwisCore/core/ilwisobjects/domain/domain.h"
#include "../../IlwisCore/core/ilwisobjects/domain/datadefinition.h"
#include "../../IlwisCore/core/ilwisobjects/table/columndefinition.h"
#include "../../IlwisCore/core/ilwisobjects/table/table.h"

#include "../../IlwisCore/core/ilwisobjects/coverage/raster.h"
#include "../../IlwisCore/core/ilwisobjects/coverage/coverage.h"

#include "../../IlwisCore/core/ilwisobjects/coverage/rastercoverage.h"
#include "../../IlwisCore/core/ilwisobjects/coverage/pixeliterator.h"
#include "../../IlwisCore/core/ilwisobjects/coverage/geometryhelper.h"

#include "../../IlwisCore/core/ilwisobjects/operation/operationoverloads.h"

#include "pythonapi_rastercoverage.h"
#include "pythonapi_engine.h"
#include "pythonapi_error.h"
#include "pythonapi_datadefinition.h"
#include "pythonapi_qvariant.h"
#include "pythonapi_pyobject.h"
#include "pythonapi_geometry.h"
#include "pythonapi_domain.h"

using namespace pythonapi;



RasterCoverage::RasterCoverage(Ilwis::IRasterCoverage *coverage):Coverage(new Ilwis::ICoverage(*coverage)){
//delete coverage;
}

RasterCoverage::RasterCoverage(){
    Ilwis::IRasterCoverage fc;
    fc.prepare();
    if (fc.isValid())
        this->_ilwisObject = std::shared_ptr<Ilwis::IIlwisObject>(new Ilwis::IIlwisObject(fc));
}

RasterCoverage::RasterCoverage(std::string resource){
    Ilwis::IRasterCoverage fc(QString::fromStdString(resource), itRASTER);
    if (fc.isValid())
        this->_ilwisObject = std::shared_ptr<Ilwis::IIlwisObject>(new Ilwis::IIlwisObject(fc));
}

RasterCoverage::~RasterCoverage(){

}

RasterCoverage* RasterCoverage::operator+(RasterCoverage &rc){
    return (RasterCoverage*)Engine::_do(
                QString("add_%1_%2").arg((*this->ptr())->id()).arg((*rc.ptr())->id()).toStdString(),
                "binarymathraster",
                this->__str__(),
                rc.__str__(),
                "add"
            );
}

RasterCoverage *RasterCoverage::operator+ (double value){
    return (RasterCoverage*)Engine::_do(
                QString("add_%1_%2").arg((*this->ptr())->id()).arg(value).toStdString(),
                "binarymathraster",
                this->__str__(),
                QString("%1").arg(value).toStdString(),
                "add"
            );
}

RasterCoverage *RasterCoverage::__radd__(double value){
    return (RasterCoverage*)Engine::_do(
                QString("add_%2_%1").arg((*this->ptr())->id()).arg(value).toStdString(),
                "binarymathraster",
                QString("%1").arg(value).toStdString(),
                this->__str__(),
                "add"
            );
}

RasterCoverage *RasterCoverage::operator-(RasterCoverage &rc){
    return (RasterCoverage*)Engine::_do(
        QString("sub_%1_%2").arg((*this->ptr())->id()).arg((*rc.ptr())->id()).toStdString(),
        "binarymathraster",
        this->__str__(),
        rc.__str__(),
        "substract"
    );
}

RasterCoverage *RasterCoverage::operator-(double value){
    return (RasterCoverage*)Engine::_do(
                QString("sub_%1_%2").arg((*this->ptr())->id()).arg(value).toStdString(),
                "binarymathraster",
                this->__str__(),
                QString("%1").arg(value).toStdString(),
                "substract"
            );
}

RasterCoverage *RasterCoverage::__rsub__(double value){
    return (RasterCoverage*)Engine::_do(
                QString("sub_%2_%1").arg((*this->ptr())->id()).arg(value).toStdString(),
                "binarymathraster",
                QString("%1").arg(value).toStdString(),
                this->__str__(),
                "substract"
            );
}

RasterCoverage *RasterCoverage::operator*(RasterCoverage &rc){
    return (RasterCoverage*)Engine::_do(
        QString("mul_%1_%2").arg((*this->ptr())->id()).arg((*rc.ptr())->id()).toStdString(),
        "binarymathraster",
        this->__str__(),
        rc.__str__(),
        "times"
    );
}

RasterCoverage *RasterCoverage::operator*(double value){
    return (RasterCoverage*)Engine::_do(
                QString("mul_%1_%2").arg((*this->ptr())->id()).arg(value).toStdString(),
                "binarymathraster",
                this->__str__(),
                QString("%1").arg(value).toStdString(),
                "times"
            );
}

RasterCoverage *RasterCoverage::__rmul__(double value){
    return (RasterCoverage*)Engine::_do(
                QString("mul_%2_%1").arg((*this->ptr())->id()).arg(value).toStdString(),
                "binarymathraster",
                QString("%1").arg(value).toStdString(),
                this->__str__(),
                "times"
            );
}

RasterCoverage *RasterCoverage::__truediv__(RasterCoverage &rc){
    return (RasterCoverage*)Engine::_do(
        QString("div_%1_%2").arg((*this->ptr())->id()).arg((*rc.ptr())->id()).toStdString(),
        "binarymathraster",
        this->__str__(),
        rc.__str__(),
        "divide"
    );
}

RasterCoverage *RasterCoverage::__truediv__(double value){
    return (RasterCoverage*)Engine::_do(
                QString("div_%1_%2").arg((*this->ptr())->id()).arg(value).toStdString(),
                "binarymathraster",
                this->__str__(),
                QString("%1").arg(value).toStdString(),
                "divide"
            );
}

RasterCoverage *RasterCoverage::__rtruediv__(double value){
    return (RasterCoverage*)Engine::_do(
                QString("div_%2_%1").arg((*this->ptr())->id()).arg(value).toStdString(),
                "binarymathraster",
                QString("%1").arg(value).toStdString(),
                this->__str__(),
                "divide"
            );
}

double RasterCoverage::coord2value(const Coordinate& c){
    return this->ptr()->as<Ilwis::RasterCoverage>()->coord2value(c.data());
}

PixelIterator RasterCoverage::__iter__(){
    return PixelIterator(this);
}

double RasterCoverage::pix2value(const PixelD &pix){
    return this->ptr()->as<Ilwis::RasterCoverage>()->pix2value(pix.data());
}

double RasterCoverage::pix2value(const Pixel &pix){
    return this->ptr()->as<Ilwis::RasterCoverage>()->pix2value(pix.data());
}

Size RasterCoverage::size(){
    return Size(this->ptr()->as<Ilwis::RasterCoverage>()->size());
}

void RasterCoverage::setSize(const Size &sz){
    this->ptr()->as<Ilwis::RasterCoverage>()->size(sz.data());
}

void RasterCoverage::unloadBinary(){
    this->ptr()->as<Ilwis::RasterCoverage>()->unloadBinary();
}

RasterCoverage* RasterCoverage::toRasterCoverage(Object* obj){
    RasterCoverage* ptr = dynamic_cast<RasterCoverage*>(obj);
    if(!ptr)
        throw InvalidObject("cast to RasterCoverage not possible");
    return ptr;
}

CoordinateSystem RasterCoverage::coordinateSystem(){
    return CoordinateSystem(new Ilwis::ICoordinateSystem(this->ptr()->as<Ilwis::RasterCoverage>()->georeference()->coordinateSystem()));
}

GeoReference RasterCoverage::geoReference(){
    return GeoReference(new Ilwis::IGeoReference(this->ptr()->as<Ilwis::RasterCoverage>()->georeference()));
}

void RasterCoverage::setGeoReference(const GeoReference& gr){
    this->ptr()->as<Ilwis::RasterCoverage>()->georeference(gr.ptr()->as<Ilwis::GeoReference>());
}

DataDefinition& RasterCoverage::datadef() const{
    Ilwis::DataDefinition ilwdef = this->ptr()->as<Ilwis::RasterCoverage>()->datadef();
    DataDefinition* pydef = new DataDefinition(&ilwdef);
    return *pydef;
}

void RasterCoverage::setDataDef(DataDefinition* datdef){
    Ilwis::DataDefinition& ilwdef = this->ptr()->as<Ilwis::RasterCoverage>()->datadefRef();
    ilwdef = datdef->ptr();
}

void RasterCoverage::setDataDef(Domain& dm){
    Ilwis::DataDefinition& ilwdef = this->ptr()->as<Ilwis::RasterCoverage>()->datadefRef();
    DataDefinition* newDef = new DataDefinition(dm);
    ilwdef = newDef->ptr();
}

NumericStatistics* RasterCoverage::statistics(int mode, int bins){
    return new NumericStatistics(this->ptr()->as<Ilwis::RasterCoverage>()->statistics(mode, bins));
 }

PixelIterator RasterCoverage::begin(){
    return PixelIterator(this);
}

PixelIterator RasterCoverage::end(){
    return PixelIterator(this).end();
}

PixelIterator RasterCoverage::band(PyObject *pyTrackIndex){
    QVariant qIndex = resolveIndex(pyTrackIndex);
    if((QMetaType::Type)qIndex.type() == QMetaType::Double){
         Ilwis::PixelIterator* ilwIter = new Ilwis::PixelIterator(this->ptr()->as<Ilwis::RasterCoverage>()->band(qIndex.toDouble()));
        return PixelIterator(ilwIter);
    } else if((QMetaType::Type)qIndex.type() == QMetaType::QString){
         Ilwis::PixelIterator* ilwIter = new Ilwis::PixelIterator(this->ptr()->as<Ilwis::RasterCoverage>()->band(qIndex.toString()));
        return PixelIterator(ilwIter);
    }

    throw InvalidObject("Couldn't find data at this index.");
}

void RasterCoverage::addBand(PyObject* pyTrackIndex, PixelIterator* pyIter){
    QVariant qIndex = resolveIndex(pyTrackIndex);
    if((QMetaType::Type)qIndex.type() == QMetaType::Double){
        this->_ilwisObject->as<Ilwis::RasterCoverage>()->band(qIndex.toDouble(), pyIter->ptr());
    } else if((QMetaType::Type)qIndex.type() == QMetaType::QString){
        this->_ilwisObject->as<Ilwis::RasterCoverage>()->band(qIndex.toString(), pyIter->ptr());
    } else {
        throw InvalidObject("The index is not included in the definition.");
    }
}

void RasterCoverage::setBandDefinition(PyObject *pyTrackIndex, const DataDefinition &datdef){
    QVariant qIndex = resolveIndex(pyTrackIndex);
    if((QMetaType::Type)qIndex.type() == QMetaType::Double){
        this->_ilwisObject->as<Ilwis::RasterCoverage>()->setBandDefinition(qIndex.toDouble(), datdef.ptr());
    } else if((QMetaType::Type)qIndex.type() == QMetaType::QString){
        this->_ilwisObject->as<Ilwis::RasterCoverage>()->setBandDefinition(qIndex.toString(), datdef.ptr());
    } else{
        throw InvalidObject("The index is not included in the definition.");
    }
}

QVariant RasterCoverage::resolveIndex(PyObject *pyTrackIndex){
    if(PyFloatCheckExact(pyTrackIndex)){
        double val = PyFloatAsDouble(pyTrackIndex);
        return QVariant(val);
    }else if(PyLongCheckExact(pyTrackIndex)){
        long val = PyLongAsLong(pyTrackIndex);
        return QVariant((double) val);
    }else if(PyUnicodeCheckExact(pyTrackIndex)){
        std::string val = PyBytesAsString(pyTrackIndex);
        return QVariant(QString::fromStdString(val));
    }else if(PyDateTimeCheckExact(pyTrackIndex) || PyDateCheckExact(pyTrackIndex) || PyTimeCheckExact(pyTrackIndex)){
        int year = PyDateTimeGET_YEAR(pyTrackIndex);
        int month = PyDateTimeGET_MONTH(pyTrackIndex);
        int day = PyDateTimeGET_DAY(pyTrackIndex);
        std::string dateStr = std::to_string(year) + dateToString(month) + dateToString(day);
        return QVariant(QString::fromStdString(dateStr));
    }
    throw InvalidObject("Could not resolve index.");
}

std::string RasterCoverage::dateToString(int datepart){
    std::string dateStr;
    if(datepart < 10){
        dateStr = "0" + std::to_string(datepart);
    } else {
        dateStr = std::to_string(datepart);
    }

    return dateStr;
}

void RasterCoverage::setSubDefinition(const Domain& dom, PyObject* items){
    if(PyTupleCheckExact(items)){
        int sz = PyTupleSize(items);
        if(PyFloatCheckExact(PyTupleGetItem(items, 0))){
            std::vector<double> ilwVec;
            for(int i = 0; i < sz; ++i){
                double val = PyFloatAsDouble(PyTupleGetItem(items, i));
                ilwVec.push_back(val);
            }
            this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().setSubDefinition(dom.ptr()->as<Ilwis::Domain>(), ilwVec);
        }else if(PyLongCheckExact(PyTupleGetItem(items, 0))){
            std::vector<double> ilwVec;
            for(int i = 0; i < sz; ++i){
                long val = PyLongAsLong(PyTupleGetItem(items, i));
                ilwVec.push_back((double)val);
            }
            this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().setSubDefinition(dom.ptr()->as<Ilwis::Domain>(), ilwVec);
        }else if(PyUnicodeCheckExact(PyTupleGetItem(items, 0))){
            std::vector<QString> ilwVec;
            for(int i = 0; i < sz; ++i){
                std::string val = PyBytesAsString(PyTupleGetItem(items, i));
                ilwVec.push_back(QString::fromStdString(val));
            }
            this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().setSubDefinition(dom.ptr()->as<Ilwis::Domain>(), ilwVec);
        }else if(PyDateTimeCheckExact(PyTupleGetItem(items, 0)) || PyDateCheckExact(PyTupleGetItem(items, 0)) || PyTimeCheckExact(PyTupleGetItem(items, 0))){
            std::vector<QString> ilwVec;
            for(int i = 0; i < sz; ++i){
                int year = PyDateTimeGET_YEAR(PyTupleGetItem(items, i));
                int month = PyDateTimeGET_MONTH(PyTupleGetItem(items, i));
                int day = PyDateTimeGET_DAY(PyTupleGetItem(items, i));
                std::string dateStr = std::to_string(year) + dateToString(month) + dateToString(day);
                ilwVec.push_back(QString::fromStdString(dateStr));
            }
            this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().setSubDefinition(dom.ptr()->as<Ilwis::Domain>(), ilwVec);
        }
    }
}

quint32 RasterCoverage::indexOf(const std::string& variantId) const{
    return this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().index(QString::fromStdString(variantId));
}

quint32 RasterCoverage::indexOf(double domainItem) const{
    return this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().index(domainItem);
}

quint32 RasterCoverage::indexOf(PyObject* obj) const{
    if(PyDateTimeCheckExact(obj) || PyDateCheckExact(obj) || PyTimeCheckExact(obj)){
        int year = PyDateTimeGET_YEAR(obj);
        int month = PyDateTimeGET_MONTH(obj);
        int day = PyDateTimeGET_DAY(obj);
        std::string dateStr = std::to_string(year) + std::to_string(month) + std::to_string(day);
        return this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().index(QString::fromStdString(dateStr));
    }
    return iUNDEF;
}

std::string RasterCoverage::atIndex(quint32 idx) const{
    QString qStr =  this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().index(idx);
    return qStr.toStdString();
}

PyObject* RasterCoverage::indexes() const{
    std::vector<QString> qVec = this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().indexes();
    PyObject* pyTup = newPyTuple(qVec.size());

    for(int i = 0; i < qVec.size(); ++i){
        std::string actStr = qVec[i].toStdString();
        setTupleItem(pyTup, i, PyBuildString(actStr));
    }

    return pyTup;
}

quint32 RasterCoverage::countSubs() const{
    return this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().count();
}

Domain RasterCoverage::subDomain() const{
    Ilwis::IDomain ilwDom =  this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().domain();
    return Domain(new Ilwis::IDomain(ilwDom));
}

void RasterCoverage::clear(){
    this->ptr()->as<Ilwis::RasterCoverage>()->stackDefinitionRef().clear();
}

RasterCoverage RasterCoverage::select(std::string selectionQ){
    QString selectGeom = QString::fromStdString(selectionQ);
    geos::geom::Geometry *geom = Ilwis::GeometryHelper::fromWKT(selectGeom, this->ptr()->as<Ilwis::RasterCoverage>()->coordinateSystem());


    if(geom){
        Ilwis::PixelIterator iterIn(this->ptr()->as<Ilwis::RasterCoverage>(), geom);
        const geos::geom::Envelope *env = geom->getEnvelopeInternal();
        Ilwis::Envelope envelope(Ilwis::Coordinate(env->getMinX(), env->getMinY()),Ilwis::Coordinate(env->getMaxX(), env->getMaxY()));
        Ilwis::BoundingBox box = this->ptr()->as<Ilwis::RasterCoverage>()->georeference()->coord2Pixel(envelope);
        QString grfcode = QString("code=georef:type=corners,csy=%1,envelope=%2,gridsize=%3,name=selection").arg(this->ptr()->as<Ilwis::RasterCoverage>()->coordinateSystem()->id()).arg(envelope.toString()).arg(box.size().toString());
        Ilwis::IGeoReference grf(grfcode);

        Ilwis::IRasterCoverage map2;
        map2.prepare();

        map2->coordinateSystem(this->ptr()->as<Ilwis::RasterCoverage>()->coordinateSystem());
        map2->georeference(grf);
        map2->datadefRef() = this->ptr()->as<Ilwis::RasterCoverage>()->datadef();

        Ilwis::PixelIterator iterOut(map2, geom);

        Ilwis::PixelIterator iterEnd = this->ptr()->as<Ilwis::RasterCoverage>()->end();
        while( iterIn != iterEnd){
            *iterOut = *iterIn;
            ++iterOut;
            ++iterIn;
        }

        return RasterCoverage(&map2);
    }
    else{
        throw InvalidObject("Not a valid geometry description");
    }

    delete geom;
}

RasterCoverage RasterCoverage::select(Geometry& geom){
    return select(geom.toWKT());
}

RasterCoverage* RasterCoverage::reprojectRaster(std::string newName, quint32 epsg, std::string interpol){
    CoordinateSystem* targetPyCsy = new CoordinateSystem("code=epsg:" + std::to_string(epsg));
    Ilwis::ICoordinateSystem targetIlwCsy = targetPyCsy->ptr()->as<Ilwis::CoordinateSystem>();
    Ilwis::IGeoReference georef = this->geoReference().ptr()->as<Ilwis::GeoReference>();
    Ilwis::ICoordinateSystem sourceCsy = georef->coordinateSystem();
    if(sourceCsy->name() == "unknown"){
        this->geoReference().setCoordinateSystem(*targetPyCsy);
        delete targetPyCsy;
        return this;
    }
    Ilwis::Envelope env  = this->ptr()->as<Ilwis::RasterCoverage>()->envelope();
    env = sourceCsy->convertEnvelope(targetIlwCsy, env);
    Ilwis::BoundingBox bo = georef->coord2Pixel(env);
    Ilwis::Size<> sz = bo.size();
    std::string refStr = "code=georef:type=corners,csy=epsg:" + std::to_string(epsg) + ",envelope=" +
            env.toString().toStdString() + ",gridsize=" + std::to_string(sz.xsize()) + " " + std::to_string(sz.ysize()) +
            ",name=grf1";

    GeoReference grf(refStr);
    QString expr = QString::fromStdString(newName + "=resample(" + this->name() + ",grf1," + interpol  + ")");
    //expr = expr.fromStdString(newName + "=resample(" + this->name() + ",grf1," + interpol  + ")");
    Ilwis::ExecutionContext ctx;
    Ilwis::SymbolTable syms;
    Ilwis::commandhandler()->execute(expr,&ctx,syms);
    QString path;
    path = path.fromStdString("ilwis://internalcatalog/" + newName);
    Ilwis::IRasterCoverage* raster = new Ilwis::IRasterCoverage(path);
    return new RasterCoverage(raster);
}

RasterCoverage* RasterCoverage::clone(){
    Ilwis::IRasterCoverage ilwRc = this->ptr()->as<Ilwis::RasterCoverage>()->clone();
    return new RasterCoverage(&ilwRc);
}

Envelope RasterCoverage::envelope(){
    return Envelope(this->ptr()->as<Ilwis::RasterCoverage>()->envelope());
}
