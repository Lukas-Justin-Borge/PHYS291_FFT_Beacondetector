// moduleC_detector.C
// Lukas — Module C: 5-Sigma Threat Detection with 0.4 Hz Noise Gating
// Run with: root -l lukas.cpp

#include "TH1D.h"
#include "TFile.h"
#include <iostream>
#include <cmath>

void lukas()
{
    // 1. Åpne Sages nylig genererte ROOT-fil
    TFile *inFile = TFile::Open("august.root");
    if (!inFile || inFile->IsZombie()) {
        std::cout << "Error: Kunne ikke åpne august.root! Kjør august.cpp først.\n";
        return;
    }

    TH1D *hFreq = (TH1D*)inFile->Get("hFreq");
    if (!hFreq) {
        std::cout << "Error: Fant ikke histogrammet hFreq i filen.\n";
        return;
    }

    int nBins = hFreq->GetNbinsX();

    // 2. EKTE DYNAMISK 5-SIGMA BEREGNING
    double sum = 0.0;
    double sum_sq = 0.0;
    int count = 0;

    // Vi ignorerer alt under 0.05 Hz for å fjerne den massive surfestøyveggen fra snittet
    for (int i = 1; i <= nBins; i++) {
        double freq = hFreq->GetBinCenter(i);
        if (freq >= 0.05) { 
            double val = hFreq->GetBinContent(i);
            sum += val;
            sum_sq += val * val;
            count++;
        }
    }

    double mean = sum / count;
    double variance = (sum_sq / count) - (mean * mean);
    double sigma = std::sqrt(variance);

    // Sett terskelen dynamisk til 5-Sigma over det nye, rene gjennomsnittet
    double dynamic_threshold = mean + (5.0 * sigma);

    std::cout << "\n--- LUKAS' THREAT DETECTION REPORT ---" << std::endl;
    std::cout << "Dynamisk Terskel: " << dynamic_threshold << " (5-Sigma basert på støy)" << std::endl;

    // 3. DETEKSJONSLOOP
    bool detected = false;
    for (int i = 1; i <= nBins; i++) {
        double freq = hFreq->GetBinCenter(i);
        double power = hFreq->GetBinContent(i);

        // Skjær bort alt av lavfrekvent surfestøy (< 0.4 Hz)
        if (freq < 0.4) continue;

        // Sjekk om signalet skyter over 5-Sigma-veggen
        if (power > dynamic_threshold) {
            std::cout << "[!] BEACON DETECTED at " << freq << " Hz (Power: " << power << ")" << std::endl;
            detected = true;
        }
    }

    if (!detected) {
        std::cout << "[*] Ingen mistenkelig beacon-trafikk detektert over støygrensen." << std::endl;
    }
    std::cout << "--------------------------------------\n" << std::endl;

    inFile->Close();
}
