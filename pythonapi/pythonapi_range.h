#ifndef PYTHONAPI_RANGE_H
#define PYTHONAPI_RANGE_H

#include "pythonapi_object.h"
#include<string>
#include<map>

namespace Ilwis {
class Range;
}

typedef struct _object PyObject;

namespace pythonapi {


class Range: public Object{
public:
    friend class Domain;

    bool __bool__() const;
    std::string __str__();
    IlwisTypes ilwisType();

    IlwisTypes valueType() const;
    PyObject* ensure(const PyObject* v, bool inclusive = true) const;
    bool contains(const PyObject *value, bool inclusive = true) const;

    bool isContinuous() const;
    PyObject* impliedValue(const PyObject *value) const;

protected:
    std::shared_ptr<Ilwis::Range> _range;
    Range(Ilwis::Range *rng);

    Range();
private:
};

class NumericRange : public Range {
public:
    NumericRange(double mi, double ma, double resolution=0);
    NumericRange(const NumericRange &vr);


    bool contains(double v, bool inclusive = true) const;
    double max() const;
    void setMax(double v);
    double min() const;
    void setMin(double v);
    double distance() const;

    void setResolution(double resolution);
    double resolution() const ;

    void set(const NumericRange& vr);
    void clear();
};

class ItemRange : public Range {
public:
    virtual void add(PyObject *dItem) = 0;
    void count();
    void remove(const std::string& name);
    void clear();
};

class NumericItemRange : public ItemRange{
public:
    NumericItemRange();
    void add(PyObject *item);
};

class NamedItemRange : public ItemRange {
public:
    NamedItemRange();
    void add(PyObject *item);
};

class ThematicRange : public ItemRange {
public:
    ThematicRange();
    void add(PyObject *item);
};

#ifdef SWIG
%rename(ColorModel) ColorModelNS;
#endif

struct ColorModelNS{
    enum Value{cmRGBA, cmHSLA, cmCYMKA, cmGREYSCALE};
};

typedef ColorModelNS::Value ColorModel;

class Color{
public:
    Color();
    Color(ColorModel type, PyObject* obj);
    void readColor(ColorModel type, PyObject* obj);
    double getItem(std::string key) const;
    ColorModel getColorModel() const;
    std::string toString() const;
private:
    ColorModel _type = ColorModel::cmRGBA;
    PyObject* _colorVal;
};


class ColorRange : public Range{
public:

    ColorRange();
    ColorRange(IlwisTypes tp, ColorModel clrmodel);
    ColorModel defaultColorModel() const;
    void defaultColorModel(ColorModel m);

    //static Color toColor(quint64 clrint, ColorModel clrModel) ;
    static Color toColor(PyObject*, ColorModel colortype);
    std::string toString(const Color &clr, ColorModel clrType);
    ColorModel stringToColorModel(std::string clrmd);

private:
    IlwisTypes _valuetype;
    ColorModel _defaultModel = ColorModel::cmRGBA;
};

class ContinousColorRange : public ColorRange{
public:
    ContinousColorRange();
    ContinousColorRange(const Color& clr1, const Color& clr2, ColorModel colormodel=ColorModel::cmRGBA);
    std::string toString() const;
    bool isValid() const;
    ContinousColorRange *clone() const;
    PyObject* ensure(const PyObject *v, bool inclusive = true) const;
    bool containsVar(const PyObject *v, bool inclusive = true) const;
    bool containsColor(const Color &clr, bool inclusive = true) const;
    bool containsRange(ColorRange *v, bool inclusive = true) const;
    Color impliedValue(const PyObject* v) const;
};

class TimeInterval : public NumericRange{
public:
    TimeInterval(IlwisTypes tp = itUNKNOWN);
    TimeInterval(PyObject* beg, PyObject* end, std::string step="", IlwisTypes tp = itUNKNOWN);

    //TimeInterval& operator=(const TimeInterval& tiv);
    PyObject* begin() const;
    PyObject* end() const ;
    void begin(const PyObject* t) ;
    void end(const PyObject* t);
    //Duration getStep() const { return _step;}
    std::string toString(bool local, IlwisTypes) const;
    bool contains(const std::string& value, bool inclusive = true) const;
    bool contains(const PyObject* value, bool inclusive = true) const;

    Ilwis::Range *clone() const ;
    bool isValid() const;
};

}

#endif // PYTHONAPI_RANGE_H
