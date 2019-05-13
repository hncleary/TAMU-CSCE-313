#include <iostream>
#include <vector>
#include <stdio.h>
#include "Histogram.h"
using namespace std;

class HistogramCollection{
private:
    vector<Histogram*> hists; //collection of histograms
public:
    HistogramCollection (){
        hists.clear();
    }
    
    void add (Histogram* h){
        hists.push_back (h);
    }
    
    void print (){
        int nhists = hists.size();
        if (nhists <= 0){
            cout << "Histogram collection is empty" << endl;
            return;
        }
        int sum [nhists];
        memset (sum, 0, nhists * sizeof (int));
    
        int nbins = hists [0]->size();   // number of bins in each hist
        vector<double> range = hists [0]->get_range();
        float delta = (range[1] - range [0])/nbins;
        float st = range [0];
    
        int ndots = 15 + nhists * 6;
        for (int i=0; i< ndots; i++)
            cout << "-";
        cout << endl;
        for (int i=0; i<nbins; i++){
            printf ("[%5.2f,%5.2f): ", st, st + delta);
            for (int j=0; j<nhists; j++){
                cout << setw(5) << hists[j]->get_hist()[i] << " "; 
                sum [j] += hists[j]->get_hist()[i];
            }
            cout << endl;
            st += delta;
        }
        //cout << "--------------------------------------------------------------------------------------" << endl;
        for (int i=0; i< ndots; i++)
            cout << "-";
        cout << endl;
    
        printf ("[%5.2f,%5.2f): ", range[0], range[1]);
        for (int j=0; j<nhists; j++){
            cout << setw(5) << sum [j] << " "; 
        }
        cout << endl;
    }

    

    
};