/*
    This file is part of libmbGB.
    Copyright (C) 2021 BueniaDev.

    libmbGB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libmbGB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.
*/

    
#ifndef LIBMBGB_API_H
#define LIBMBGB_API_H

#if defined(_MSC_VER) && !defined(LIBMBGB_STATIC)
    #ifdef LIBMBGB_EXPORTS
        #define LIBMBGB_API __declspec(dllexport)
    #else
        #define LIBMBGB_API __declspec(dllimport)
    #endif // LIBMBGB_EXPORTS
#else
    #define LIBMBGB_API
#endif // _MSC_VER

#endif // LIBMBGB_API_H
