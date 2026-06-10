// Master-skript som kjører alle modulene i riktig rekkefølge.
// Kjør med: root -l run_all.cpp

#include "TROOT.h"
#include <iostream>

using namespace std; 

int main()
{
    cout << "\n=========================================" << endl;
    cout << "   PHYS291 - NETWORK BEACON DETECTOR     " << endl;
    cout << "=========================================\n" << endl;

    cout << ">>> STARTER MODUL B (AUGUST) <<<" << endl;
    gROOT->ProcessLine(".x august.cpp");

    cout << "\n>>> STARTER MODUL C & D (WILLIAM / LUKAS) <<<" << endl;
    gROOT->ProcessLine(".x william_lukas.cpp");

    cout << "\n=========================================" << endl;
    cout << "Kjøring fullført! Sjekk william_dashboard.png" << endl;
    cout << "=========================================\n" << endl;
}