_# Documentation for IMSI catcher
------
## Contents
- [What it does](#what-it-does-in-a-summary)
- [Captured stuff](#what-it-could-capture)
- [Wireshark](#reading-wireshark-files)
- [Files](#files)
- [What the Files do](#what-the-files-do)
  - [loop_catcher.sh](#loop_catchersh)
  - [pdsch_ue](#pdsch_ue)
  - [parse_data.c](#parse_datac)
  - [convert_to_csv.c](#convert_to_csvc)
  - [cell_measurement](#cell_measurement)
  - [Others](#others)
- [Future Work](#future-work)
- [References](#references)


---
### What it does in a summary
Catches IMSI (The unique identifiers for SIM cards) using **pdsch_ue** and parses paging requests into _imsi.pcap_ while IMSI's are filtered into _imsi.csv_

### Running The Catcher
Besides srsLTE, make sure the package **text2pcap** is installed.

```bash
cd /home/pi/srsLTE/build/lib/examples
bash loop_catcher.sh
```

If **pdsch_ue** succesfully found a cell, it should show stuff in the screenshot below.
![](Screenshots/run.png "Terminal fills up very quickly once the main loop is in the stage of decoding PDSCH")

If it is catching the signal properly, there should be a nice constellation.
![](Screenshots/constellation1.png)
![](Screenshots/constellation2.png)

- PDSCH - Equalised Symbols  
  Should not be too crazy. The 4 dots should be very clear. Doesn't show if something is wrong with the signal.
- PDCCH - Equalised Symbols  
  The main 5 dots should be distinguishable at least. Gets fuzzy sometimes. If there is nothing/completely scattered (sometimes concentrated in the centre) restart the program.
- Channel Response Magnitude  
  As long as it doesn't look like a seismograph during a massive earthquake, it's cool.
- PSS Cross-Corr abs value  
  Curve should be very distinguishable. The first to freak out and show very intense activity if the signal messes up.

---
## What it could capture
- Master Information Block  
  Very important to if you want to decode everything else but does not really hold information besides that.
- System Information Block 
  - SIB1
    - PLMN ID  
      - Country Code (MCC)
      - Operator (MNC)
  - SIB2 -- TODO
    - Some system configuration settings
      Theoroetically could make a rouge eNB using the settings from SIB2
      > recieved powerthreshold  to  trigger  a  handoff  to  an  adjacent  cell  and  aseries  of  configuration  parameters  that  could  be  leveraged  toconfigure a rogue base station. [3]
- Paging requests
  - IMSI (Not more than 15 digits)
    - MCC (3)
    - MNC (2-3)
    - MSIN (9-10)
  - S-TMSI (40 bits)
    - MMEC
    - M-TMSI

---

## Reading Wireshark files
Captured stuff is in _imsi.txt_

#### Useful filters
- Packets with IMSI in it - `lte-rrc.ue_Identity == 1`  
- Only s-TMSI - `lte-rrc.ue_Identity == 0`
- Strange packets that contain s-tmsi with IMSI - `lte-rrc.ue_Identity == 1 && frame.len > 45`

#### Reading the hex
**pdsch_ue** will dump out raw hex bytes. To read it in a wireshark format, **text2pcap** is used. 

A header needs to be fixed before the hex dump for it to be read.  

Headers for different messages in _imsi_pcap.txt_ (Before it gets process by _text2pcap_)
- Paging request  
  `0000 01 01 01 02 ff fe 03 00 00 04 00 00 07 01 01`
- SIB1  
  `0000 01 01 04 02 ff ff 03 00 00 04 09 05 07 01 01`
- SIB2  
  `0000 01 01 04 02 ff ff 03 00 00 04 0a 12 07 01 01 00 00`

then the payload is attached afterwards

## Files
<table>
  <thead>
    <tr class="header">
      <th>Modified Files</th>
      <th>Location</th>
    </th>
  </thead>
  <tbody>
    <tr>
      <td>pdsch_ue.c</td>
      <td>/home/pi/srsLTE/lib/examples</td>
    </tr>
    <tr>
      <td>cell_measurement.c</td>
      <td>/home/pi/srsLTE/lib/examples</td>
    </tr>
  </tbody>
</tabl>
<table>
  <thead>
    <tr class="header">
      <th>New Files</th>
      <th>Location</th>
    </th>
  </thead>
  <tbody>
    <tr>
      <td>parse_data.c</td>
      <td>/home/pi/srsLTE/lib/include/srsLTE/</td>
    </tr>
    <tr>
      <td>convert_to_csv.c</td>
      <td>/home/pi/srsLTE/build/lib/examples</td>
    </tr>
    <tr>
      <td>payload.txt</td>
      <td>/home/pi/srsLTE/build/lib/examples</td>
    </tr>
    <tr>
      <td>imsi.pcap</td>
      <td>/home/pi/srsLTE/build/lib/examples</td>
    </tr>
    <tr>
      <td>imsi.txt</td>
      <td>/home/pi/srsLTE/build/lib/examples</td>
    </tr>
    <tr>
      <td>imsi.csv</td>
      <td>/home/pi/srsLTE/build/lib/examples</td>
    </tr>
  </tbody>
</table>

---
## What the files do

#### loop_catcher.sh
It first compiles **srsLTE** and **convert_to_csv.c**  

Then enters a loop that runs _pdsch_ue_, changing between frequencies _1845000000 mhz_ and _1815000000 mhz_ each time.  

Each time the process ends, it runs **convert_to_csv.c** to get _imsi.csv_ as well as running **text2pcap** to get _imsi.pcap_  

Maybe the frequency of that should be changed as when there are a lot of payloads collected, the conversion to pcap can take awhile, delaying the next time **pdsch_ue** can be run

---
#### pdsch_ue
From srsLTE, this is the IMSI catcher

>Location:
>- /home/pi/srsLTE/build/lib/examples/pdsch_ue -- executable  
>- /home/pi/srsLTE/lib/examples/pdsch_ue.c -- source file

The IMSI catcher. functions that saves captured payloads are in **parse_data.c**

Syntax to run it
```bash
./pdsch_ue -f [frequency in hz] -r [rnti]
```
Two known frequencies are _1845000000_ and _1815000000_  
For rnti, _0xfffe_ works.
> Not to sure why _0xfffe_ but it was mentioned [here](https://github.com/tysonv7/CS3235-Project-/blob/master/GUTI%20Capture/README2.txt)

##### When the pdsch_ue runs
>Change these variables if you want to change where to save paging requests to. Although **convert_to_c** reads _imsi.txt_ and **loop_catcher.sh** reads imsi_pcap.txt so you need to change those as well.
```C
char *parse_file = "imsi.txt";
char *pcap_data = "imsi_pcap.txt";                 
```

1. Searches for a cell. If after multiple tries and a cell is still not found, try double checking the frequency.
> To get a frequency if you don't know it:  

>- Samsung:  
  Dial \*#0011#  
>- Iphone:  
Dial \*3001#12345#\* and press the call button  

>  Convert the EARFCN to a frequency using http://niviuk.free.fr/lte_band.php  
2. Once cell is found, it enters the main loop and attempts to decode the MIB (around line 670). 
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

```C
//When decoding PDSCH, line 705
if (cell.nof_ports == 1) {
  /* Transmission mode 1 (Single Transmission Antenna and Single Reciever Antenna)*/ 
    n = srslte_ue_dl_decode(&ue_dl, data, 0, sfn*10+srslte_ue_sync_get_sfidx(&ue_sync), acks); 

/*Some stuff...*/

//line 750, data[0] is written written to payload.txt, imsi_pcap.txt and imsi.txt if it's an IMSI
save_bytes(pcap_data, parse_file, "IMSI", data[0], n/4);
 
```
Side note:
`n = srslte_ue_mib_decode(&ue_mib, bch_payload, NULL, &sfn_offset);` and `n = srslte_ue_dl_decode(&ue_dl, data, 0, sfn*10+srslte_ue_sync_get_sfidx(&ue_sync), acks);` is really important because those functions will return data[0]  

---
### parse_data.c 
All the functions that captures payloads while pdsch_ue is running  

>Location:
/home/pi/srsLTE/lib/include/srsLTE/parse-data.c

Functions that are actually being called in the program.
```C
//All fo this is after line 110

//Not used in the main program. Mainly for debugging when I just want to do a quick capture and don't want to scroll through.
empty_file(char *filename)

//The very useful function that saves the captured bytes and outputs io imsi.txt
save_bytes(char *pcap, char *output, char *type, uint8_t *x, const uint32_t len)

//Used just there to save any other numbers from the main program to the file for reference. Like comparing the MIB sfn decoded using print_MIB() and see if actually matches up to the one decoded by the program already. 
save_ref(char *filename, char *ref, int refnum){

//Again, for debugging. Just append a string to a file.
append_to_file(char *filename, char *string){
```

`save_bytes` should be elaborated on since it's doing all the work. Specifically `print_IMSI` which is called when save_bytes is used to read the paging requests from **pdsch_ue**. 

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
    //This is checking if an IMSI exists.
    if (payload[i] == '9' && payload[i+1] == '5' && !(payload[i+2]=='0' && payload[i+3]=='0') && !(payload[i+4]=='0' && payload[i+5]=='0') && !(payload[i+6]=='0' && payload[i+7]=='0' && payload[i+8]=='0' && payload[i+9]=='0') && (payload[i+15] == '8' || payload[i+16] == '8')){
      is_imsi = true;l
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
Payload is printed to payload.txt (so if it crashes you can easily see what was the last payload it crashed at.)  
Also there were some issues ~~most probably because my C is atrocious~~ storing the payload directly into a variable and printing it. Then printed into a text2pcap format in _imsi_pcap.txt_ and _imsi.txt_  

Not going too much into detail about how it looks for IMSI since it's more in-depth in **convert_to_csv**.  
Over here it's just checking if an IMSI exists. If it does, it will filter it out to _imsi.txt_.

---
### convert_to_csv.c
**Kind important!**
If there is an empty line at the beginning of the file, it will crash. Removing that should make it work.

~~somewhat~~ parses captured payloads from _imsi.txt_ to _imsi.csv
Runs after **pdsch_ue** and converts the filtered IMSI in _imsi.txt_ to a csv file.
> Can also be used to filter IMSI as long as it's in the right format. 

Checking if IMSI exists in the payload  
Criteria for IMSI:  

- In the payload, the IMSI appears like this
  > [9][IMSI Number][8]
- IMSI = MCC + MNC + MSIN  
  - MNC must be more than 0
  - MCC must be between 500 and 599  
  > [9][5][ ][ ][MNC][ ][8]
- Does not contain hex, hence whole string can only be made of digits.

```C
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
}
```
Once IMSI is found, based on the index just carve it out from there.

However there are occasional exceptions where S-TMSI is caught in the same frame as IMSI. 

Still needs a bit of fixing to find the S-TMSI if it appears after the IMSI. That part is commented out.

Checking for S-TMSI:

- S-TMSI = MMEC + MSIN (In hex)
  - MMEC, 2 char
  - MSIN, 8 char 
- How it can appear:  
  - [S-TMSI][IMSI]  
  - [IMSI][S-TMSI]
  - [S-TMSI][IMSI][S-TMSI]
  - [S-TMSI][0][S-TMSI][IMSI]
  - [IMSI][S-TMSI][0][S-TMSI]

```C
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
```


---
### cell_measurement
**TODO**
SIB1 and SIB2 blocks can be captured here. SIB1 is captured but still working out where SIB2 is.  
Theoretically it should be in the same payload as SIB1 since when it was put through wireshark, there were a lot of extra bytes. Need to confirm though.

Below is the part where SIB block is decoded

```C
//Line 333
case DECODE_SIB:
  /* We are looking for SI Blocks, search only in appropiate places */
  if ((srslte_ue_sync_get_sfidx(&ue_sync) == 5 && (sfn%2)==0)) {
    n = srslte_ue_dl_decode(&ue_dl, data, 0, sfn*10+srslte_ue_sync_get_sfidx(&ue_sync), acks);
    if (n < 0) {
      fprintf(stderr, "Error decoding UE DL\n");fflush(stdout);
      return -1;
    } else if (n == 0) {
      printf("CFO: %+6.4f kHz, SFO: %+6.4f kHz, PDCCH-Det: %.3f\r",
              srslte_ue_sync_get_cfo(&ue_sync)/1000, srslte_ue_sync_get_sfo(&ue_sync)/1000, 
              (float) ue_dl.nof_detected/nof_trials);
      nof_trials++; 
    } else {
      printf("Decoded SIB1. Payload: ");
      srslte_vec_fprint_byte(stdout, data[0], n/8);;
      save_bytes("database.txt", "sniffing_data.txt", "SIB1", data[0], n);
      break;
    }
  }
```

---
### Others
- **payload.txt** -- stores the most recent payload captured. Read by **pdsch_ue** to store payload to other files.
- **imsi.pcap** -- all the payloads with the paging header added in front for it to be formatted into a wireshark with
`text2pcap input_file output_file -l 147`
- **imsi.txt** -- filtered IMSI's captured are appended here to read by **convert_to_csv**. Can be seen updated live if opened in sublime text.
-- **imsi.csv** -- The output file. Filtered out IMSI's are broken down into country code(MCC), operator(MNC), and identity number assigned by operator(MSIN). If S-TMSI is captured in the same frame, it is included as well.

------
## Future work

- Fix **convert_to_csv.c**
  - If S-TMSIs are caught in the same frame, it doesn't carve it out if it appears after the IMSI
  -  When parsed into wireshark, the packets that contain only an imsi at the beginning of the block is cut off. Some bytes can be seen at the point it is cut off. Possibly S-TMSI? But it's a bit too long to be it though.
  > If it's true that the S-TMSI that was cut off is the one assigned to the IMSI, you could possibly follow it around the network. [Sniff and Capture](#useful-reading-material)

- Capture **SIB2** blocks in **cell_measurement.c** for the system configurations  
  > Could be possible to set up a rogue eNB and get UEs to connect to it instead. [3]


------
## References

#### Useful reading material
- 4G/LTE IMSI Catchers for Non-Programmers [1]  
https://arxiv.org/pdf/1702.04434.pdf
- LTE security, protocol exploitation and location tracking experimentation with low-cost software radio [2]  
https://arxiv.org/pdf/1607.05171.pdf  
http://rogerpiquerasjover.net/LTE_5G_security_VATech.pdf (The slides for it)
- Practical Attacks Agaisnt Privacy and Availability in 4G/LTE Mobile Communication Systems [3]  
https://arxiv.org/pdf/1510.07563.pdf
- Sniff and Capture  pg (57-61) [4]  
https://www.comp.nus.edu.sg/~hugh/CS3235/PREVIOUSPROJECTS/CS3235-SemII-2015-16-Projects.pdf


#### Quick references
- Convert EARFCN to frequency [5]  
http://niviuk.free.fr/lte_band.php
- Useful lte encyclopedia for the ludicrous amount for acronyms [6]    
https://sites.google.com/site/lteencyclopedia/lte-acronyms  
- Find frequency on Iphone or Android [7]  
https://www.stelladoradus.com/finding-my-frequency-on-my-iphone/

#### Others =D
- LTE sniffer (much older version of srsLTE) [8]  
https://github.com/tysonv7/CS3235-Project-   
- SRS Airscope Demonstration  
https://www.youtube.com/watch?v=v7vADOqatRc