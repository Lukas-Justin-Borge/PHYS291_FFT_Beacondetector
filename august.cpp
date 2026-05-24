// moduleB_fft.C
// August — Module B: Time histogram + FFT
// Run with: root -l moduleB_fft.C

#include "TH1D.h"
#include "TVirtualFFT.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TRandom3.h"
#include <vector>
#include <fstream>
#include <iostream>

void august()
{

    // ── 1. PARAMETERS (agree these with the group) ──────────────────────────
    const double T_MAX = 600.0; // total observation window in seconds
    const int N_BINS = 1024;    // must be power of 2 for FFT efficiency
    const double BIN_SIZE = T_MAX / N_BINS;

    // ── 2. TIME-DOMAIN HISTOGRAM ─────────────────────────────────────────────
    TH1D *hTime = new TH1D("hTime", "Network Traffic (time domain);Time [s];Packet count",
                           N_BINS, 0, T_MAX);

    // ── 3. LOAD DATA (from Sage's output, or synthetic for now) ─────────────
    // Option A: read from file (Sage's timestamps)
    /*
    std::ifstream fin("timestamps.txt");
    double t;
    while (fin >> t) hTime->Fill(t);
    fin.close();
    */

    // Option B: synthetic beacon + noise (use this until Sage is ready)
    TRandom3 rng(42);
    double beacon_interval = 30.0; // seconds — tunable
    double jitter = 1.5;           // ± seconds of timing noise
    int noise_packets = 2000;      // background noise

    for (int i = 0; i < noise_packets; i++)
        hTime->Fill(rng.Uniform(0, T_MAX));

    for (double t = beacon_interval; t < T_MAX; t += beacon_interval)
        hTime->Fill(t + rng.Gaus(0, jitter));

    // ── 4. FFT ───────────────────────────────────────────────────────────────
    int n = hTime->GetNbinsX();
    TVirtualFFT *fft = TVirtualFFT::FFT(1, &n, "R2C ES"); // Real-to-Complex

    // Copy histogram bin contents into FFT input
    for (int i = 1; i <= n; i++)
        fft->SetPoint(i - 1, hTime->GetBinContent(i));

    fft->Transform();

    // ── 5. POWER SPECTRUM ────────────────────────────────────────────────────
    // Frequency resolution: df = 1/T_MAX
    // Max frequency (Nyquist): 1/(2*BIN_SIZE)
    double df = 1.0 / T_MAX;
    int nFreqBins = n / 2 + 1; // only positive frequencies needed

    TH1D *hFreq = new TH1D("hFreq", "Power Spectrum (freq domain);Frequency [Hz];Power",
                           nFreqBins, 0, nFreqBins * df);

    for (int i = 0; i < nFreqBins; i++)
    {
        double re, im;
        fft->GetPointComplex(i, re, im);
        double power = re * re + im * im;
        hFreq->SetBinContent(i + 1, power);
    }

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

    delete fft;
}