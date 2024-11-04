#ifndef DSUTILSGLOBAL_H
#define DSUTILSGLOBAL_H

#include <memory>

#include <stdcorelib/global.h>
#include <stdcorelib/pimpl.h>

#ifndef DSUTILS_EXPORT
#  ifdef DSUTILS_STATIC
#    define DSUTILS_EXPORT
#  else
#    ifdef DSUTILS_LIBRARY
#      define DSUTILS_EXPORT STDCORELIB_DECL_EXPORT
#    else
#      define DSUTILS_EXPORT STDCORELIB_DECL_IMPORT
#    endif
#  endif
#endif

#endif // DSUTILSGLOBAL_H
