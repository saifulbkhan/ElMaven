#include "peakclassification.h"

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
