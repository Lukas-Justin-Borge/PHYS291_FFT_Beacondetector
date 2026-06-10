// Master-skript som kjører alle modulene i riktig rekkefølge.
// Kjør med: root -l run_all.cpp

#include "TROOT.h"
#include <iostream>

int main()
{
    std::cout << "\n=========================================" << std::endl;
    std::cout << "   PHYS291 - NETWORK BEACON DETECTOR     " << std::endl;
    std::cout << "=========================================\n" << std::endl;

    std::cout << ">>> STARTER MODUL B (AUGUST) <<<" << std::endl;
    // Dette tilsvarer å skrive ".x august.cpp" i ROOT-terminalen
    gROOT->ProcessLine(".x august.cpp");

    std::cout << "\n>>> STARTER MODUL C & D (WILLIAM / LUKAS) <<<" << std::endl;
    // Kjører Williams dashboard (som inneholder Lukas sin deteksjonslogikk)
    gROOT->ProcessLine(".x william.cpp");

    std::cout << "\n=========================================" << std::endl;
    std::cout << "Kjøring fullført! Sjekk william_dashboard.png" << std::endl;
    std::cout << "=========================================\n" << std::endl;
}