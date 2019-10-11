#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <complex.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

#include "srslte/phy/utils/vector.h"

char* gettime(){
  time_t mytime = time(NULL);
  char * time_str = ctime(&mytime);
  time_str[strlen(time_str)-1] = '\0';
  return time_str;
}

void remove_last_character(const char *input, char *output){
  int len = strlen(input);
  if(len > 1){
    output[len - 1] = '\0';
  }
}

/*Printing functions below*/
//Convert the system frame number bits in the MIB to decimal
/*void print_MIB(FILE *stream, uint8_t *x, const uint32_t len) {
  fprintf(stream, "Original bits: "); //MIB bits
  for (int i=0;i<len;i++) {
    fprintf(stream, "%d", x[i]);
  }
  fprintf(stream, "\n");
  int sfn_deci = 0;
  fprintf(stream, "System frame number: "); //7th to 14th bit
  for (int i=6;i<14;i++) { //Print those bits
    fprintf(stream, "%d", x[i]);
    if (x[i] == 1){ //Converting to decimal
      int power = pow(2, 13-i);
      sfn_deci += power;
    }
  }
  //Still need to add offset but technically the program prints the sfn so this is kind of just noting down how it gets the sfn?
  //Explained very nicely here https://dsp.stackexchange.com/questions/38256/system-frame-number-in-lte-specifications
  fprintf(stream, "\nConverted sfn: %d\n\n", sfn_deci*4); 
}*/

void print_IMSI(FILE *pcap, FILE *output, uint8_t *x, const uint32_t len){
  char header[] = "0000 01 01 01 02 ff fe 03 00 00 04 00 00 07 01 01"; //Paging request header
  char payload[len*2+1];
  bool is_imsi;

  FILE *p;
  p = fopen("payload.txt", "w");

  fprintf(pcap, "\n%s", header);
  for (int i=0; i<len; i++){
    fprintf(pcap, " %02x", x[i]);
    fprintf(p, "%02x", x[i]);
  }
  fclose(p);
  p = fopen("payload.txt", "r");
  fgets(payload, len*2+1, p);
  fclose(p);

  for (int i=2; i<len*2; i++){
    if (payload[i] == '9' && payload[i+1] == '5' && !(payload[i+2]=='0' && payload[i+3]=='0') && !(payload[i+4]=='0' && payload[i+5]=='0') && !(payload[i+6]=='0' && payload[i+7]=='0' && payload[i+8]=='0' && payload[i+9]=='0') && (payload[i+15] == '8' || payload[i+16] == '8')){
      is_imsi = true;
      for (int k=i+1; k<=i+15; k++){
        if (isdigit(payload[k])==0){
          is_imsi = false;
        }
      }
    }
    if (is_imsi){
      if (is_imsi){ //Stuff to put in the csv
      fprintf(output, "%s;%s\n", gettime(), payload);
      }
      break;
    }
  }
}


void print_SIB(FILE *pcap, char *type, uint8_t *x, const uint32_t len) {
  //SIB1
  char *header = "0000 01 01 04 02 ff ff 03 00 00 04 09 05 07 01 01";
  if (strncmp(type, "SIB2", 5)==0){ //WIP
    header = "0000 01 01 04 02 ff ff 03 00 00 04 0a 12 07 01 01 00 00";
  }
  fprintf(pcap, "%s ", header);
  for (int i=0; i<len; i++){
    fprintf(pcap, "%02x ", x[i]);
  }
  fprintf(pcap, "\n");
}

void print_normal(FILE *pcap, uint8_t *x, const uint32_t len){
  int i;
  char *header = "0000 01 01 04 02 ff ff 03 00 00 04 09 05 07 01 01"; //Placeholder header. This is for SIB1.
  fprintf(pcap, "%s ", header);
  for (i=0;i<len;i++) {
    fprintf(pcap, "%02x ", x[i]);
  }
  fprintf(pcap, "\n");
}


/* Functions that can be used in the program */
//Empties the file
void empty_file(char *filename) {
  FILE *f;
  f = fopen(filename, "w");
  //fprintf(f, "Program started on: %s", get_time());
  fclose(f);
}

//Save captured byte data from mib to sniffing_data.txt
void save_bytes(char *pcap, char *output, char *type, uint8_t *x, const uint32_t len) {
  FILE *f; 
  FILE *fp;
  f = fopen(pcap, "a");
  fp = fopen(output, "a");

  if (f) {
    if (strncmp(type, "IMSI", 5)==0){ //IMSI catcher
      print_IMSI(f, fp, x, len);
    } else if (strncmp(type, "SIB", 3)==0){
      print_SIB(f, type, x, len);
    }
    fclose(f);
    fclose(fp);
    } else {
      perror("fopen");
    }  
}

//Save some extra stuff for comparison I guess
void save_ref(char *filename, char *ref, int refnum){
  FILE *f;
  f = fopen(filename, "a");
  if (f){
    static char ref_repeat[100];
    char output_ref[100];
    sprintf(output_ref, "%s: %d\n", ref, refnum);
    if (strncmp(output_ref, ref_repeat, strlen(output_ref))){
      fprintf(f, "%s", ref_repeat); 
      sprintf(ref_repeat, "%s", output_ref);
    }
    fclose(f);
  } else{
    perror("fopen");
  }
}

//Append a string to the file
void append_to_file(char *filename, char *string){
  FILE *f;
  f = fopen(filename, "a");
  if (f){
    fprintf(f, "%s\n", string);
    fclose(f);
  } else{
    perror("fopen");
  }
}
