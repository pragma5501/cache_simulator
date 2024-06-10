#include "type.h"

u16 _log2(u16 x) 
{
        u16 log = 0;

        while (x >>= 1) {
                log++;
        }
        return log;
}
