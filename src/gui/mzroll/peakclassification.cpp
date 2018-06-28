#include "peakclassification.h"

void PeakClassification::trainPeaks(vector<PeakGroup*> groups) {

    groups = filterMarkedGroups(groups);

    vector<Peak*> allPeaks;

    for (unsigned int i = 0; i < groups.size(); i++) {
        PeakGroup* group = groups[i];
        for (unsigned int j = 0; j < group->peaks.size(); j++) {
            Peak* peak = group->peaks[j];
            allPeaks.push_back(peak);
        }
    }

    if (allPeaks.size() < 2) {
        cerr << "\nAtleast 2 classified peaks required for training model\n";
        return;
    }

}

vector<PeakGroup*> PeakClassification::filterMarkedGroups(vector<PeakGroup*> groups) {

    vector<PeakGroup*> filteredGroups;

    for (unsigned int i = 0; i < groups.size(); i++) {
        PeakGroup* group = groups[i];
        if (group->label == 'g' || group->label == 'b') {
            filteredGroups.push_back(group);
        }
    }

    return filteredGroups;
}
