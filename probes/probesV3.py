from scapy.all import *

class UniqueProbeTracker:
   def __init__(self, filename="probes_capture.txt"):
       self.filename = filename
       self.unique_probes = set()
       self.excluded_prefixes = ("Livebox-", "Bbox-", "Freebox-") #exclusion a amélioré

   def capture_probes(self, pkt):
       if pkt.haslayer(Dot11ProbeReq):
           probe_ssid = pkt.info.decode('utf-8', errors='ignore')
           
           # chek la taille 8+
           if (probe_ssid and 
               len(probe_ssid) >= 8 and 
               not probe_ssid.startswith(self.excluded_prefixes) and
               probe_ssid not in self.unique_probes):
               
               self.unique_probes.add(probe_ssid)
               with open(self.filename, "a") as f:
                   f.write(f"{probe_ssid}\n")

# Initialisation
tracker = UniqueProbeTracker()

# Mode monitor
os.system('airmon-ng start wlan0')

# Capture
try:
   sniff(iface="wlan0mon", prn=tracker.capture_probes, store=0)
except KeyboardInterrupt:
   print("\nCapture terminée")
