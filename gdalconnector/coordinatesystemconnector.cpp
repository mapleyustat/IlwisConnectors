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
#include "ilwisdata.h"
#include "ellipsoid.h"
#include "geodeticdatum.h"
#include "projection.h"
//#include "domain.h"
#include "coordinatesystem.h"
#include "conventionalcoordinatesystem.h"
#include "gdalproxy.h"
#include "gdalconnector.h"
#include "coordinatesystemconnector.h"


using namespace Ilwis;
using namespace Gdal;

ConnectorInterface *CoordinateSystemConnector::create(const Resource &resource, bool load, const IOOptions &options){
    return new CoordinateSystemConnector(resource, load, options);
}

IlwisObject *CoordinateSystemConnector::create() const{
    IlwisObject *object = 0;
    if ( type() == itCONVENTIONALCOORDSYSTEM)
        object = new ConventionalCoordinateSystem(_resource);
    return object;
}

CoordinateSystemConnector::CoordinateSystemConnector(const Resource &resource, bool load, const IOOptions &options) : GdalConnector(resource, load, options)
{

}



bool CoordinateSystemConnector::loadMetaData(IlwisObject *data, const IOOptions &options){

    if (!GdalConnector::loadMetaData(data, options))
        return false;
    bool ret = true;
    OGRSpatialReferenceH srshandle = gdal()->srsHandle(_handle, data->name());
    if (srshandle){
        CoordinateSystem *csy = static_cast<CoordinateSystem *>(data);

        if ( type() == itCONVENTIONALCOORDSYSTEM) {
            ConventionalCoordinateSystem *csyp = static_cast<ConventionalCoordinateSystem *>(csy);
            QString datumName(gdal()->getAttributeValue(srshandle,"Datum",0));
            if ( datumName == "WGS_1984")
                csy->prepare("+proj=longlat +ellps=WGS84 +datum=WGS84");
            else {
                GeodeticDatum *datum = new GeodeticDatum(datumName);
                //datum.prepare("wkt=" + datumName);
                if ( datum->isValid())
                    csyp->setDatum(datum);

                QString projectionName(gdal()->getAttributeValue(srshandle,"Projection",0));
                if ( projectionName != "") {
                    IProjection projection;
                    projection.prepare("code=wkt:" + projectionName);
                    if ( projection.isValid()) {
                        setProjectionParameter(srshandle, "false_easting", Projection::pvX0, projection);
                        setProjectionParameter(srshandle, "false_northing", Projection::pvY0, projection);
                        setProjectionParameter(srshandle, "scale_factor", Projection::pvK0, projection);
                        setProjectionParameter(srshandle, "central_meridian", Projection::pvLON0, projection);
                        setProjectionParameter(srshandle, "latitude_of_origin", Projection::pvLAT0, projection);
                        setProjectionParameter(srshandle, "standard_parallel_1", Projection::pvLAT1, projection);
                        setProjectionParameter(srshandle, "standard_parallel_2", Projection::pvLAT2, projection);
                        setProjectionParameter(srshandle, "zone", Projection::pvZONE, projection);
                        csyp->setProjection(projection);
                    }
                }
            }
            QString ellipsoidName(gdal()->getAttributeValue(srshandle,"SPHEROID",0));
            char *wkt = new char[10000];
            gdal()->exportToPrettyWkt(srshandle,&wkt,TRUE);
            IEllipsoid ellipsoid;
            if ( (ellipsoidName.compare("unnamed",Qt::CaseInsensitive) != 0) && (ellipsoidName.compare("unknown",Qt::CaseInsensitive) != 0)) {
                ellipsoid.prepare("code=wkt:" + ellipsoidName);
                if ( ellipsoid.isValid())
                    csyp->setEllipsoid(ellipsoid);
            }else {
                QString axis(gdal()->getAttributeValue(srshandle,"SPHEROID",1));
                QString flat(gdal()->getAttributeValue(srshandle,"SPHEROID",2));
                bool ok1, ok2;
                double laxis = axis.toDouble(&ok1);
                double flattening = flat.toDouble(&ok2);
                if ( ok1 && ok2){
                    ellipsoid.prepare();
                    ellipsoid->name("unnamend");
                    ellipsoid->setEllipsoid(laxis, 1.0/flattening);
                    csyp->setEllipsoid(ellipsoid);
                }

            }
            Envelope env = gdal()->envelope(_handle, 0);
            if ( env.isValid() && !env.isNull()){
                csy->envelope(env);
            }

        }
    }else{
        ret = ERROR2(ERR_INVALID_PROPERTY_FOR_2,"OGRSpatialReference",data->name());
    }
    gdal()->releaseSrsHandle(_handle, srshandle, data->name());
    QFileInfo fileinf(source().toLocalFile());
    gdal()->closeFile(fileinf.absoluteFilePath(), data->id());
    return ret;
}

void CoordinateSystemConnector::setProjectionParameter(OGRSpatialReferenceH handle, const char *wkt_name, Projection::ProjectionParamValue parmType, IProjection& projection){
    OGRErr err;
    double value = gdal()->getProjectionParm(handle, wkt_name, rUNDEF, &err);
    if ( err == OGRERR_NONE && value != rUNDEF) {
        projection->setParameter(parmType, value);
    }
}



