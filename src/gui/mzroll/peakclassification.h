#ifndef PEAKCLASSIFICATION_H
#define PEAKCLASSIFICATION_H

#include <vector>

#include "csvparser.h"
#include "PeakGroup.h"

class PeakClassification {

    public:
        void trainPeaks(vector<PeakGroup*> groups);
        void trainPeaks(vector<Peak> peaks);

    private:
        int num_outputs;
        vector< vector<float> > getPeakFeatures(vector<Peak> peaks);
        vector<float> getPeakFeatures(Peak peak);

};

#endif // PEAKCLASSIFICATION_H