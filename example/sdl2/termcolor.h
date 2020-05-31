#ifdef __WIN32__
#include <windows.h>
#endif

#include <cstdio>
#include <unistd.h>

#ifdef __WIN32__
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

namespace beeterm
{
    namespace _internal
    {
    	static int colorizeindex = std::ios_base::xalloc();
    
        inline FILE* getstandardstream(const std::ostream& stream)
        {
            if (&stream == &std::cout)
            {
                return stdout;
            }
            else if ((&stream == &std::cerr) || (&stream == &std::clog))
            {
                return stderr;
            }
            
            return nullptr;
        }
        
        inline bool isatty(const std::ostream& stream)
        {
            FILE* stdstream = getstandardstream(stream);
            
            if (!stdstream)
            {
                return false;
            }
            
            #ifdef __WIN32__
            return ::_isatty(_fileno(stdstream));
            #else
            return ::isatty(fileno(stdstream));
            #endif
        }
        
        inline bool iscolorized(std::ostream& stream)
        {
            return (isatty(stream) || static_cast<bool>(stream.iword(colorizeindex)));
        }
    };

    inline bool isinit = false;
    
    #ifdef __WIN32__
    static HANDLE stdouthandle;
    static DWORD outmodeinit;
    #endif

    inline void initconsole()
    {
        #ifdef __WIN32__
        DWORD outmode = 0;
        
        stdouthandle = GetStdHandle(STD_OUTPUT_HANDLE);
        
        if (stdouthandle == INVALID_HANDLE_VALUE)
        {
            exit(GetLastError());
        }
        
        if (!GetConsoleMode(stdouthandle, &outmode))
        {
            exit(GetLastError());
        }
        
        outmodeinit = outmode;
        outmode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        
        if (!SetConsoleMode(stdouthandle, outmode))
        {
            exit(GetLastError());
        }
        #endif
        
        isinit = true;
    }
    
    inline std::ostream& blue(std::ostream& stream)
    {
        if (_internal::iscolorized(stream))
        {
            #ifdef __WIN32__
            if (!isinit)
            {
                initconsole();
            }
            #endif
        
            stream << "\033[34m";
        }
        
        return stream;
    }
    
    inline std::ostream& white(std::ostream& stream)
    {
        if (_internal::iscolorized(stream))
        {
            #ifdef __WIN32__
            if (!isinit)
            {
                initconsole();
            }
            #endif
        
            stream << "\033[37m";
        }
        
        return stream;
    }
    
    inline std::ostream& dark(std::ostream& stream)
    {
        if (_internal::iscolorized(stream))
        {
            #ifdef __WIN32__
            if (!isinit)
            {
                initconsole();
            }
            #endif
        
            stream << "\033[2m";
        }
        
        return stream;
    }
    
    inline std::ostream& reset(std::ostream& stream)
    {
        if (_internal::iscolorized(stream))
        {
            #ifdef __WIN32__
            if (!isinit)
            {
                initconsole();
            }
            #endif
        
            stream << "\033[00m";
            
            #ifdef __WIN32__
            if (!SetConsoleMode(stdouthandle, outmodeinit))
            {
                exit(GetLastError());
            }
            
            isinit = false;
            #endif
        }
        
        return stream;
    }
};
