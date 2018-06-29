#include "peakclassification.h"

PeakClassification::PeakClassification() {
    num_features = 9;
}

void PeakClassification::trainPeaks(vector<PeakGroup*> groups) {

    vector<Peak> allPeaks;
    for (unsigned int i = 0; i < groups.size(); i++) {
        PeakGroup* group = groups[i];
        if (!group->isMarked()) continue;
        for (unsigned int j = 0; j < group->peaks.size(); j++) {
            Peak peak = group->peaks[j];
            allPeaks.push_back(peak);
        }
    }

    trainPeaks(allPeaks);

}

void PeakClassification::trainPeaks(vector<Peak> peaks) {

    if (peaks.size() < 2) {
        cerr << "\nAtleast 2 classified peaks required for training model\n";
        return;
    }

    vector< vector<float> > features = getPeakFeatures(peaks);

}

vector< vector<float> > PeakClassification::getPeakFeatures(vector<Peak> peaks) {

    vector< vector<float> > features;

    for(unsigned int i = 0; i < peaks.size(); i++) {
        Peak peak = peaks[i];
        features.push_back(getPeakFeatures(peak));
    }

}

vector <float> PeakClassification::getPeakFeatures(Peak peak) {

	vector<float> features(num_features, 0);

	if (peak.width > 0) {
		features[0] = peak.peakAreaFractional;
		features[1] = peak.noNoiseFraction;
		features[2] = peak.symmetry / (peak.width + 1) * log2(peak.width + 1);
		features[3] = peak.groupOverlapFrac;
		features[4] = peak.gaussFitR2 * 100.0;
		
        features[5] = peak.signalBaselineRatio > 0 ? log2(peak.signalBaselineRatio) / 10.0 : 0;
		features[6] = peak.peakRank / 10.0;
		features[7] = peak.peakIntensity > 0 ? log10(peak.peakIntensity) : 0;
		features[8] = peak.width <= 3 && peak.signalBaselineRatio >= 3.0 ? 1 : 0;
		if (peak.peakRank / 10.0 > 1)
			features[6] = 1;
	}
	return features;


}
