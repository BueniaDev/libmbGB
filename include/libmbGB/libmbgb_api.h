#ifndef LIBMBGB_API_H
#define LIBMBGB_API_H

#if defined(_WIN32) && !defined(LIBMBGB_STATIC)
    #ifdef LIBMBGB_EXPORTS
        #define LIBMBGB_API __declspec(dllexport)
    #else
        #define LIBMBGB_API __declspec(dllimport)
    #endif // LIBMBGB_EXPORTS

    #define LIBMBGB __cdecl
    #define WIN32_AUDIO
#else
    #define LIBMBGB_API
    #define LIBMBGB
    #define UNIX_AUDIO
#endif // _WIN32

#endif // LIBMBGB_API_H
