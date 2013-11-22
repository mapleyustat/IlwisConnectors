#ifndef PYTHONAPI_OBJECT_H
#define PYTHONAPI_OBJECT_H

//Qt typedefs only for GNU compiler
typedef signed char qint8;         /* 8 bit signed */
typedef unsigned char quint8;      /* 8 bit unsigned */
typedef short qint16;              /* 16 bit signed */
typedef unsigned short quint16;    /* 16 bit unsigned */
typedef int qint32;                /* 32 bit signed */
typedef unsigned int quint32;      /* 32 bit unsigned */
typedef long long qint64;           /* 64 bit signed */
typedef unsigned long long quint64; /* 64 bit unsigned */
typedef quint64 IlwisTypes;

namespace pythonapi{

    class Object{
    public:
        Object(){}
        virtual ~Object(){}
        virtual bool __bool__() const = 0;
        virtual const char* __str__() = 0;
        virtual IlwisTypes ilwisType() = 0;
    };

}
#endif // PYTHONAPI_OBJECT_H