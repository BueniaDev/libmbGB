//
// You should have received a copy of the GNU General Public License
// along with libmbGB.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBMBGB_SERIAL
#define LIBMBGB_SERIAL

#include "mmu.h"
#include "libmbgb_api.h"
#include <functional>
using namespace gb;
using namespace std;

namespace gb
{
    class LIBMBGB_API Serial
    {
	public:
	    Serial(MMU& memory);
	    ~Serial();

	    void init();
	    void shutdown();

	    MMU& serialmem;

	    int serialclock = 0;
	    int bitstoshift = 0;
	    bool previnc = false;

	    bool transfersignal = false;
	    bool prevtransfersignal = false;

	    void updateserial();

	    inline void initserialclock(uint8_t initval)
	    {
		serialclock = initval;
	    }

	    inline void shiftserialbit()
	    {
		serialmem.sb <<= 1;

		serialmem.sb |= 0x01;

		if (--bitstoshift == 0)
		{
		    serialmem.sc &= 0x7F;
		    serialmem.requestinterrupt(3);
		}
	    }

	    inline int selectclockbit()
	    {
		return 7;
	    }

	    inline bool usinginternalclock()
	    {
		return (TestBit(serialmem.sc, 0));
	    }
    };
};

#endif // LIBMBGB_SERIAL
