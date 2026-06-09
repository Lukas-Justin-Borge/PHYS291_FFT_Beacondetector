# PHYS291_FFT_Beacondetector

**Topic:**
Beacon detection in network traffic using Fourier analysis. We simulate network
traffic where compromised machines send periodic signals (beacons) to a C2 server,
hidden in random background traffic. FFT in ROOT reveals these hidden periodicities
as spikes in the frequency spectrum — a technique used in real SOC environments.

**Methods:**
Fast Fourier Transform (TVirtualFFT), histograms (TH1D), curve fitting (TF1) for
threshold-based detection, statistical analysis, data visualization (TCanvas,
TGraph, TMultiGraph). Optional: interactive GUI with sliders (TGMainFrame, TGHSlider).

**Additional information:**
Group project (4 members). We generate synthetic traffic data in C++ with tunable
parameters (beacon interval, jitter, noise level, multiple beacons). The project
demonstrates how physics signal processing methods apply directly to cybersecurity
threat detection. All code in C++/ROOT.




**Student(Sage August Brandtzæg):** Ansvarlig for Datagenerering og Wireshark-integrasjon (Modul A)Fokus: Utvikling av Python-skriptet for beacon-simulering, oppsett av støy-scenarier i Wireshark, tshark-eksport og vasking av rådata til tidsstempler.

**Student(August):** Ansvarlig for ROOT Signalbehandling og FFT-implementering (Modul B)Fokus: C++/ROOT-arkitekturen, oppsett av tids-histogrammer (TH1D), konfigurering av TVirtualFFT, og matematisk transformasjon av dataene over i frekvensdomenet.

**Student(Lukas):** Ansvarlig for Statistisk Analyse og Terskeldeteksjon (Modul C)Fokus: Implementering av trusseldeteksjon basert på statistisk signifikans (Vi tenker 5sigma terskel), fiting av bakgrunnsstøy (TF1), og matematisk beregning av alarmbetingelser.

**Student(William):** Ansvarlig for Systemintegrasjon, Visualisering og GUI (Modul D)Fokus: Design av det endelige kontrollpanelet (TCanvas med tids- og frekvensdomene synkronisert), teknisk dokumentasjon av kode-grensesnittene, og eventuell implementering av interaktive kontrollskivere (sliders).


Når vi oppdager en beacon, ser vi at FFT-en også slår ut på de harmoniske frekvensene (multipler av grunnfrekvensen). Vi vurderte å filtrere ut disse i koden for å få en renere utskrift. Vi valgte imidlertid å ikke gjøre det, fordi det skaper en sårbarhet: En angriper kunne da ha gjemt en 2 Hz beacon i skyggen av en 1 Hz beacon, og detektoren vår ville ignorert den.