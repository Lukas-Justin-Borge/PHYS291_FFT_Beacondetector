#include <iostream>
#include <vector>
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"
#include "TCanvas.h"

using namespace std;

void detect_beacons(TH1D *h)
{
    if (h == NULL) {
        cout << "no histogram found" << endl;
        return;
    }

    int bins = h->GetNbinsX();

    // Definerer rekkevidden. Ignorerer bin 1 (0 Hz / DC-komponenten) 
    // fordi den ekstreme toppen ødelegger skaleringen.
    double start = h->GetBinLowEdge(2);
    double end = h->GetBinLowEdge(bins + 1);

    // TILPASNING AV STØY:
    // Legger en flat linje ("pol0") over dataene for å finne 
    // gjennomsnittlig bakgrunnsstøy ('m').
    TF1 *fit = new TF1("fit", "pol0", start, end);
    h->Fit(fit, "Q 0 R");
    double m = fit->GetParameter(0); 

    /*
     * BEREGNING AV 5-SIGMA:
     * Terskelen for et signal er: gjennomsnitt + (5 * standardavvik).
     * Standardavviket ('sig') finnes ved å summere kvadrert avvik fra snittet,
     * og dele på (bins - 1) for et godt estimat.
     */
    double sum = 0;
    for (int i = 2; i <= bins; i++) {
        double val = h->GetBinContent(i);
        sum = sum + ((val - m) * (val - m)); 
    }
    
    // Finner standardavviket og definerer terskelverdien 
    double sig = sqrt(sum / (bins - 1));
    double th = m + (5.0 * sig);

    cout << "\n--- Lukas sin trusseldeteksjon ---" << endl;
    cout << "Grensesnitt: " << th << " (5-Sigma)" << endl;

    // SIGNALDETEKSJON:
    // Alt som bryter 5-sigma-terskelen er nesten garantert et ekte signal, ikke støy.
    for (int i = 2; i <= bins; i++) {
        double c = h->GetBinContent(i);
        if (c > th) {
            cout << "[!] BEACON DETECTED at " << h->GetBinCenter(i) << " Hz" << endl;
        }
    }

    // Tegner inn terskellinjen i rødt for William (Modul D)
    TF1 *line = new TF1("line", "[0]", start, end);
    line->SetParameter(0, th);
    line->SetLineColor(2); 
    h->GetListOfFunctions()->Add(line);
}


void lukas()
{
    // Åpner fil og henter histogrammet fra August
    TFile *f = new TFile("august.root");
    TH1D *h = (TH1D*)f->Get("hFreq");
    
    if(!h) {
        cout << "Error: Run Augusts script first!!" << endl;
        return;
    }

    // Kjører analysen
    detect_beacons(h);

    // Tegner grafen (uten den stygge stat-boksen)
    TCanvas *c1 = new TCanvas("c1", "Lukas' Detection Result", 800, 600);
    h->SetStats(0); 
    h->Draw("HIST"); 
}