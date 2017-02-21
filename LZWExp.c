// Final version (Silver)
// @author Anh Nguyen
// Spring 2015

#include "LZWExp.h"
#include "MyLib.h"
#include "SmartAlloc.h"
#define INT_LENGTH 32

/* Initialize a LZWExp the DataSink to which to send uncompressed symbol
 * sequences and an anonymous state object for the DataSink.
 */
void LZWExpInit(LZWExp *exp, DataSink sink, void *sinkState, int recycleCode) {
   int ndx = 0;

   exp->dict = CreateCodeSet(recycleCode);
   while (ndx <= EOD) {
      NewCode(exp->dict, ndx++);
   }

   exp->sink = sink;
   exp->sinkState = sinkState;
   exp->lastCode = EOD;
   exp->numBits = START_NUMBIT;
   exp->maxCode = (1 << exp->numBits) - 1;
   exp->recycleCode = recycleCode;
   buInit(&exp->bitUnpacker);
   exp->EODSeen = 0;
}

/* get a sequence symbol from dictionary, sink it
 * then
 * reset Dictionay to initial state */
void RecycleDictionary(LZWExp *exp, UInt lastValue) {
   int ndx = 0;
   Code sequenceSymbol = GetCode(exp->dict, lastValue);
   char suffixToSet = *sequenceSymbol.data;

   if (lastValue == exp->lastCode) {
      *(sequenceSymbol.data + sequenceSymbol.size - 1) = suffixToSet;
   }
   exp->sink(&sequenceSymbol, sequenceSymbol.data, sequenceSymbol.size);
   FreeCode(exp->dict, lastValue);

   DestroyCodeSet(exp->dict);

   exp->dict = CreateCodeSet(exp->recycleCode);
   while (ndx <= EOD) {
      NewCode(exp->dict, ndx++);
   }
   
   exp->numBits = START_NUMBIT;
   exp->lastCode = EOD;
   exp->maxCode = (1 << exp->numBits) - 1;
}

/* update Dictionary including getting the sequence of symbol
 * adding a new sequence of symbol to the dictionary
 * and set suffix for the previous added sequence
 *
 * Send data to sink if it is a valid data.
 */
static void UpdateDictionary(LZWExp *exp, UInt lastValue) {
   Code sequenceSymbol = GetCode(exp->dict, lastValue);
   char suffixToSet = *sequenceSymbol.data;
   
   if (lastValue != EOD && lastValue <= exp->lastCode) {
      SetSuffix(exp->dict, exp->lastCode, suffixToSet);
      ExtendCode(exp->dict, lastValue);

      if (lastValue == exp->lastCode) {
         *(sequenceSymbol.data + sequenceSymbol.size - 1) = suffixToSet;
      }

      exp->sink(&sequenceSymbol, sequenceSymbol.data, sequenceSymbol.size);
   }

   FreeCode(exp->dict, lastValue);
}

/* update all the statistic in LZWEXp struct
 * after a successful unpack dat
 */
static void UpdateLZWExpStats(LZWExp *exp, UInt *ret) {
   exp->lastCode++;
   if (exp->lastCode > exp->maxCode) {
      exp->numBits++;
   }
   exp->maxCode = (1 << exp->numBits) - 1;
   if (*ret == EOD) {
      exp->EODSeen = 1;
   }
}

/* Breaks apart compressed data in "bits" into one or more codes and send
 * the corresponding symbol sequences to the DataSink. Save any leftover
 * compressed bits to combine with the bits from the next call of LZWExpDecode.
 *
 * Returns 0 on successful processing of a code (not done), 1 when EOD is
 * received (done), or BAD_CODE if a code that is not in the dictionary is
 * received (problem).
 */
int LZWExpDecode(LZWExp *exp, UInt bits) { 
   int validTake = TRUE;
   UInt ret = 0;
   int unpackStatus = TRUE;

   buTakeData(&exp->bitUnpacker, bits);

   while (unpackStatus && !exp->EODSeen) {
      unpackStatus = unpack(&exp->bitUnpacker, exp->numBits, &ret);
      if (!unpackStatus && validTake) {
         buTakeData(&exp->bitUnpacker, bits);
         validTake = FALSE;
      }
      
      if (unpackStatus && exp->lastCode == exp->recycleCode - 1) {
         RecycleDictionary(exp, ret);
      }
      else if (unpackStatus && ret <= exp->lastCode) {
         UpdateDictionary(exp, ret);
         UpdateLZWExpStats(exp, &ret);
      }
      else if (unpackStatus && ret > exp->lastCode) {
         return BAD_CODE;
      }

      if (exp->EODSeen) {
         return 1;
      }
   }

   return 0;
}

/* Called when LZWExpDecode is done to peform error checking and/or
 * housekeeping that should be performed at the end of decoding.
 *
 * Returns 0 if all is okay, MISSING_EOD if no terminating EOD was found, or
 * CORRUPT_FILE if non-zero bits follow the EOD.
 */
int LZWExpStop(LZWExp *exp) {
   int toReturn = 0;
    
   if (exp->bitUnpacker.bitsLeft &&
    exp->bitUnpacker.curData << (INT_LENGTH - exp->bitUnpacker.bitsLeft)) {
      toReturn = CORRUPT_FILE;
   }

   if (!exp->EODSeen) {
      toReturn = MISSING_EOD;
   }

   return toReturn;
}

/* Free all storage associated with LZWExp (not the sinkState, though,
 * which is "owned" by the caller.  Must be called even if LZWExpInit
 * returned an error.
 */
void LZWExpDestruct(LZWExp *exp) {
   DestroyCodeSet(exp->dict);
}



