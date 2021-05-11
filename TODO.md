* [ ] rx_packet: re-enter recv mode

---
1. frequency programmieren
2. preamble auf maximale groesse setzen
   - sender sendet
   - empfaenger hat waehrend preamble zeit frequenz zu finden
3. symbol rate evtl runterstellen, da sonst nicht schnell genug
4. brute force frequenzen
   - nach frequenzy kalibierung -> reg sichern, damit nicht jedes mal neu kalibriert werden muss
5. finde sender -> messe rssi -> wenn rssi empfangen -> rest automatisch

sender:
- preamble zeit: max
- symbol rate: min

empfaenger:
"""
for each frequency:
  set frequency
  if first frequency:
    save kalibration
  else:
    write kalibration
  check rssi
  if rssi high:
    recv packet
"""
