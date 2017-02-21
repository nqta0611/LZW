// Final version (Silver)
// @author Anh Nguyen
// Spring 2015

#include "MyLib.h"
#include "stdlib.h"

typedef struct CodeEntry {
   struct CodeEntry *prefix;
   unsigned char data;
   int size;
   unsigned char *allData;
    
} CodeEntry;

typedef struct CodeSet {
   CodeEntry *entries;
   int ndx;
} CodeSet;

/* Allocate, initialize, and return a CodeSet object, via void *
 * The CodeSet will have room for |numCodes| codes, though it will
 * initially be empty. */
void *CreateCodeSet(int numCodes) {
   CodeSet *codeSet = calloc(1, sizeof(CodeSet));

   codeSet->entries = calloc(numCodes, sizeof(CodeEntry));

   return codeSet;
}

/* Add a new 1-byte code to |codeSet|, returning its index, with
 * the first added code having index 0.  The new code's byte is
 * equal to |val|.  Assume (and assert if needed) that there
 * is room in the |codeSet| for a new code. */
int NewCode(void *codeSet, char val) {
   CodeSet *set = codeSet;
   CodeEntry *entries = set->entries;

   entries += set->ndx;
   entries->data = val;
   entries->size++;

   return set->ndx++;
}

/* Create a new code by copying the existing code at index
 * |oldCode| and extending it by one zero-valued byte.  Any
 * existing code might be extended, not just the most recently
 * added one. Return the new code's index.  Assume |oldCode|
 * is a valid index and that there is enough room for a new code. */
int ExtendCode(void *codeSet, int oldCode) {
   CodeSet *set = codeSet;
   CodeEntry *entries = set->entries;
   CodeEntry *newEntry = entries + set->ndx;

   entries += oldCode;
   newEntry->prefix = entries;
   newEntry->size = entries->size + 1;

   return set->ndx++;
}

/* Set the final byte of the code at index |code| to |suffix|.
 * This is used to override the zero-byte added by ExtendCode.
 * If the code in question has been returned by a GetCode call,
 * and not yet freed via FreeCode, then the changed final byte
 * will also show in the Code data that was returned from GetCode.*/
void SetSuffix(void *codeSet, int code, char suffix) {
   CodeSet *set = codeSet;
   CodeEntry *entries = set->entries;
   unsigned char *setSuffix;
 
   entries += code;
   entries->data = suffix;
   if (entries->allData) {
      setSuffix = entries->allData + entries->size - 1;
      *setSuffix = suffix;
   }
}

/* Return the code at index |code| */
Code GetCode(void *codeSet, int code) {
   CodeSet *set = codeSet;
   CodeEntry *entries = set->entries;
   Code toReturn;
   unsigned char *returnData;
   int size;

   entries += code;
   toReturn.size = size = entries->size;

   if (entries->allData) {
      toReturn.data = entries->allData;
   }
   else {
      returnData = malloc(sizeof(char) * entries->size);
      toReturn.data = returnData;
      returnData += size - 1;

      while (entries && !entries->allData && size--) {
         *returnData = entries->data;
         returnData--;
         entries = entries->prefix;
      }

      if (size) {
         memcpy(toReturn.data, entries->allData, entries->size);
      }
      entries = set->entries + code;
      entries->allData = toReturn.data;
   } 
   return toReturn;
}

/* Mark the code at index |code| as no longer needed, until a new
 * GetCode call is made for that code. */
void FreeCode(void *codeSet, int code) {
   CodeSet *set = codeSet;
   CodeEntry *entries = set->entries;

   entries += code;
   if (entries->allData) {
      free(entries->allData);
      entries->allData = NULL;
   }
}

/* Free all dynamic storage associated with |codeSet| */
void DestroyCodeSet(void *codeSet) {
   CodeSet *set = codeSet;
   CodeEntry *entries = set->entries;
   int index = set->ndx;

   while (index--) {
      if (entries->allData) {
         free(entries->allData);
      }
      entries++;
   }

   free(set->entries);
   free(set);
}
