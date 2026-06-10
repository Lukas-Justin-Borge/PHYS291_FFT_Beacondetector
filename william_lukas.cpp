// william.cpp
// William -- Modul D: Systemintegrasjon, visualisering og dashboard.
//
// Kjør fra denne mappen etter at August har laget august.root:
//   root -l august.cpp
//   root -l william.cpp
//
// Denne filen leser August sine hTime og hFreq histogrammer og bruker Lukas
// sin nåværende 5-sigma regel slik at alt kan vises samlet i ett canvas.

#include "TCanvas.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1D.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TLine.h"
#include "TList.h"
#include "TMath.h"
#include "TStyle.h"

#include <cmath>
#include <iostream>
#include <vector>

struct BeaconHit {
    double frequency;
    double power;
    double period;
};

struct DetectionResult {
    double threshold;
    double noiseGateHz;
    std::vector<BeaconHit> hits;
};

DetectionResult run_lukas_detection_rule(TH1D *hFreq, double noiseGateHz = 0.05)
{
    // Kjører samme deteksjonslogikk som Lukas: ignorer lavfrekvent støy
    // og beregn en dynamisk 5-sigma terskel fra resten av spekteret.
    DetectionResult result;
    result.threshold = -1.0;
    result.noiseGateHz = noiseGateHz;

    if (!hFreq) {
        return result;
    }

    double sum = 0.0;
    double sumSq = 0.0;
    int count = 0;

    for (int i = 1; i <= hFreq->GetNbinsX(); i++) {
        double frequency = hFreq->GetBinCenter(i);
        if (frequency <= noiseGateHz) {
            continue;
        }

        double power = hFreq->GetBinContent(i);
        sum += power;
        sumSq += power * power;
        count++;
    }

    if (count == 0) {
        return result;
    }

    // Beregner gjennomsnitt og standardavvik for frekvensområdet over noise gate.
    double mean = sum / count;
    double variance = (sumSq / count) - (mean * mean);
    double sigma = std::sqrt(variance);
    result.threshold = mean + (5.0 * sigma);

    // Samler alle frekvenstopper som bryter terskelen.
    for (int i = 1; i <= hFreq->GetNbinsX(); i++) {
        double frequency = hFreq->GetBinCenter(i);
        double power = hFreq->GetBinContent(i);

        if (frequency <= noiseGateHz || power <= result.threshold) {
            continue;
        }

        BeaconHit hit;
        hit.frequency = frequency;
        hit.power = power;
        hit.period = 1.0 / frequency;
        result.hits.push_back(hit);
    }

    return result;
}

void draw_status_text(const DetectionResult &detection)
{
    // Skriver korte statuslinjer direkte inn i frekvensplottet.
    TLatex text;
    text.SetNDC();
    text.SetTextSize(0.035);

    if (detection.hits.empty()) {
        text.SetTextColor(kGreen + 2);
        text.DrawLatex(0.13, 0.86, "No beacon above Lukas' gated 5-sigma threshold");
        text.SetTextColor(kBlack);
        text.DrawLatex(0.13, 0.80, Form("Noise gate: f >= %.1f Hz", detection.noiseGateHz));
        return;
    }

    text.SetTextColor(kRed + 1);
    text.DrawLatex(0.13, 0.86,
                   Form("%zu beacon candidate(s) above gated 5-sigma threshold",
                        detection.hits.size()));

    text.SetTextColor(kBlack);
    text.DrawLatex(0.13, 0.80, Form("Noise gate: f >= %.1f Hz", detection.noiseGateHz));

    int maxLines = detection.hits.size() < 3 ? detection.hits.size() : 3;
    for (int i = 0; i < maxLines; i++) {
        text.DrawLatex(0.13, 0.74 - 0.05 * i,
                       Form("f = %.4f Hz  ->  period = %.2f s",
                            detection.hits[i].frequency, detection.hits[i].period));
    }
}

void william_lukas(const char *rootFileName = "august.root")
{
    gStyle->SetOptStat(0);

    // Åpner ROOT-filen som August lager.
    TFile *inputFile = TFile::Open(rootFileName, "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cout << "[William] Error: Could not open " << rootFileName << "\n";
        std::cout << "[William] Run August's script first: root -l august.cpp\n";
        return;
    }

    TH1D *hTime = (TH1D *)inputFile->Get("hTime");
    TH1D *hFreq = (TH1D *)inputFile->Get("hFreq");

    // Sjekker at grensesnittet mot August fortsatt stemmer.
    if (!hTime || !hFreq) {
        std::cout << "[William] Error: " << rootFileName
                  << " must contain histograms named hTime and hFreq.\n";
        return;
    }

    // Holder histogrammene i live etter at ROOT-filen lukkes.
    hTime->SetDirectory(0);
    hFreq->SetDirectory(0);
    inputFile->Close();

    // Setter farger og titler for tidsdomenet.
    hTime->SetLineColor(kBlue + 1);
    hTime->SetFillColorAlpha(kAzure + 1, 0.25);
    hTime->SetTitle("Module D Dashboard: Network traffic over time;Time [s];Packet count");

    // Setter farger og titler for frekvensdomenet.
    hFreq->SetLineColor(kBlue + 2);
    hFreq->SetFillColorAlpha(kAzure + 2, 0.20);
    hFreq->SetTitle("Frequency spectrum with Lukas' gated detection threshold;Frequency [Hz];Power");
    hFreq->GetXaxis()->SetRangeUser(0.4, hFreq->GetXaxis()->GetXmax());

    // Kjører Lukas sin terskelmetode på histogrammet fra August.
    DetectionResult detection = run_lukas_detection_rule(hFreq);

    // Lager kontrollpanelet med tidsdomene øverst og frekvensdomene nederst.
    TCanvas *dashboard = new TCanvas("william_dashboard",
                                     "William Module D Dashboard",
                                     1300, 850);
    dashboard->Divide(1, 2);

    dashboard->cd(1);
    hTime->Draw("HIST");

    TLatex timeText;
    timeText.SetNDC();
    timeText.SetTextSize(0.035);
    timeText.DrawLatex(0.13, 0.86, "August output: hTime shows packet counts binned over time");

    // Tegner frekvensspekteret med litt ekstra plass over høyeste synlige topp.
    dashboard->cd(2);
    double visibleMax = hFreq->GetMaximum();
    if (visibleMax > 0.0) {
        hFreq->SetMaximum(visibleMax * 1.25);
    }
    hFreq->Draw("HIST");

    TF1 *thresholdLine = nullptr;
    if (detection.threshold > 0.0) {
        // Tegner Lukas sin 5-sigma terskel som en rød linje.
        thresholdLine = new TF1("lukas_threshold", "[0]", 0.4, hFreq->GetXaxis()->GetXmax());
        thresholdLine->SetParameter(0, detection.threshold);
        thresholdLine->SetLineColor(kRed + 1);
        thresholdLine->SetLineWidth(3);
        thresholdLine->Draw("SAME");
    }

    for (const BeaconHit &hit : detection.hits) {
        // Marker hver mistenkelige beacon-frekvens med en stiplet rød linje.
        TLine *marker = new TLine(hit.frequency, 0.0, hit.frequency, hit.power);
        marker->SetLineColor(kRed + 1);
        marker->SetLineStyle(2);
        marker->SetLineWidth(2);
        marker->Draw("SAME");
    }

    draw_status_text(detection);

    TLegend *legend = new TLegend(0.65, 0.72, 0.88, 0.88);
    legend->SetBorderSize(0);
    legend->SetFillStyle(0);
    legend->AddEntry(hFreq, "Power spectrum", "l");
    if (thresholdLine) {
        legend->AddEntry(thresholdLine, "Lukas 5-sigma threshold", "l");
    }
    legend->Draw();

    // Lagrer figuren slik at den kan brukes i presentasjon/rapport.
    dashboard->SaveAs("william_dashboard.png");
    std::cout << "[William] Saved dashboard to william_dashboard.png\n";

    std::cout << "\n[William] Lukas-style detection settings:\n";
    std::cout << "  noise gate = f >= " << detection.noiseGateHz << " Hz\n";
    std::cout << "  dynamic 5-sigma threshold = " << detection.threshold << "\n";

    if (detection.hits.empty()) {
        std::cout << "[William] No beacon candidates above Lukas' threshold.\n";
    } else {
        std::cout << "\n[William] Beacon candidates for presentation:\n";
        for (const BeaconHit &hit : detection.hits) {
            std::cout << "  frequency = " << hit.frequency
                      << " Hz, period = " << hit.period
                      << " s, power = " << hit.power << "\n";
        }
    }
}