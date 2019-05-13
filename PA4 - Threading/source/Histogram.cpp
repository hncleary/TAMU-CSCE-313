#include <iostream>
#include <string.h>
#include <iomanip>
using namespace std;

#include "Histogram.h"

Histogram::Histogram(int _nbins, double _start, double _end): nbins (_nbins), start(_start), end(_end){
	//memset (hist, 0, nbins * sizeof (int));	
	hist = vector<int> (nbins, 0);
}
Histogram::~Histogram(){
}
void Histogram::update (double value){
	int bin_index = (int) ((value - start) / (end - start) * nbins);
	if (bin_index <0)
		bin_index= 0;
	else if (bin_index >= nbins)
		bin_index = nbins-1;

	//cout << value << "-" << bin_index << endl;
	hist [bin_index] ++;
}
vector<int> Histogram::get_hist(){
	return hist;
}

vector<double> Histogram::get_range (){
	vector<double> r;
	r.push_back (start);
	r.push_back (end);
	return r;
}
int Histogram::size(){
	return nbins;		
}