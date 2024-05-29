#ifndef DSINFERCOREGLOBAL_H
#define DSINFERCOREGLOBAL_H

#ifdef _MSC_VER
#  define DSINFER_CORE_DECL_EXPORT __declspec(dllexport)
#  define DSINFER_CORE_DECL_IMPORT __declspec(dllimport)
#else
#  define DSINFER_CORE_DECL_EXPORT __attribute__((visibility("default")))
#  define DSINFER_CORE_DECL_IMPORT __attribute__((visibility("default")))
#endif

#ifndef DSINFER_CORE_EXPORT
#  ifdef DSINFER_CORE_STATIC
#    define DSINFER_CORE_EXPORT
#  else
#    ifdef DSINFER_CORE_LIBRARY
#      define DSINFER_CORE_EXPORT DSINFER_CORE_DECL_EXPORT
#    else
#      define DSINFER_CORE_EXPORT DSINFER_CORE_DECL_IMPORT
#    endif
#  endif
#endif



#endif // DSINFERCOREGLOBAL_H
