#ifndef PEAKCLASSIFICATION_H
#define PEAKCLASSIFICATION_H

#include <vector>

#include "csvparser.h"
#include "PeakGroup.h"

struct PeakFeatures;

class PeakClassification {

    public:
        void trainPeaks(vector<PeakGroup*> groups);
        void trainPeaks(vector<Peak> peaks);

    private:
        PeakFeatures getPeakFeatures(Peak peak);
        void exportFeatures(vector<PeakFeatures> features);

};

struct PeakFeatures {

    char label = '\0';
    float peakAreaFractional = 0;
    float noNoiseFraction = 0;
    float symmetry = 0;
    float groupOverlap = 0;
    float gaussFitR2 = 0;
    float signalToNoise = 0;
    float peakRank = 0;
    float peakIntensity = 0;
    float skinnyPeak = 0;

};

#endif // PEAKCLASSIFICATION_H