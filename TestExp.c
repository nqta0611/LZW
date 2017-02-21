`// Final version (Silver)
// @author Anh Nguyen
// Spring 2015

#include "LZWExp.h"
#include "MyLib.h"
#include "SmartAlloc.h"

#define NINE 9
#define TWELVE 12
#define THIRTEEN 13

/* a function to sink all information */
void DataSink1(void *a, unsigned char *data, int numBytes) {
   while (numBytes--) {
      printf("%c", *data++);
   }
}

/* running the test of LZW program with -R flag indicates
 * the recycle code.*/
int main(int argc, char *argv[]) {
   int recycleCode;
   int *sinkState = NULL;
   LZWExp stats;
   UInt dataIn = 0;
   int decodeError = 0;
   char *error1 = "Corrupt file\n";
   char *error2 = "Bad code\n";
   char *error3 = "Missing EOD\n";

   if (argc <= 2)
      LZWExpInit(&stats, DataSink1, sinkState, DEFAULT_RECYCLE_CODE);
   else if (argv[1][0] == '-' && argv[1][1] == 'R') {
      recycleCode = atoi(argv[2]);
    
      if (recycleCode > EOD) {
         LZWExpInit(&stats, DataSink1, sinkState, recycleCode);
      }
      else {
         LZWExpInit(&stats, DataSink1, sinkState, DEFAULT_RECYCLE_CODE);
      }
   }

   while (!decodeError && EOF != scanf(" %x", &dataIn)) {
      decodeError = LZWExpDecode(&stats, dataIn);
   }
    
   if (decodeError == BAD_CODE) {
      DataSink1(sinkState, (unsigned char *)error2, NINE);
   }
   else if (EOF != scanf(" %x", &dataIn)) {
      DataSink1(sinkState, (unsigned char *)error1, THIRTEEN);
   }
   else {
      decodeError = LZWExpStop(&stats);
      if (decodeError == MISSING_EOD) {
         DataSink1(sinkState, (unsigned char *)error3, TWELVE);
      }
      if (decodeError == CORRUPT_FILE) {
         DataSink1(sinkState, (unsigned char *)error1, THIRTEEN);
      }
   }
 
   LZWExpDestruct(&stats);

   if (report_space()) 
      printf("%lu", report_space());

   return 0;
}













