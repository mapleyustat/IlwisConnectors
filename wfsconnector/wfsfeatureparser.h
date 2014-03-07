#ifndef WFSFEATUREPARSER_H
#define WFSFEATUREPARSER_H

#include "wfsconnector_global.h"

#include "geos/geom/Polygon.h"

class QString;
class XmlStreamParser;

namespace Ilwis {
namespace Wfs {

class WfsResponse;
class FillerColumnDef;

class WFSCONNECTORSHARED_EXPORT WfsFeatureParser
{
public:
    WfsFeatureParser(WfsResponse *response, FeatureCoverage *fcoverage);
    ~WfsFeatureParser();

    void context(const WfsParsingContext &context);

    WfsParsingContext context() const;

    /**
     * Parses feature instances and add them to the feature coverage.<br/>
     * <br/>
     * Note that the feature's attribute table has to be constructed first to give
     * valuable results. This can be done by means of a WfsFeatureDescriptionParser
     * which uses the feature's schema description served by the WFS to create appropriate
     * Domain columns for each feature attribute.<br/>
     * <br/>
     * The parsed xml schema description may use different namespace mappings than the
     * feature collection. To keep track of the elements the WfsFeatureDescriptionParser
     * gives access to those used in the schema document.
     *
     * TODO: for now only the target namespace is being used ... propably this is enough
     */
    void parseFeatureMembers();

private:
    XmlStreamParser *_parser;
    FeatureCoverage *_fcoverage;
    WfsParsingContext _context;

    void parseFeature(std::vector<QVariant> &record);

    QVariant fillStringColumn();
    QVariant fillDoubleColumn();
    QVariant fillDateTimeColumn();
    QVariant fillIntegerColumn();
    /**
     * Parses the feature's geometry from GML. <br/>
     * <br/>
     * Note that parsing GML has its limitations, so don't expect a full GML parsing engine
     * here, e.g. attributes are only read until a certain depth.
     *
     * @return the parsed geometry.
     */
    geos::geom::Geometry *parseFeatureGeometry();
    void createNewFeature();

    geos::geom::Geometry *createPolygon(bool isMultiGeometry);
    geos::geom::Geometry *createLineString(bool isMultiGeometry);

    bool isPolygonType();
    bool isLineStringType();
    bool isPointType();

    void updateSrsInfo();
    bool updateSrsInfoUntil(QString qname);

    geos::geom::LineString *parseLineString(bool &ok);
    geos::geom::Polygon *parsePolygon(bool &ok);
    geos::geom::LinearRing *parseExteriorRing();
    std::vector<geos::geom::Geometry *> *parseInteriorRings();


    QString gmlPosListToWktCoords(QString gmlPosList);
    QString gmlPosListToWktPolygon(QString gmlPosList);
    QString gmlPosListToWktLineString(QString gmlPosList);
    QString gmlPosListToWktPoint(QString gmlPosList);

};

}
}
#endif // WFSFEATUREPARSER_H
