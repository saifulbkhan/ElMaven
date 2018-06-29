#include "peakclassification.h"

void PeakClassification::trainPeaks(vector<PeakGroup*> groups) {

    map<Peak, char> allPeaks;
    for (unsigned int i = 0; i < groups.size(); i++) {
        PeakGroup* group = groups[i];
        if (!group->isMarked()) continue;
        for (unsigned int j = 0; j < group->peaks.size(); j++) {
            Peak peak = group->peaks[j];
            allPeaks[peak] = group->label;
        }
    }
    trainPeaks(allPeaks);
}

void PeakClassification::trainPeaks(map<Peak, char> peaks) {

    if (peaks.size() < 2) {
        cerr << "\nAtleast 2 classified peaks required for training model\n";
        return;
    }

    vector<PeakFeatures*> features;
    for(map<Peak, char>::iterator it= peaks.begin(); it!=peaks.end(); ++it) {
        PeakFeatures* peakfeatures = getPeakFeatures(it->first); 
        peakfeatures->label = it->second;
        features.push_back(PeakFeatures);
    }

    featuresToCsv(features);

}

PeakFeatures* PeakClassification::getPeakFeatures(Peak peak) {

    PeakFeatures* peakFeatures;

	if (peak.width > 0) {
		peakFeatures->peakAreaFractional = peak.peakAreaFractional;
		peakFeatures->noNoiseFraction = peak.noNoiseFraction;
		peakFeatures->symmetry = peak.symmetry / (peak.width + 1) * log2(peak.width + 1);
		peakFeatures->groupOverlap = peak.groupOverlapFrac;
		peakFeatures->gaussFitR2 = peak.gaussFitR2 * 100.0;
        peakFeatures->signalToNoise = peak.signalBaselineRatio > 0 ? log2(peak.signalBaselineRatio) / 10.0 : 0;
		peakFeatures->peakRank = peak.peakRank / 10.0;
		peakFeatures->peakIntensity = peak.peakIntensity > 0 ? log10(peak.peakIntensity) : 0;
		peakFeatures->skinnyPeak = peak.width <= 3 && peak.signalBaselineRatio >= 3.0 ? 1 : 0;
		if (peak.peakRank / 10.0 > 1)
			peakFeatures->peakRank = 1;
	}
	return peakFeatures;

}

void PeakClassification::featuresToCsv(vector<PeakFeatures*> features) {

    QString fileName = "test.csv";

    QFile data(fileName);
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);

        //header
        out << "label" << SEP;
        out << "peakAreaFractional" << SEP;
	    out << "noNoiseFraction" << SEP;
	    out << "symmetry" << SEP;
	    out << "groupOverlap" << SEP;
	    out << "gaussFitR2" << SEP;
	    out << "signalToNoise" << SEP;
	    out << "peakRank" << SEP;
	    out << "peakIntensity" << SEP;
	    out << "skinnyPeak" << SEP;


        for(unsigned int i=0;  i < features.size(); i++ ) {
            PeakFeatures* peakFeatures = features[i];
            out << peakFeatures->label << SEP;
            out << peakFeatures->peakAreaFractional << SEP;
            out << peakFeatures->noNoiseFraction << SEP;
            out << peakFeatures->symmetry << SEP;
            out << peakFeatures->groupOverlap << SEP;
            out << peakFeatures->gaussFitR2 << SEP;
            out << peakFeatures->signalToNoise << SEP;
            out << peakFeatures->peakRank << SEP;
            out << peakFeatures->peakIntensity << SEP;
            out << peakFeatures->skinnyPeak << SEP;
            out << "\n";
        }
    }

}
