#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <complex.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


void print_imsi(FILE *csv, char payload[], int index, int len, bool end){
  fprintf(csv, "\"");
  for (int i=0; i<len; i++){
    fprintf(csv, "%c", payload[index+i]);
  }
  fprintf(csv, "\"");
  if (!end){
    fprintf(csv, ";");
  }
}

void print_s_tmsi(FILE *csv, char payload[], int index, int len, bool first, bool end){
 for (int i=0; i<len; i++){
    fprintf(csv, "%c", payload[index+i]);
  }
}

int main(void){
  FILE *fp;
  FILE *csv;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen("imsi.txt", "r");
  csv = fopen("imsi.csv", "w");

  fprintf(csv, "\"IMSI\";\"Timestamp\";\"Country\";\"Operator\";\"MSIN\";\"s-TSMI\"\n");

  while ((read = getline(&line, &len, fp)) != -1){
    char read_line[len];
    sprintf(read_line, "%s", line);

    char payload[len];
    sprintf(payload, "%s", line);
    sprintf(payload, "%s", strstr(payload, ";"));


    char *time;
    char check_s_tmsi[32];
    int index;
    int imsi_index;
    int counter;
    int imsi_length;
    bool s_tmsi_check = false;
    bool s_tmsi_checked = false;
    bool found_imsi = false;

    //Look for IMSI
    for (int i=0; i<strlen(payload); i++){
      if (payload[i] == '9' && payload[i+1] == '5' && !(payload[i+2]=='0' && payload[i+3]=='0') && !(payload[i+4]=='0' && payload[i+5]=='0') && !(payload[i+6]=='0' && payload[i+7]=='0' && payload[i+8]=='0' && payload[i+9]=='0') && (payload[i+15] == '8' || payload[i+16] == '8')){
        for (int j=16; j>14; j--){
          if (payload[i+j] == '8'){
            imsi_index = i+1;
            imsi_length = j-1;
            found_imsi = true;
            break;
          }
        }
      }
      if (found_imsi){
        for (int k=i+1; k<=i+15; k++){
          if (isdigit(payload[k])==0){
            found_imsi = false;
          }
        } 
        if (found_imsi){
          break;
        }
      }
    }

    if (found_imsi){
      //get time
      time = strtok(read_line, ";");

      if (strstr(payload, "800000000") == NULL || imsi_index > 14){
        s_tmsi_check = true;
      }

      //print IMSI
      print_imsi(csv, payload, imsi_index, imsi_length, false);
      fprintf(csv, "\"%s\";", time);
      print_imsi(csv, payload, imsi_index, 3, false);
      print_imsi(csv, payload, imsi_index+3, 2, false);
      print_imsi(csv, payload, imsi_index+5, 10, !s_tmsi_check);
      //if s-tmsi is caught in the same frame
      counter = 0;
      index = imsi_index;
      if (s_tmsi_check){
        fprintf(csv, "\"");
        while (counter<2){
          if (counter == 1){
            s_tmsi_checked = true;
          }
          if (imsi_index > 14 && index > 14 && counter != 1){
            print_s_tmsi(csv, payload, index-11, 10, s_tmsi_check, s_tmsi_checked); 
            index -= 10;
            s_tmsi_check = false;
          } /*else if (payload[imsi_length+index+2] != '0' && payload[imsi_length+index+3] != '0' && index >= imsi_index && index+imsi_length < strlen(payload)){
            print_s_tmsi(csv, payload, index+imsi_length+1, 10, s_tmsi_check, s_tmsi_checked);
            index += 10;
            s_tmsi_check = false;
          } */else {
            index = imsi_index;
            counter++;
          }
          if (counter != 2){
            fprintf(csv, ", ");
          }
        }  
        fprintf(csv, "\"");
      }
      fprintf(csv, "\n");
    }
  }


  fclose(fp);
  fclose(csv);
  /*if (line){
    free(line);
  }*/
}