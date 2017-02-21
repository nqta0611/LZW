//  LinearSim
//
//  Created by Anh Nguyen on 5/31/15.
//  Copyright (c) 2015 Anh Nguyen. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define LENGTH 41
#define LENGTH2 24


typedef struct {
    int id;       // ID of the Cell reporting
    int step;     // Time for which report is being made
    double value; // Value of the cell at time "step"
} Report;

typedef struct Data {
   int cell;
   int simulate;
   double leftValue;
   double rightValue;
} Data;

void InitData(Data *data) {
   data->cell = 0;
   data->simulate = 0;
   data->leftValue = 0.0;
   data->rightValue = 0.0;
}

void MakeIntArgument(char type, int value, char *arg) {
   char *temp = arg;
   
   *temp = type;
   temp++;
   sprintf(temp, "%d", value);
}

void MakeDoubleArgument(char type, double value, char *arg) {
   char *temp = arg;
   
   *temp = type;
   temp++;
   sprintf(temp, "%f", value);
}

void ExecuteData(Data *data) {
   Report toPrint;
   int pipe1[2];
   int pipe2[2];
   int pipe3[2];
   int pipe4[2];
   int pipe5[2];
   int childPid;
   char input[5];
   char input2[5];
   char output[5];
   char output2[5];
   char sim[5];
   char val[15];
   char cellId[5];
   int loop;
   int cell = data->cell;

   pipe(pipe1);
   if ((childPid = fork()) > 0){
      close(pipe1[1]);
      while(cell--)
      wait(NULL);
      for(loop = 0; loop < data->cell * (data->simulate + 1); loop++) {
         read(pipe1[0], &toPrint, sizeof(Report));
         printf("Result from %d, step %d: %.3f\n", toPrint.id, toPrint.step, toPrint.value);
      }
      close(pipe1[0]);
   }
   else if (data->cell == 1){
      close(pipe1[0]);
      MakeIntArgument('S', data->simulate, sim);
      MakeIntArgument('O', pipe1[1], output);
      MakeIntArgument('D', 0, cellId);
      MakeDoubleArgument('V', data->leftValue, val);
      execl("Cell", "Cell", sim, output, val, cellId, NULL);
      printf("fail to exec\n");
   }
   else if (data->cell == 2){
      close(pipe1[0]);
      if ((childPid = fork()) > 0) {
         MakeIntArgument('S', data->simulate, sim);
         MakeIntArgument('O', pipe1[1], output);
         MakeIntArgument('D', 0, cellId);
         MakeDoubleArgument('V', data->leftValue, val);
         execl("Cell", "Cell", sim, output, val, cellId, NULL);
         printf("fail to exec\n");
           
    }
      else {
         MakeIntArgument('S', data->simulate, sim);
         MakeIntArgument('O', pipe1[1], output);
         MakeIntArgument('D', 1, cellId);
         MakeDoubleArgument('V', data->rightValue, val);
         execl("Cell", "Cell", sim, output, val, cellId, NULL);
         printf("fail to exec\n");
      }
   }
   else if (data->cell == 3){
      close(pipe1[0]);
      pipe(pipe2);
      if ((childPid = fork()) > 0){
         close(pipe2[0]);
         MakeIntArgument('S', data->simulate, sim);
         MakeIntArgument('O', pipe1[1], output);
         MakeIntArgument('O', pipe2[1], output2);
         MakeIntArgument('D', 0, cellId);
         MakeDoubleArgument('V', data->leftValue, val);
         execl("Cell", "Cell", sim, output, output2, val, cellId, NULL);
         printf("fail to exec\n");
      }
      else {
         close(pipe2[1]);
         pipe(pipe3);
         if ((childPid = fork()) > 0){
            close(pipe3[0]);
            MakeIntArgument('S', data->simulate, sim);
            MakeIntArgument('O', pipe1[1], output);
            MakeIntArgument('O', pipe3[1], output2);
            MakeIntArgument('D', 2, cellId);
            MakeDoubleArgument('V', data->rightValue, val);
            execl("Cell", "Cell", sim, output, output2, val, cellId, NULL);
            printf("fail to exec\n");
         }
         else {
            MakeIntArgument('S', data->simulate, sim);
            MakeIntArgument('O', pipe1[1], output);
            MakeIntArgument('I', pipe2[0], input);
            MakeIntArgument('I', pipe3[0], input2);
            MakeIntArgument('D', 1, cellId);
            execl("Cell", "Cell", sim, output, input, input2, cellId, NULL);
            printf("fail to exec\n");
         }
      }
   }
}

int main(int argc, const char * argv[]) {
   Data dataSet;
   char argType;
   char *usageError = "Usage: LinearSim C S L R (in any order)\n";
   //char *exitStatus = "Nonzero exit status: 1\n";

   int hadLeft = 0;
   int hadRight = 0;
   int hadCell = 0;
   int hadSimulate = 0;

   InitData(&dataSet);

   while (argc--) {
      argType = *argv[argc];
      switch (argType){
      case 'C':
         hadCell = 1;
         dataSet.cell = atoi(argv[argc] + 1);
         break;
      case 'S':
         hadSimulate = 1;
         dataSet.simulate = atoi(argv[argc] + 1);
         break;
      case 'L':
         hadLeft = 1;
         dataSet.leftValue = atof(argv[argc] + 1);
         break;
      case 'R':
         hadRight = 1;
         dataSet.rightValue = atof(argv[argc] + 1);
         break;
      }
   }
   
   
   if ((dataSet.cell == 1) && (hadRight || !hadLeft || !hadSimulate || !hadCell)) {
      write(2, usageError, LENGTH);
      return 1;
   }
   
   if ((dataSet.cell > 1) && (!hadLeft || !hadRight || !hadSimulate || !hadCell)){
      write(2, usageError, LENGTH);
      return 1;
   }
   
   ExecuteData(&dataSet);
    
   return 0;
}
