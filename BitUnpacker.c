// Final version (Silver)
// @author Anh Nguyen
// Spring 2015

#include "MyLib.h"
#include "BitUnpacker.h"

#define UINT_SIZE 32
#define UINT_MASK 0xFFFFFFFF

/* Initialize every data to 0 */
void buInit(BitUnpacker *bu) {
   bu->curData = 0;
   bu->nextData = 0;
   bu->bitsLeft = 0;
   bu->validNext = 0;
}

/* this function is to take data into the newData filed,
 * then turn on the validNext flag */
void buTakeData(BitUnpacker *bu, UInt newData) {
   bu->validNext = 1;
   bu->nextData = newData;
}

/* unpack data
 * return 0 if fail to unpack
 * return 1 if unpack sucessfully*/
int unpack (BitUnpacker *bu, int size, UInt *ret) {
    
   if (!bu->bitsLeft && bu->validNext) {
      bu->bitsLeft = UINT_SIZE;
      bu->curData = bu->nextData;
      bu->validNext = 0;
   }

   if (bu->bitsLeft >= size) {
      *ret = (size < UINT_SIZE) ? (bu->curData >> (bu->bitsLeft - size)) &
       ((1<<size)-1) : bu->curData;
      bu->bitsLeft -= size;
      return 1;
   }
   else if (!bu->validNext) {
      return 0;
   }
   else {
      *ret = (((bu->curData & ((1 << bu->bitsLeft)-1)) <<
       (size - bu->bitsLeft)))|
        (bu->nextData >> (UINT_SIZE - size + bu->bitsLeft));
      bu->bitsLeft = UINT_SIZE - size + bu->bitsLeft;
      bu->curData = bu->nextData;
      bu->validNext = 0;
      return 1;
   }
}
