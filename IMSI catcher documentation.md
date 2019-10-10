#Documentation for IMSI catcher
------
##Contents
- [What it does in a summary](#Usage)
- [Files](#Files) - What was modified and what was added

---
### What it does in a summary
Catches IMSI (The unique identifiers for SIM cards) using **pdsch_ue** and parses paging requests into _imsi.pcap_ while IMSI's are filtered into _imsi.csv_

### Running The Catcher
Besides srsLTE, make sure the package **text2pcap** is installed.

```bash
cd /home/pi/srsLTE/build/lib/examples
bash loop_catcher.sh
```
___
##Files
<table>
	<thead>
		<tr class="header">
			<th>New Files</th>
			<th>Modified Files</th>
		</th>
	</thead>
	<tbody>
		<tr>
			<td>loop_catcher.sh</td>
			<td>pdsch_ue.c</td>
		</tr>
		<tr>
			<td>parse_data.c</td>
			<td>cell_measurement.c</td>
		</tr>
		<tr>
			<td>convert_to_csv.c</td><td></td>
		</tr>
		<tr>
			<td>payload.txt</td><td></td>
		</tr>
		<tr>
			<td>imsi.pcap</td><td></td>
		</tr>
		<tr>
			<td>imsi.txt</td><td></td>
		</tr>
		<tr>
			<td>imsi.csv</td><td></td>
		</tr>
	</tbody>
</table>
---
##What the files do

####loop_catcher.sh -- This is script that runs all the other files.

It first compiles **srsLTE** and **convert_to_csv.c**  

Then enters a loop that runs _pdsch_ue_, changing between frequencies _1845000000 mhz_ and _1815000000 mhz_ each time.  

Each time the process ends, it runs **convert_to_csv.c** to get _imsi.csv_ as well as running **text2pcap** to get _imsi.pcap_  

Maybe the frequency of that should be changed as when there are a lot of payloads collected, the conversion to pcap can take awhile, delaying the next time **pdsch_ue** can be run

---
###pdsch_ue -- From srsLTE, this is the IMSI catcher
Location:  
- /home/pi/srsLTE/build/lib/examples/pdsch_ue -- executable  
- /home/pi/srsLTE/lib/examples/pdsch_ue.c -- source file

The IMSI catcher. functions that saves captured payloads are in **parse_data.c**

Syntax to run it
```bash
./pdsch_ue -f [frequency in hz] -r [rnti]
```
Two known frequencies are _1845000000_ and _1815000000_  
For rnti, _0xfffe_ works.
> Not to sure why _0xfffe_ but it was mentioned here https://github.com/tysonv7/CS3235-Project-/blob/master/GUTI%20Capture/README2.txt

#####When the program runs
>Change these variables if you want to change where to save paging requests to. Although **convert_to_c** reads _imsi.txt_ and **loop_catcher.sh** reads imsi_pcap.txt so you need to change those as well.
```C
char *parse_file = "imsi.txt";
char *pcap_data = "imsi_pcap.txt";                 
```

1. Searches for a cell. If after multiple tries and a cell is still not found, try double checking the frequency.
> To get a frequency if you don't know it:  
Samsung:  Dial \*#0011#  
Iphone:  Dial \*3001#12345#\* and press the call button  
  Convert the EARFCN to a frequency using http://niviuk.free.fr/lte_band.php  
2. Once cell is found, it enters the main loop attempts to decode the MIB (around line 670). 
>`save_bytes` is a function from **parse_data.c** that is trying to break up how to decode the MIB which is in binary. That function is also commented out inside **parse_data.c**

```C
//line 670
case DECODE_MIB{
//Stuff
	n = srslte_ue_mib_decode(&ue_mib, bch_payload, NULL, &sfn_offset);
    if (n < 0) {
    	fprintf(stderr, "Error decoding UE MIB\n");
    	exit(-1);
	} else if (n == SRSLTE_UE_MIB_FOUND) {             
	srslte_pbch_mib_unpack(bch_payload, &cell, &sfn);
    srslte_cell_fprint(stdout, &cell, sfn);
    printf("Decoded MIB. SFN: %d, offset: %d\n", sfn, sfn_offset);
    //save_bytes(pcap_data, parse_file, "MIB", bch_payload, 24);
    sfn = (sfn + sfn_offset)%1024; 
    state = DECODE_PDSCH;               
	}
}
break;
```
3. Once MIB is succesfully decoded, it then decodes PDSCH which is where the paging requests are captured.  
Once the the bit of code below runs, data[0] will contain the paging requests. 
>If this is the first time the UE is connecting to the network, the IMSI will be sent in the clear. After that, only s-TMSI will be sent on DL.

```C
//When decoding PDSCH, line 705
if (cell.nof_ports == 1) {
	/* Transmission mode 1 (Single Transmission Antenna and Single Reciever Antenna)*/ 
    n = srslte_ue_dl_decode(&ue_dl, data, 0, sfn*10+srslte_ue_sync_get_sfidx(&ue_sync), acks); 

//Some stuff...

//line 750, data[0] is written written to payload.txt, imsi_pcap.txt and imsi.txt if it's an IMSI
save_bytes(pcap_data, parse_file, "IMSI", data[0], n/4);
 
```
Side note:
`n = srslte_ue_mib_decode(&ue_mib, bch_payload, NULL, &sfn_offset);` and `n = srslte_ue_dl_decode(&ue_dl, data, 0, sfn*10+srslte_ue_sync_get_sfidx(&ue_sync), acks);` is really important because those functions will return data[0]  

---
### parse-data.c -- all the functions that captures payloads while pdsch_ue is running
Functions that are actually being called in the program. Refer to actual code to read the comments.
```C
//All fo this is after line 110

//Not used in the main program. Mainly for debugging when I just want to do a quick capture and don't want to scroll through.
empty_file(char *filename)

//The very useful function that saves the captured bytes and outputs io imsi.txt
save_bytes(char *pcap, char *output, char *type, uint8_t *x, const uint32_t len)

//Used just there to save any other numbers from the main program to the file for reference. Like comparing the MIB sfn decoded using print_MIB() and see if actually matches up to the one decoded by the program already. 
save_bytes(char *pcap, char *output, char *type, uint8_t *x, const uint32_t len) {


//Again, for debuggin. Just append a string to a file.
append_to_file(char *filename, char *string){
```

Although **save_bytes** should be elaborated on since it's doing all the work. Specifically **print_IMSI**.  

```C
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
```

---
###convert_to_csv.c -- ~~somewhat~~ parses captured payloads from _imsi.txt_ to _imsi.csv

---
### cell_measurement -- TODO
SIB1 and SIB2 blocks can be captured here. SIB1 is captured but still working out where SIB2 is. Theoretically it should be in the same payload as SIB1 since when it was put through wireshark, there were a lot of extra bytes. Need to confirm though.

### Files that are not code.
- **payload.txt** -- stores the most recent payload captured. Read by **pdsch_ue** to store payload to other files.
- **imsi.pcap** -- all the payloads with the paging header added in front for it to be formatted into a wireshark with
`text2pcap input_file output_file -l 147`
- **imsi.txt** -- filtered IMSI's captured are appended here to read by **convert_to_csv**. Can be seen updated live if opened in sublime text.
-- **imsi.csv** -- The output file. Filtered out IMSI's are broken down into country code(MCC), operator(MNC), and identity number assigned by operator(MSIN). If S-TMSI is captured in the same frame, it is included as well.

------
##Why the information could be useful

##Stuff that could be explored ~~AKA unfinished~~

#References
