#ifndef PEAKCLASSIFICATION_H
#define PEAKCLASSIFICATION_H

#include <vector>

#include "csvparser.h"
#include "PeakGroup.h"


class PeakClassification {

    public:
        PeakClassification();
        void trainPeaks(vector<PeakGroup*> groups);

    private:
        vector<PeakGroup*> filterMarkedGroups(vector<PeakGroup*> groups);

};

#endif // PEAKCLASSIFICATION_H