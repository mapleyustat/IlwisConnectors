#include <QSqlQuery>
#include <QSqlError>
#include <fstream>
#include <iterator>

#include "kernel.h"
#include "coverage.h"
#include "module.h"
#include "inifile.h"
#include "coverage.h"
#include "polygon.h"
#include "ilwiscontext.h"
#include "catalog.h"
#include "numericdomain.h"
#include "numericrange.h"
#include "columndefinition.h"
#include "table.h"
#include "geometry.h"
#include "attributerecord.h"
#include "feature.h"
#include "featurecoverage.h"
#include "featureiterator.h"
#include "containerstatistics.h"
#include "rawconverter.h"
#include "ilwisobjectconnector.h"
#include "ilwis3connector.h"
#include "binaryilwis3table.h"
#include "coordinatedomain.h"
#include "coverageconnector.h"
#include "featureconnector.h"

using namespace Ilwis;
using namespace Ilwis3;

struct XYZ {
    double x,y,z;
};

ConnectorInterface *FeatureConnector::create(const Resource &item, bool load) {
    return new FeatureConnector(item, load);

}

Ilwis::IlwisObject *FeatureConnector::create() const
{
    return new Ilwis::FeatureCoverage(_resource);
}

void FeatureConnector::calcStatics(const IlwisObject *obj, NumericStatistics::PropertySets set) const
{
}

FeatureConnector::FeatureConnector(const Resource &item, bool load) : CoverageConnector(item, load)
{
}

bool FeatureConnector::loadBinaryPolygons30(FeatureCoverage *fcoverage, ITable& tbl) {
    BinaryIlwis3Table polTable;
    if ( !polTable.load(_odf)) {
        return ERROR1(ERR_COULD_NOT_OPEN_READING_1,_odf->fileinfo().fileName())    ;
    }

    BinaryIlwis3Table topTable;
    if ( !topTable.load(_odf,"top")) {
        return ERROR1(ERR_COULD_NOT_OPEN_READING_1,_odf->fileinfo().fileName())    ;
    }

    qint32 colValue = polTable.index("PolygonValue");
    qint32 colTopStart = polTable.index("TopStart");
    qint32 colArea = polTable.index("Area");
    int nrPolygons = polTable.rows();
    SPAttributeRecord record( new AttributeRecord(tbl,COVERAGEKEYCOLUMN));
    bool isNumeric = _odf->value("BaseMap","Range") != sUNDEF;

    double v;
    for(int i = 0; i < nrPolygons; ++i) {
        polTable.get(i,colArea, v);
        if ( v < 0)
            continue;
        polTable.get(i,colTopStart,v);
        qint32 index = v;
        std::vector<std::vector<Coordinate2d>> rings;
        if (getRings(index, topTable, polTable, rings)) {
            if ( rings.size() == 0)
                continue;
            Polygon polygon;
            polygon.outer().resize(rings[0].size());
            std::copy(rings[0].begin(), rings[0].end(), polygon.outer().begin());
            for(int j = 1; j < rings.size(); ++j) {
                polygon.inners()[j-1].resize(rings[j].size());
                std::copy(rings[j].begin(), rings[j].end(), polygon.inners()[j-1].begin());
            }
            polTable.get(i, colValue, v);
            if ( isNumeric) {
                tbl->cell(COVERAGEKEYCOLUMN, i, QVariant(i));
                tbl->cell(FEATUREVALUECOLUMN, i, QVariant(v));
                fcoverage->newFeature({polygon},i, record);
            } else {
                quint32 itemId = v;
                tbl->cell(COVERAGEKEYCOLUMN, i, QVariant(itemId));
                SPFeatureI feature = fcoverage->newFeature({polygon},itemId, record);
                tbl->cell(FEATUREIDCOLUMN, i, QVariant(feature->featureid()));
            }
        }
    }
    return true;
}

bool FeatureConnector::getRings(qint32 startIndex, const BinaryIlwis3Table& topTable, const BinaryIlwis3Table& polTable, std::vector<vector<Coordinate2d>>& rings ){
    qint32 row = startIndex;
    qint32 colCoords = topTable.index("Coords");
    qint32 colForward = topTable.index("ForwardLink");
    qint32 colBackward = topTable.index("BackwardLink");
    std::vector<Coordinate> ring;
    bool forward = isForwardStartDirection(topTable,colForward, colBackward, colCoords, row);
    do{
        std::vector<Coordinate> coords;
        topTable.get(row,colCoords,coords);
        if( coords.size() == 0 ||coords.back() == coords.front()){
            ring.resize(coords.size());
            std::copy(coords.begin(), coords.end(), ring.begin());
        } else if ( coords.back() == ring.back()) {
            ring.resize(coords.size());
            std::reverse_copy(coords.begin(), coords.end(), ring.begin());
        } else if ( ring.front() == coords.front()) {
            ring.resize(coords.size());
            std::reverse_copy(coords.begin(), coords.end(), ring.begin());
        } else if ( ring.front() == coords.back()) {
            ring.resize(coords.size());
            std::copy(coords.begin(), coords.end(), ring.begin());
        }

        if ( ring.front() == ring.back() && ring.size() > 3) {
            std::vector<Coordinate2d> ring2d;
            Coordinate crdOld;
            for(const Coordinate& crd : ring) {
                if ( crd == crdOld)  // remove duplicates
                    continue;
                ring2d.push_back(crd);
                crdOld = crd;
            }
            rings.push_back(ring2d);
            coords.clear();
            ring.clear();
        }
        qint32 oldIndex = row;
        double v;
        if ( forward)
            topTable.get(abs(row),colForward,v);
        else
            topTable.get(abs(row), colBackward, v);\
        row = v;
        if ( oldIndex == row && row != startIndex) // this would indicate infintite loop. corrupt data
            return false;
        forward = row > 0;
    } while(abs(row) != abs(startIndex) && row != iUNDEF);

    return true;
}

bool FeatureConnector::isForwardStartDirection(const BinaryIlwis3Table& topTable, qint32 colForward, qint32 colBackward, qint32 colCoords, long index) {
    qint32 fwl, bwl;
    double v;
    topTable.get(abs(index),colForward, v );
    fwl = v;
    if ( fwl != iUNDEF)
        --fwl; // due to being raw values
    topTable.get(abs(index),colBackward, v );
    bwl = v;
    if ( bwl != iUNDEF)
        --bwl;

    if ( abs(fwl) == abs(bwl)	)
        return true;
    if ( index < 0)
        return false;
    std::vector<Coordinate2d> startLine, forwardLine;
    topTable.get(abs(index), colCoords,startLine);
    topTable.get(abs(fwl), colCoords, forwardLine);

    bool forward = false;
    if ( fwl > 0)
        forward =  startLine.back() == forwardLine.front();
    else
        forward = startLine.back() == forwardLine.back();

    return forward;

}


bool FeatureConnector::loadBinaryPolygons37(FeatureCoverage *fcoverage, ITable& tbl) {
    QString datafile = _odf->value("PolygonMapStore","DataPol");
    datafile = context()->workingCatalog()->filesystemLocation().toLocalFile() + "/" + datafile;
    QFile file(datafile);

    if (!file.exists()){
        kernel()->issues()->log(TR(ERR_MISSING_DATA_FILE_1).arg(file.fileName()));
        return false;
    }
    if(!file.open(QIODevice::ReadOnly )){
        kernel()->issues()->log(TR(ERR_COULD_NOT_OPEN_READING_1).arg(file.fileName()));
        return false;
    }
    QDataStream stream(&file);
    int nrPolygons = fcoverage->featureCount(itPOLYGON);
    SPAttributeRecord record( new AttributeRecord(tbl,COVERAGEKEYCOLUMN));
    bool isNumeric = _odf->value("BaseMap","Range") != sUNDEF;

    for(int j=0; j < nrPolygons; ++j) {
        Polygon pol;
        readRing(stream, pol.outer());
        double value;
        quint32 numberOfHoles;
        stream.readRawData((char *)&value, 8);
        stream.readRawData((char *)&numberOfHoles, 4);
        pol.inners().resize(numberOfHoles);
        for(quint32 i=0; i< numberOfHoles;++i)
            readRing(stream, pol.inners()[i]);
        if ( isNumeric) {
            tbl->cell(COVERAGEKEYCOLUMN, j, QVariant(j));
            tbl->cell(FEATUREVALUECOLUMN, j, QVariant(value));
            fcoverage->newFeature({pol},j, record);
        } else {
            quint32 itemId = value;
            tbl->cell(COVERAGEKEYCOLUMN, j, QVariant(itemId));
            SPFeatureI feature = fcoverage->newFeature({pol},itemId, record);
            tbl->cell(FEATUREIDCOLUMN, j, QVariant(feature->featureid()));
        }

    }
    file.close();

    return true;
}

bool FeatureConnector::readRing(QDataStream& stream, std::vector<Coordinate2d> &ring ) {
    quint32 numberOfCoords;

    if (stream.readRawData((char *)&numberOfCoords, 4) <= 0)
        return ERROR1(ERR_COULD_NOT_OPEN_READING_1,"data file");
    vector<XYZ> pnts(numberOfCoords);
    stream.readRawData((char *)&pnts[0],numberOfCoords*3*8);
    ring.resize(numberOfCoords);
    for(quint32 i=0; i < numberOfCoords; ++i) {
        ring[i] = Coordinate2d(pnts[i].x, pnts[i].y);
    }

   return true;
}

bool FeatureConnector::loadBinaryPolygons(FeatureCoverage *fcoverage, ITable& tbl) {
    QString dataFile = _odf->value("PolygonMapStore","DataPol");
    if ( dataFile == sUNDEF) {
        return loadBinaryPolygons30(fcoverage, tbl);
    } else {
        return loadBinaryPolygons37(fcoverage, tbl);
    }
    return false;
}

bool FeatureConnector::loadBinarySegments(FeatureCoverage *fcoverage, ITable& tbl) {
    BinaryIlwis3Table mpsTable;
    if ( !mpsTable.load(_odf)) {
        return ERROR1(ERR_COULD_NOT_OPEN_READING_1,_odf->fileinfo().fileName())    ;
    }
    int colCoords = mpsTable.index("Coords");
    int colItemId = mpsTable.index("SegmentValue");
    bool isNumeric = _odf->value("BaseMap","Range") != sUNDEF;
    if ( isNumeric) // in other case nr of record already has been set as it is based on a real table
        tbl->setRows(mpsTable.rows());

    SPAttributeRecord record( new AttributeRecord(tbl,COVERAGEKEYCOLUMN));
    double value;
    for(quint32 i= 0; i < mpsTable.rows(); ++i) {
        std::vector<Coordinate > coords;
        mpsTable.get(i,colCoords,coords);
        Line2D<Coordinate2d > line;
        line.resize(coords.size());
        std::copy(coords.begin(), coords.end(), line.begin());
        mpsTable.get(i, colItemId,value);
        if ( isNumeric) {
            tbl->cell(COVERAGEKEYCOLUMN, i, QVariant(i));
            tbl->cell(FEATUREVALUECOLUMN, i, QVariant(value));
            fcoverage->newFeature({line},i, record);
        } else {
            quint32 itemId = value;
            tbl->cell(COVERAGEKEYCOLUMN, i, QVariant(itemId));
            SPFeatureI feature = fcoverage->newFeature({line},itemId, record);
            tbl->cell(FEATUREIDCOLUMN, i, QVariant(feature->featureid()));
        }


    }
    return true;


}

bool FeatureConnector::loadBinaryPoints(FeatureCoverage *fcoverage, ITable& tbl) {
    BinaryIlwis3Table mppTable;
    if ( !mppTable.load(_odf)) {
        return ERROR1(ERR_COULD_NOT_OPEN_READING_1,_odf->fileinfo().fileName())    ;
    }
    // two cases; the old case; 2 columns for x and y. and the new case one column for coord
    int coordColumnX = mppTable.index("x");
    int coordColumnY = mppTable.index("y");
    int coordColumn = mppTable.index("Coordinate");
    int colItemId = mppTable.index("Name");

    bool newCase =  coordColumnX == iUNDEF;
    SPAttributeRecord record( new AttributeRecord(tbl,COVERAGEKEYCOLUMN));


    for(quint32 i= 0; i < mppTable.rows(); ++i) {
        Coordinate c;
        double itemIdT;
        if ( newCase) {
            mppTable.get(i, coordColumn, c);
        } else {
            double x,y;
            mppTable.get(i, coordColumnX, x);
            mppTable.get(i, coordColumnY, y);
            c = Coordinate(x,y);
        }
        mppTable.get(i, colItemId,itemIdT);
        quint32 itemId = itemIdT;
        tbl->cell(COVERAGEKEYCOLUMN, i, QVariant(itemId));

        SPFeatureI feature = fcoverage->newFeature({c},itemId, record);

        tbl->cell(FEATUREIDCOLUMN, i, QVariant(feature->featureid()));

    }
    return true;
}

bool FeatureConnector::loadBinaryData(Ilwis::IlwisObject *obj) {
    if ( obj == nullptr)
        return false;

    FeatureCoverage *fcoverage = static_cast<FeatureCoverage *>(obj);

    IDomain covdom;
    ITable tbl = fcoverage->attributeTable(ilwisType(_odf->fileinfo().fileName()));
    if (!covdom.prepare("count")){
        return false;
    }
    tbl->addColumn(COVERAGEKEYCOLUMN,covdom);
    tbl->addColumn(FEATUREIDCOLUMN,covdom);

    bool isNumeric = _odf->value("BaseMap","Range") != sUNDEF;
    if ( isNumeric) {
        IDomain featuredom;
        if (!featuredom.prepare("value")){
            return false;
        }
        tbl->addColumn(FEATUREVALUECOLUMN,featuredom);
    }

    if (fcoverage->featureTypes() == itPOINT)
        return loadBinaryPoints(fcoverage, tbl);
    else if (fcoverage->featureTypes() == itLINE)
        return loadBinarySegments(fcoverage, tbl);
    else if (fcoverage->featureTypes() == itPOLYGON)
        return loadBinaryPolygons(fcoverage, tbl);
    return false;
}

bool FeatureConnector::loadMetaData(Ilwis::IlwisObject *obj)
{
    bool ok = CoverageConnector::loadMetaData(obj);
    if ( !ok)
        return false;
    FeatureCoverage *fcoverage = static_cast<FeatureCoverage *>(obj);
    IlwisTypes coverageType = itPOINT;

    int features = _odf->value("PointMap","Points").toInt(&ok);
    if (!ok) {
        coverageType = itLINE;
        features = _odf->value("SegmentMapStore","Segments").toInt(&ok);
        if (!ok) {
            coverageType = itPOLYGON;
            features = _odf->value("PolygonMapStore","Polygons").toInt(&ok);
        }
    }

    if (ok){
        fcoverage->featureTypes(coverageType);
        fcoverage->setFeatureCount(coverageType, features);
    }
    else
       return ERROR2(ERR_INVALID_PROPERTY_FOR_2,"Records",obj->name());

    return true;

}

bool FeatureConnector::storeBinaryDataPolygon(FeatureCoverage *fcov, const QString& baseName) {
    QString filename = baseName + ".mpz#";

    std::ofstream output_file(filename.toLatin1(),ios_base::out | ios_base::binary | ios_base::trunc);
    if ( !output_file.is_open())
        return ERROR1(ERR_COULD_NOT_OPEN_WRITING_1,filename);
    IFeatureCoverage cov;
    cov.set(fcov);
    FeatureIterator iter(cov);
    double raw = 1;
    for_each(iter, iter.end(), [&](SPFeatureI feature){
        const Geometry& geom = feature->geometry();
        for(int i=0; i < feature->trackSize(); ++i) {
            if ( geom.ilwisType() == itPOLYGON) {
                Polygon pol = geom.toType<Polygon>();
                writeCoords(output_file, pol.outer());
                output_file.write((char *)&raw,8);
                quint32 holeCount = pol.inners().size();
                output_file.write((char *)&holeCount,4);
                for(const std::vector<Coordinate2d>& coords : pol.inners() ) {
                    writeCoords(output_file, coords);
                }
                ++raw;
            }
        }

    });

    output_file.close();

    return true;
}

bool FeatureConnector::storeBinaryDataLine(FeatureCoverage *fcov, const QString& baseName) {
    BinaryIlwis3Table binTable;
    std::ofstream output_file;
    if(!binTable.openOutput(baseName + ".mps#", output_file))
        return false;
    IFeatureCoverage cov;
    cov.set(fcov);
    FeatureIterator iter(cov);
    double raw = 1;
    IDomain crddom;
    crddom.prepare(cov->resource(IlwisObject::cmOUTPUT).url().toString(), itCOORDDOMAIN);
    if ( !crddom.isValid()) {
        return ERROR2(ERR_NO_INITIALIZED_2, "CoordinateDomain", baseName);
    }
    DataDefinition def(crddom, cov->envelope().clone());
    binTable.addStoreDefinition(def);
    binTable.addStoreDefinition(def);
    binTable.addStoreDefinition(def);
    for_each(iter, iter.end(), [&](SPFeatureI feature){
        const Geometry& geom = feature->geometry();
        for(int i=0; i < feature->trackSize(); ++i) {
            vector<QVariant> record(5);
            if ( geom.ilwisType() == itLINE) {
                 Line2D<Coordinate2d> line = geom.toType<Line2D<Coordinate2d>>();
                 const Coordinate2d& crdmin = geom.envelope().min_corner();
                 const Coordinate2d& crdmax = geom.envelope().max_corner();
                 record[0].setValue(crdmin);
                 record[1].setValue(crdmax);
                 QList<Coordinate2d> points;
                 for(const Coordinate2d& crd: line) {
                    points.push_back(crd);    ;
                 }
                 record[2].setValue(points);
                 record[3].setValue(false);
                 record[4].setValue(raw);
                 binTable.storeRecord(output_file, record);

                 ++raw;
            }
        }

    });

    output_file.close();

    return true;
}

bool FeatureConnector::storeBinaryData(FeatureCoverage *fcov, IlwisTypes type) {
    if ( type == 0)
        return true;
    if (!CoverageConnector::storeBinaryData(fcov, type)) // attribute tables
        return false;
    QFileInfo inf(fcov->name());
    QString dir = context()->workingCatalog()->location().toLocalFile();
    QString baseName = dir + "/" + inf.baseName();
    bool ok = false;
    if ( hasType(type, itPOLYGON)) {
        ok = storeBinaryDataPolygon(fcov, baseName);
    } else if ( hasType(type, itLINE)) {
        ok = storeBinaryDataLine(fcov, baseName)    ;
    }

    return ok;
}

bool FeatureConnector::storeBinaryData(IlwisObject *obj) {

    FeatureCoverage *fcov = static_cast<FeatureCoverage *>(obj);
    IlwisTypes featureTypes = fcov->featureTypes();
    bool ok = true;

    ok &= storeBinaryData(fcov, featureTypes & itPOLYGON);
    ok &= storeBinaryData(fcov, featureTypes & itLINE);
    ok &= storeBinaryData(fcov, featureTypes & itPOINT);

    return ok;
}

void FeatureConnector::writeCoords(std::ofstream& output_file, const std::vector<Coordinate2d>& coords, bool singleton) {
    quint32 crdCount = coords.size();
    if(!singleton) {
        output_file.write((char *)&crdCount,4);
    }
    std::vector<double> crds(crdCount * 3);
    quint32 count = 0;
    for(const Coordinate2d& crd : coords) {
        crds[count] = crd.x();
        crds[count+1] = crd.y();
        crds[count+2] = 0;
        count +=3;
    }
    output_file.write((char *)&crds[0], crdCount * 8 * 3);

}

bool FeatureConnector::storeMetaPolygon(FeatureCoverage *fcov, const QString& dataFile){

    _odf->setKeyValue("BaseMap","Type","PolygonMap");
    _odf->setKeyValue("PolygonMap","Type","PolygonMapStore");
    _odf->setKeyValue("PolygonMap","Toplogical","No");
    _odf->setKeyValue("PolygonMapStore","Format",QString::number(5));
    _odf->setKeyValue("PolygonMapStore","DeletedPolygons",QString::number(0));
    _odf->setKeyValue("Ilwis","Class","ILWIS::Polygon Map");

    QString localFile = dataFile + ".mpz#";
    _odf->setKeyValue("PolygonMapStore","DataPol", localFile);
    _odf->setKeyValue("PolygonMapStore", "Polygons", QString::number(fcov->featureCount(itPOLYGON)));

    return true;
}

bool FeatureConnector::storeMetaData(FeatureCoverage *fcov, IlwisTypes type) {
    if ( type == 0)
        return false;
    bool ok = CoverageConnector::storeMetaData(fcov, type);
    if ( !ok)
        return false;

    QString dataFile = fcov->name();
    int index = dataFile.lastIndexOf(".");
    if ( index != -1) {
        dataFile = dataFile.left(index);
    }

    _odf->setKeyValue("Domain","Type","DomainUniqueID");
    _odf->setKeyValue("DomainSort","Sorting","AlphaNumeric");
    _odf->setKeyValue("DomainSort","Prefix","feature");
    _odf->setKeyValue("DomainSort","Class","Domain UniqueID");
    _odf->setKeyValue("DomainIdentifier","Nr",QString::number(fcov->featureCount(itPOLYGON)));


    if ( fcov->featureTypes() & itPOLYGON){
        ok = storeMetaPolygon(fcov, dataFile);
    }

    _odf->store();
    return ok;
}

bool FeatureConnector::storeMetaData(IlwisObject *obj)
{
    FeatureCoverage *fcov = static_cast<FeatureCoverage *>(obj);
    IlwisTypes featureTypes = fcov->featureTypes();
    bool ok = true;
    ok &= storeMetaData(fcov, featureTypes & itPOLYGON);
    ok &= storeMetaData(fcov, featureTypes & itLINE);
    ok &= storeMetaData(fcov, featureTypes & itPOINT);

    return ok;


}
