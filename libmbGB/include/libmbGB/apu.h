// This file is part of libmbGB.
// Copyright (C) 2019 Buenia.
// libmbGB is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// libmbGB is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBMBGB_APU
#define LIBMBGB_APU

#include "mmu.h"
#include "libmbgb_api.h"
#include <functional>
using namespace gb;
using namespace std;

namespace gb
{
    class LIBMBGB_API APU
    {
	public:
	    APU(MMU& memory);
	    ~APU();

	    MMU& apumem;

	    int frametimer = 0;

	    void updateaudio();

	    inline void s1update(int frameseq)
	    {
		s1lengthcountertick(frameseq);
	    }

	    inline void s1lengthcountertick(int frameseq)
	    {
		bool lengthcounterdec = TestBit(frameseq, 0);
	    }

	    inline int getframesequencer()
	    {
		return (frametimer >> 13);
	    }
    };
};

#endif // LIBMBGB_APU
