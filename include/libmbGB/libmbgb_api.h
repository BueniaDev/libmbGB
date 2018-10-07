#ifndef LIBMBGB_API_H
#define LIBMBGB_API_H

#ifdef _WIN32
    #ifdef LIBMBGB_EXPORTS
        #define LIBMBGB_API __declspec(dllexport)
    #else
        #define LIBMBGB_API __declspec(dllimport)
    #endif // LIBMBGB_EXPORTS

    #define LIBMBGB __cdecl
#else
    #define LIBMBGB_API
    #define LIBMBGB
#endif // _WIN32

#endif // LIBMBGB_API_H
