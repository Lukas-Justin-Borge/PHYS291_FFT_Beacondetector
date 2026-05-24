#include <iostream>
#include <vector>
#include "TFile.h"
#include "TH1D.h"
#include "TF1.h"
#include "TCanvas.h"

// --- LUKAS' CORE MATH FUNCTION ---
void DetectBeacons(TH1D *hFreq)
{
    if (!hFreq)
        return;

    int nBins = hFreq->GetNbinsX();

    // 1. Fit background (ignore bin 1/DC component)
    double xMin = hFreq->GetBinLowEdge(2);
    double xMax = hFreq->GetBinLowEdge(nBins + 1);

    TF1 *bgFit = new TF1("bgFit", "pol0", xMin, xMax);
    hFreq->Fit(bgFit, "Q 0 R");
    double mean = bgFit->GetParameter(0);

    // 2. Calculate 5-Sigma Threshold
    double sumSq = 0;
    for (int i = 2; i <= nBins; i++)
    {
        sumSq += pow(hFreq->GetBinContent(i) - mean, 2);
    }
    double sigma = sqrt(sumSq / (nBins - 1));
    double threshold = mean + (5.0 * sigma);

    // 3. Detect Alarms
    std::cout << "\n--- LUKAS' THREAT DETECTION REPORT ---" << std::endl;
    std::cout << "Threshold: " << threshold << " (5-Sigma)" << std::endl;

    for (int i = 2; i <= nBins; i++)
    {
        if (hFreq->GetBinContent(i) > threshold)
        {
            std::cout << "[!] BEACON DETECTED at " << hFreq->GetBinCenter(i) << " Hz" << std::endl;
        }
    }

    // 4. Visuals for William (Modul D)
    TF1 *tLine = new TF1("tLine", "[0]", xMin, xMax);
    tLine->SetParameter(0, threshold);
    tLine->SetLineColor(kRed);
    hFreq->GetListOfFunctions()->Add(tLine);
}

// --- MAIN EXECUTION PART ---
void lukas()
{
    // 1. Open August's output file
    TFile *file = TFile::Open("august.root");
    if (!file || file->IsZombie())
    {
        std::cout << "Error: Could not open august.root. Run August's script first!" << std::endl;
        return;
    }

    // 2. Get the Frequency Spectrum histogram from the file
    TH1D *hFreq = (TH1D *)file->Get("hFreq");
    if (!hFreq)
    {
        std::cout << "Error: Could not find histogram 'hFreq' in the file!" << std::endl;
        return;
    }

    // 3. Run the analysis
    DetectBeacons(hFreq);

    // 4. Draw result
    TCanvas *cLukas = new TCanvas("cLukas", "Lukas' Detection Result", 800, 600);
    hFreq->SetStats(0);
    hFreq->Draw("HIST"); // Red threshold line will draw automatically
}