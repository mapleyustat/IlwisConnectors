#include <QUrl>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QStringList>

#include "kernel.h"
#include "geometries.h"
#include "connectorinterface.h"
#include "mastercatalog.h"
#include "ilwisobjectconnector.h"
#include "catalogexplorer.h"
#include "catalogconnector.h"
#include "inifile.h"
#include "ilwisdata.h"
#include "ilwis3connector.h"
#include "ellipsoid.h"
#include "geodeticdatum.h"
#include "projection.h"
#include "coordinatesystem.h"
#include "conventionalcoordinatesystem.h"
#include "boundsonlycoordinatesystem.h"
#include "coordinatesystemconnector.h"

using namespace Ilwis;
using namespace Ilwis3;

ConnectorInterface *CoordinateSystemConnector::create(const Resource &resource,bool load,const IOOptions& options) {
    return new CoordinateSystemConnector(resource, load, options);

}

CoordinateSystemConnector::CoordinateSystemConnector(const Resource &resource, bool load, const IOOptions &options) : Ilwis3Connector(resource, load, options)
{
    QString type = _odf->value("CoordSystem","Type");
}

IlwisObject *CoordinateSystemConnector::create() const {
    IlwisObject *object = 0;
    if ( type() == itCONVENTIONALCOORDSYSTEM)
        object = new ConventionalCoordinateSystem(_resource);
    if ( type() == itBOUNDSONLYCSY)
        object = new BoundsOnlyCoordinateSystem(_resource);

    return object;
}

bool CoordinateSystemConnector::loadMetaData(IlwisObject* data, const IOOptions& options)
{
    Ilwis3Connector::loadMetaData(data, options);
    CoordinateSystem *csy = static_cast<CoordinateSystem *>(data);
    QString ellipsoideName;

    IEllipsoid ell = getEllipsoid();
    GeodeticDatum *datum = getDatum(ellipsoideName);
    if ( !ell.isValid() && ellipsoideName != sUNDEF){
        QString ellres = QString("ilwis://tables/ellipsoid?code=%1").arg(ellipsoideName);
       if (!ell.prepare(ellres)) {
              return ERROR1("No ellipsoid for this code %1",ellipsoideName);
       }
    }

    QString cb = _odf->value("CoordSystem", "CoordBounds");
    QStringList cbparts = cb.split(" ");
    if ( cbparts.size() == 4 && cbparts[0] != "-1e+308") {
        bool ok1, ok2, ok3, ok4;
        Envelope box( Coordinate(
                                cbparts[0].toDouble(&ok1),
                                cbparts[1].toDouble(&ok2)),
                           Coordinate(
                                cbparts[2].toDouble(&ok3),
                                cbparts[3].toDouble(&ok4)));
        if ( !( ok1 && ok2 && ok3 && ok4)) {
            return ERROR2(ERR_NO_INITIALIZED_2, TR("envelop"), csy->name());
        }
        csy->envelope(box);
    } else {
        QString type = _odf->value("CoordSystem", "Type");
        if ( type == "LatLon") {
            Envelope box(Coordinate(-180,-90), Coordinate(180,90));
            csy->envelope(box);
        }
    }

    if ( type() == itCONVENTIONALCOORDSYSTEM ) {
        ConventionalCoordinateSystem *csycc = static_cast<ConventionalCoordinateSystem *>(csy);
        IProjection proj = getProjection(csycc);
        if ( !proj.isValid()) {
            return ERROR1(ERR_NO_INITIALIZED_1, "projection");
        }
        csycc->setDatum(datum);
        csycc->setEllipsoid(ell);
        csycc->setProjection(proj);
        proj->setCoordinateSystem(csycc);


        proj->setParameter(Projection::pvELLCODE, ell->toProj4());
        csycc->prepare();
    } else if ( type() == itUNKNOWN){
        //TODO: other types of csy
    }
    return true;
}

bool CoordinateSystemConnector::canUse(const Resource& resource,const UPCatalogConnector &container) // static
{
    IlwisTypes requiredType = resource.ilwisType();
    if ( (requiredType & itCOORDSYSTEM)!= 0)
        return true;
    QString file = resource.url().toLocalFile();
    QFileInfo inf(file);
    if ( inf.exists()) {
        IniFile odf;
        odf.setIniFile(inf);
        QString t = odf.value("CoordSystem","Type");
        if ( t == sUNDEF ) {
            if ( odf.value("CoordSystem","Projection") != "") // necessary due to incompleteness of ilwis odf's
                t = "Projection";
        }
        if (t == "LatLon" && requiredType == itCONVENTIONALCOORDSYSTEM)
            return true;
        else if ( t == "Projection" && requiredType == itCONVENTIONALCOORDSYSTEM)
            return true;
        if ( t == "BoundsOnly" && requiredType == itBOUNDSONLYCSY)
            return true;

    }
    return false;
}

IEllipsoid CoordinateSystemConnector::getEllipsoid() {
    QString ell = _odf->value("CoordSystem","Ellipsoid");
    if ( ell == "?")
        return IEllipsoid();
    if ( ell == "User Defined") {
        double invf = _odf->value("Ellipsoid","1/f").toDouble();
        double majoraxis = _odf->value("Ellipsoid","a").toDouble();
        Ellipsoid *ellips = new Ellipsoid();
        QString newName = ellips->setEllipsoid(majoraxis,invf);;
        ellips->name(newName);
        IEllipsoid ellipsoid(ellips);
        return ellipsoid;
    }
    IEllipsoid ellipsoid;
    QString code = name2Code(ell, "ellipsoid");
    if ( code == sUNDEF)
        return IEllipsoid();

    QString resource = QString("ilwis://tables/ellipsoid?code=%1").arg(code);

    ellipsoid.prepare(resource);

    return ellipsoid;
}

QString CoordinateSystemConnector::prjParam2IlwisName(Projection::ProjectionParamValue parm)
{
    switch(parm){
        case Projection::pvAZIMCLINE:
        return "Azim of Central Line of True Scale";
    case Projection::pvAZIMYAXIS:
        return "Azim of Projection Y-Axis";
    case Projection::pvHEIGHT:
        return "Height Persp. Center";
    case Projection::pvK0:
        return "Scale Factor";
    case Projection::pvLAT0:
        return "Central Parallel";
    case Projection::pvLAT1:
        return "Standard Parallel 1";
    case Projection::pvLAT2:
        return "Standard Parallel 2";
    case Projection::pvLATTS:
        return "Latitude of True Scale";
    case Projection::pvLON0:
        return "Central Meridian";
    case Projection::pvNORIENTED:
        return "North Oriented XY Coord System";
    case Projection::pvNORTH:
        return "Northern Hemisphere";
    case Projection::pvPOLE:
        return "Pole of Oblique Cylinder";
    case Projection::pvTILTED:
        return "Tilted/Rotated Projection Plane";
    case Projection::pvTILT:
        return "Tilt of Projection Plane";
    case Projection::pvX0:
        return "False Easting";
    case Projection::pvY0:
        return "False Northing";
    case Projection::pvZONE:
        return "Zone";
    default:
        return sUNDEF;
    }
    return sUNDEF;
}

GeodeticDatum *CoordinateSystemConnector::getDatum(QString& ellipsoid) {
    QString datum =_odf->value("CoordSystem","Datum");
    if ( datum == sUNDEF)
        return 0; // not an error; simply no datum with this csy

    QString area = _odf->value("CoordSystem","Datum Area");
    if ( area != sUNDEF && area != "" )
        datum = datum + "." + area;
    QString code = name2Code(datum,"datum");

    if ( code == "?"){
        kernel()->issues()->log(TR("No datum code for this alias %1").arg(datum));
        return 0;
    }

    QSqlQuery stmt(kernel()->database());
    QString query = QString("Select * from datum where code='%1'").arg(code);

    if (stmt.exec(query)) {
        if ( stmt.next()) {
            GeodeticDatum *gdata = new GeodeticDatum();
            QString area = stmt.value(stmt.record().indexOf("area")).toString();
            QString code = stmt.value(stmt.record().indexOf("code")).toString();
            double dx = stmt.value(stmt.record().indexOf("dx")).toDouble();
            double dy = stmt.value(stmt.record().indexOf("dy")).toDouble();
            double dz = stmt.value(stmt.record().indexOf("dz")).toDouble();
            gdata->setArea(area);
            gdata->code(code);
            gdata->set3TransformationParameters(dx, dy, dz);
            ellipsoid = stmt.value(stmt.record().indexOf("ellipsoid")).toString();

            return gdata;

        } else {
            kernel()->issues()->log(TR("No datum for this code %1").arg(code));
        }
    } else {
        kernel()->issues()->logSql(stmt.lastError());
    }
    return 0;
}

IProjection CoordinateSystemConnector::getProjection(ConventionalCoordinateSystem *csycc) {
    QString projection = _odf->value("CoordSystem","Projection");
    if (projection == "?") {
        QString type = _odf->value("CoordSystem","Type");
        if ( type == "LatLon") {
            projection = type;
            GeodeticDatum *gdata = new GeodeticDatum();
            gdata->fromCode("DWGS84");
            csycc->setDatum(gdata);
        } else
            return IProjection();
    }

    QString code = name2Code(projection, "projection");
    Resource resource(QUrl(QString("ilwis://tables/projection?code=%1").arg(code)), itPROJECTION);

    if ( code == sUNDEF) {
        kernel()->issues()->log(TR("Couldnt find projection %1").arg(projection));
        return IProjection();
    }

    IProjection proj;
    if(!proj.prepare(resource))
        return IProjection();

    bool ok;
    double falseEasting = _odf->value("Projection","False Easting").toDouble(&ok);
    if ( ok)
        proj->setParameter(Projection::pvX0, falseEasting);
    double falseNorthing = _odf->value("Projection","False Northing").toDouble(&ok);
    if ( ok)
        proj->setParameter(Projection::pvY0, falseNorthing);
    double centralMeridian = _odf->value("Projection","Central Meridian").toDouble(&ok);
    if ( ok)
        proj->setParameter(Projection::pvLON0, centralMeridian);
    double centralParllel = _odf->value("Projection","Central Parallel").toDouble(&ok);
    if ( ok)
        proj->setParameter(Projection::pvLAT0, centralParllel);
    double standardParllel = _odf->value("Projection","Standard Parallel 1").toDouble(&ok);
    if ( ok)
        proj->setParameter(Projection::pvLAT1, standardParllel);
    standardParllel = _odf->value("Projection","Standard Parallel 2").toDouble(&ok);
    if ( ok)
        proj->setParameter(Projection::pvLAT2, standardParllel);
    double lattitudeOfTrueScale = _odf->value("Projection","Latitude of True Scale").toDouble(&ok);
    if ( ok)
        proj->setParameter(Projection::pvLATTS, lattitudeOfTrueScale);
    double scaleFactor = _odf->value("Projection","Scale Factor").toDouble(&ok);
    if ( ok)
        proj->setParameter(Projection::pvK0, scaleFactor);
    int gzone = _odf->value("Projection","Zone").toInt(&ok);
    if ( ok)
        proj->setParameter(Projection::pvZONE, gzone);
    QString hemisphere = _odf->value("Projection","Northern Hemisphere");
    if ( hemisphere != sUNDEF && code == "utm")
        proj->setParameter(Projection::pvNORTH, hemisphere);

    return proj;
}

bool CoordinateSystemConnector::storeMetaData(IlwisObject *data) {
    bool ok = true;
    if ((ok = Ilwis3Connector::storeMetaData(data, itCOORDSYSTEM)) == false){
        return false;
    }
    ICoordinateSystem csy(static_cast<CoordinateSystem *>(data));
    if (!csy.isValid() || !csy->isValid()){
        return ERROR1(ERR_NO_INITIALIZED_1, "CoordinateSystem");
    }
    Envelope bounds = csy->envelope();
    if(!bounds.isValid()){
        ERROR2(ERR_NO_INITIALIZED_2, "Bounds", csy->name());
        return sUNDEF;
    }

    _odf->setKeyValue("CoordSystem","CoordBounds",QString("%1 %2 %3 %4").
                      arg(bounds.min_corner().x,10,'f').
                      arg(bounds.min_corner().y,10,'f').
                      arg(bounds.max_corner().x,10,'f').
                      arg(bounds.max_corner().y,10,'f'));

    if ( csy->isLatLon()) {
        _odf->setKeyValue("CoordSystem", "Datum", "WGS 1984");
        _odf->setKeyValue("CoordSystem","Type","LatLon");
        if ( csy.as<ConventionalCoordinateSystem>()->ellipsoid().isValid()){
            IEllipsoid ell = csy.as<ConventionalCoordinateSystem>()->ellipsoid();
            _odf->setKeyValue("CoordSystem","Ellipsoid","User Defined");
            _odf->setKeyValue("Ellipsoid","1/f", 1.0 / ell->flattening());
            _odf->setKeyValue("Ellipsoid","a", ell->majorAxis());
        }
    }else{
        IConventionalCoordinateSystem projectedCsy = csy.as<ConventionalCoordinateSystem>();
        if( !projectedCsy->ellipsoid().isValid() || !projectedCsy->projection().isValid()){
                ERROR2(ERR_NO_INITIALIZED_2, "Ellipsoid/Projection", csy->name());
                return sUNDEF;
        }
        QString projectionName = Ilwis3Connector::code2name(projectedCsy->projection()->name(), "projection");
        _odf->setKeyValue("CoordSystem","Type","Projection");
        _odf->setKeyValue("CoordSystem","Projection",projectionName);
        QString ellipsoidName = projectedCsy->ellipsoid()->name();
        //ellipsoidName = Ilwis3Connector::name2Code(ellipsoidName, "ellipsoid");
        if ( ellipsoidName == sUNDEF){
            ellipsoidName = "User Defined";
            _odf->setKeyValue("Ellipsoid","a",projectedCsy->ellipsoid()->majorAxis() );
            _odf->setKeyValue("Ellipsoid","1/f",projectedCsy->ellipsoid()->flattening() );

        }else
            ellipsoidName = Ilwis3Connector::code2name(ellipsoidName, "ellipsoid");
        _odf->setKeyValue("CoordSystem","Ellipsoid",ellipsoidName);
        IProjection projection = projectedCsy->projection();
        if ( projection->isSet(Projection::pvAZIMCLINE))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvAZIMCLINE),projection->parameter(Projection::pvAZIMCLINE).toDouble());
        if ( projection->isSet(Projection::pvAZIMYAXIS))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvAZIMYAXIS),projection->parameter(Projection::pvAZIMYAXIS).toDouble());
        if ( projection->isSet(Projection::pvHEIGHT))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvHEIGHT),projection->parameter(Projection::pvHEIGHT).toDouble());
        if ( projection->isSet(Projection::pvK0))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvK0),projection->parameter(Projection::pvK0).toDouble());
        if ( projection->isSet(Projection::pvLAT0))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvLAT0),projection->parameter(Projection::pvLAT0).toDouble());
        if ( projection->isSet(Projection::pvLAT1))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvLAT1),projection->parameter(Projection::pvLAT1).toDouble());
        if ( projection->isSet(Projection::pvLAT2))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvLAT2),projection->parameter(Projection::pvLAT2).toDouble());
        if ( projection->isSet(Projection::pvLATTS))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvLATTS),projection->parameter(Projection::pvLATTS).toDouble());
        if ( projection->isSet(Projection::pvLON0))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvLON0),projection->parameter(Projection::pvLON0).toDouble());
        if ( projection->isSet(Projection::pvNORIENTED))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvNORIENTED),projection->parameter(Projection::pvNORIENTED).toString());
        if ( projection->isSet(Projection::pvNORTH))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvNORTH),projection->parameter(Projection::pvNORTH).toString());
        if ( projection->isSet(Projection::pvX0))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvX0),projection->parameter(Projection::pvX0).toDouble());
        if ( projection->isSet(Projection::pvY0))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvY0),projection->parameter(Projection::pvY0).toDouble());
        if ( projection->isSet(Projection::pvZONE))
            _odf->setKeyValue("Projection",prjParam2IlwisName(Projection::pvZONE),projection->parameter(Projection::pvZONE).toInt());

    }
    _odf->store("csy", source().toLocalFile());
    return true;


}

QString CoordinateSystemConnector::format() const
{
    return "coordsystem";
}
