#ifndef WFSCONNECTOR_GLOBAL_H
#define WFSCONNECTOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(WFSCONNECTOR_LIBRARY)
#  define WFSCONNECTORSHARED_EXPORT Q_DECL_EXPORT
#else
#  define WFSCONNECTORSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // WFSCONNECTOR_GLOBAL_H
