counter=0
frequencies=(1845000000 1815000000)
cd /home/pi/srsLTE/build
make -j 4
cd lib/examples
gcc convert_to_csv.c -o convert_to_csv

function kill_pdsch_ue(){
  local timer=4m
  sleep ${timer}
  pkill pdsch_ue
}

while true; do
  ((counter++))
  kill_pdsch_ue &
  ./pdsch_ue -f ${frequencies[${counter}%2]} -r 0xfffe
  text2pcap imsi_pcap.txt imsi.pcap -l 147
  ./convert_to_csv
done
