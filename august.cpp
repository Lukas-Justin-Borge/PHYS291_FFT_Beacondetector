// moduleB_fft.C
// August — Module B: Time histogram + Pure C++ DFT (Med DC-offset korreksjon)
// Run with: root -l august.cpp

#include "TH1D.h"
#include "TCanvas.h"
#include "TFile.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>

void august()
{
    // ── 1. PARAMETERS (Justert til Sages 3-minutters kjøring) ────────────────
    const double T_MAX = 180.0; // total observation window in seconds (3 min)
    const int N_BINS = 1024;    // Beholdes som potens av 2 for histogrammet
    const double BIN_SIZE = T_MAX / N_BINS;

    // ── 2. TIME-DOMAIN HISTOGRAM ─────────────────────────────────────────────
    TH1D *hTime = new TH1D("hTime", "Network Traffic (time domain);Time [s];Packet count",
                           N_BINS, 0, T_MAX);

    // ── 3. LOAD DATA (Leser de tre kolonnene fra Tshark) ─────────────────────
    // SATT TIL SCENARIO 3 FOR Å TESTE DETEKSJON I STØY:
    std::ifstream fin("scenario3_mixed_beacon.txt");
    if (!fin) {
        std::cout << "Error: Kunne ikke åpne scenario3_mixed_beacon.txt!\n";
        return;
    }

    double t;
    std::string src_ip, dst_ip;

    while (fin >> t >> src_ip >> dst_ip)
    {
        if (t <= T_MAX) {
            hTime->Fill(t);
        }
    }
    fin.close();
    std::cout << "[Module B] Suksess! Lastet inn ekte data fra Sages nettverksfangst.\n";

    // ── 3.5 BEREGN DC-OFFSET (Gjennomsnittsstøy per bin) ────────────────────
    double total_packets = 0;
    for (int n = 1; n <= N_BINS; n++) {
        total_packets += hTime->GetBinContent(n);
    }
    double mean_packets = total_packets / N_BINS;
    std::cout << "[Module B] Gjennomsnittlig bakgrunnsstøy fjernet: " << mean_packets << " pakker/bin.\n";

    // ── 4 & 5. PURE C++ FOURIER TRANSFORMATION & POWER SPECTRUM ──────────────
    double df = 1.0 / T_MAX;
    int nFreqBins = N_BINS / 2 + 1; // Kun positive frekvenser trengs

    TH1D *hFreq = new TH1D("hFreq", "Power Spectrum (freq domain);Frequency [Hz];Power",
                           nFreqBins, 0, nFreqBins * df);

    std::cout << "[Module B] Kjører ren C++ Fourier-transformasjon med støyfilter..." << std::endl;

    // Loop over alle frekvens-bins vi ønsker å beregne
    for (int k = 0; k < nFreqBins; k++)
    {
        double freq = k * df;
        double real_sum = 0.0;
        double imag_sum = 0.0;

        // Loop over alle tids-bins i histogrammet
        for (int n = 1; n <= N_BINS; n++)
        {
            // Vi trekker fra gjennomsnittet (mean_packets) for å sentrere signalet rundt 0
            double signal = hTime->GetBinContent(n) - mean_packets;
            double time_val = hTime->GetXaxis()->GetBinCenter(n);

            double angle = 2.0 * M_PI * freq * time_val;
            real_sum += signal * std::cos(angle);
            imag_sum -= signal * std::sin(angle);
        }

        // Beregn Power (Amplituden i kvadrat)
        double power = (real_sum * real_sum) + (imag_sum * imag_sum);
        hFreq->SetBinContent(k + 1, power);
    }
    std::cout << "[Module B] Fourier-transformasjon fullført feilfritt!\n";

    // ── 6. SAVE OUTPUT for Module C (Lukas) ──────────────────────────────────
    TFile *outFile = new TFile("august.root", "RECREATE");
    hTime->Write();
    hFreq->Write();
    outFile->Close();
    std::cout << "[Module B] Wrote hTime and hFreq to august.root\n";

    // ── 7. QUICK VISUAL CHECK ─────────────────────────────────────────────────
    TCanvas *c = new TCanvas("c", "Module B Preview", 1200, 500);
    c->Divide(2, 1);
    c->cd(1);
    hTime->Draw();
    c->cd(2);
    hFreq->Draw();
    c->SaveAs("august_preview.png");
}
