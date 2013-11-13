#!/usr/bin/env python
# -*- coding: utf-8 -*-

from ilwisobjects import *

def main():
    #muteIssueLogger()
    fc = FeatureCoverage("file:///C:/Users/Poku/dev/Ilwis4/testdata/shape/rainfall.shp")
    if fc:
        print("successfully loaded", fc)
        print(fc ,"contains:",fc.featureCount(),"Features")
        sum = 0
        for f in fc:
            sum += int(f.attribute("MAY"))
            print(f, ":", f.attribute("coverage_key"), ",", f.attribute("MAY"), ",", f.attribute("RAINFALL"));
        print("sum of rainfall values in may:",sum)
        del sum
        print("-----------------------------------------------")
        it = fc.__iter__();
        f = it.next()
        v = f.attribute("RAINFALL")
        try:
            print(int(v))
        except TypeError as err:
            print("caught TypeError:",err)
#        del it
#        del fc
#        try:
#            print(f)
#        except:
#            print("invalid f")
#        del f
#        del v
    else:
        print("couldn't load FeatureCoverage")
    del fc

#    fc = FeatureCoverage("file:///C:/Users/Poku");
#    try:
#        for f in fc:
#            print(f)
#    except IlwisException as err:
#        print("caught error:",err)

    print("-----------------------------------------------")
    Engine.setWorkingCatalog("file:///C:/Users/Poku/dev/Ilwis4/testdata")
    rc = RasterCoverage()
    rc.connectTo("file:///C:/Users/Poku/dev/Ilwis4/testdata/n000302.mpr")
    print("rc.value(342,342,0)=>",rc.value(342,342,0))
    aa7 = Engine.do("aa7.mpr=sin(n000302.mpr)")
    print("sin(n000302.mpr)=>",aa7)
    print("aa7.value(342,342,0)=>",aa7.value(342,342,0))
    print("-----------------------------------------------")
    aaa = rc + rc;
    print(rc, " + ", rc, " = ", aaa)
    print("-----------------------------------------------")
    aa7.connectTo("file:///C:/Users/Poku/dev/Ilwis4/testdata/aa7.tif", "GTiff","gdal",IlwisObject.cmOUTPUT)
    if aa7.store(IlwisObject.smBINARYDATA + IlwisObject.smMETADATA):
        print("successfully saved aa7.mpr.tif")#stores .tif into C:/PATHONDIR/....
    else:
        print("could not save aa7.mpr.tif")



def claudio_example():#and martins solution proposal
#    ilwisengine = ilwisobjects.engine()
#    #create a feature coverage
     distribution = FeatureCoverage()#ilwisengine.features()
#    #link it to a local shape file with species distribution. the attributes will contain an attribute 'distribution'.
#    distribution.connectTo("file://d:/somepath/species.shp");
#    #create a coordinate system; we could also use the csy of the distribution map but lets create (for interface sake) a coordinate system
#    localcoordsystem = ilwisengine.coordinatesystem("+proj=utm +zone=35 +ellps=intl +towgs84=-87,-98,-121,0,0,0,0 +units=m +no_defs")
#    #a variant could be
#    # localcoordsystem = ilwisengine.coordinatesystem("code=epsg:23035")
#    #setting the bounds
#    localcoordsystem.bounds(1003700, 239900, 1004600, 2409000)
#    # create a polygon grid
#    polygongrid = ilwisengine.do("gridding", localcoordsystem,10039939, 2399393, 10045997, 2405000, 1000)
#    #add an attribute for the highest distribution ; name plus domain as parameters. Others could include ranges and such. Keep it simple here
#    polygongrid.addAttribute("highest","numeric")
#    #small trivial algorithm for detecting the highest point attribute per polygon
#    for polygon in polygongrid
#        for point in distribution
#            if polygon.contains(distribution.coordinatesystem(), point)
#                maxval = max(polygon.attribute("highest"), point.attribute("distribution"))
#                polygon.attribute("highest", maxval)

def martin_example():
    nir = RasterCoverage("file:///C:/some/dir/nir_2010.idl")
    vis = RasterCoverage("file:///C:/some/dir/vis_2010.idl")
    ndvi = (nir - vis)/(nir + vis)
    daily_ndvi = ndvi.do("select * where index > "+str(Time("2010-4-1"))+" and index < "+str(Time("2010-4-1")))
    dekadel_ndvi = Engine.do("aggregateraster",daily_ndvi,'Avg',[1,1,Duration(0).setDay(10).toInt()],True)
    dest_coordsys = CoordinateSystem("code=epsg:3148")
    out = dekadel_ndvi.transform(dest_coordsys)
    out.connectTo("file:///C:/some/dir/ndvi_2010-4-1_2010-7-1.netcdf","netcdf","gdal",IlwisObject.cmOUTPUT)
    out.store(IlwisObject.smBINARYDATA + IlwisObject.smMETADATA)

if __name__ == "__main__":
    main()
