#ifndef DSINFERCOREGLOBAL_H
#define DSINFERCOREGLOBAL_H

#include <stdcorelib/global.h>
#include <stdcorelib/pimpl.h>

#ifndef DSINFER_EXPORT
#  ifdef DSINFER_STATIC
#    define DSINFER_EXPORT
#  else
#    ifdef DSINFER_LIBRARY
#      define DSINFER_EXPORT STDCORELIB_DECL_EXPORT
#    else
#      define DSINFER_EXPORT STDCORELIB_DECL_IMPORT
#    endif
#  endif
#endif

#endif // DSINFERCOREGLOBAL_H
