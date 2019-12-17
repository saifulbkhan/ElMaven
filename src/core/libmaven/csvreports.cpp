#include "csvreports.h"
#include <boost/lexical_cast.hpp>
#include "Compound.h"
#include "datastructures/adduct.h"
#include "PeakDetector.h"
#include "classifierNeuralNet.h"
#include "constants.h"
#include "databases.h"
#include "doctest.h"
#include "isotopeDetection.h"
#include "masscutofftype.h"
#include "mavenparameters.h"
#include "mzSample.h"
#include "mzUtils.h"

CSVReports::CSVReports(string filename,
                       ReportType reportType,
                       vector<mzSample*>& insamples,
                       PeakGroup::QType quantType,
                       bool prmReport,
                       bool includeSetNamesLine,
                       MavenParameters* mp,
                       bool pollyExport)
{
    samples = insamples;
    _groupId = 0;
    _pollyExport = pollyExport;
    sort(samples.begin(), samples.end(), mzSample::compSampleOrder);
    errorReport = "";
    mavenparameters = mp;
    _qtype = quantType;
    _reportType = reportType;
    _prmReport = prmReport;
    _includeSetNamesLine = includeSetNamesLine;

    if (reportType == ReportType::PeakReport) {
        if (samples.size() == 0)
            return;
        if (QString(filename.c_str()).endsWith(".csv", Qt::CaseInsensitive))
            setCommaDelimited();
        else
            setTabDelimited();
        _reportStream.open(filename.c_str(), ios::out);
        /**@brief-  write name of column  if output file is open */
        _insertPeakReportColumnNamesintoCSVFile();
    }

    else if (reportType == ReportType::GroupReport) {
        // if number of sample is zero, output file will not open
        if (insamples.size() == 0)
            return;
        if (QString(filename.c_str()).endsWith(".csv", Qt::CaseInsensitive))
            setCommaDelimited();
        else
            setTabDelimited();
        // after checking initial check, open output file
        _reportStream.open(filename.c_str(), ios::out);

        // write name of column  if output file is open
        _insertGroupReportColumnNamesintoCSVFile(
            filename, _prmReport, _includeSetNamesLine);
    }
}

CSVReports::~CSVReports()
{
    if (_reportStream.is_open())
        _reportStream.close();
}

QString CSVReports::_sanitizeString(const char* s)
{
    QString out = s;
    out.replace(QString("\""), QString("\"\""));
    if (out.contains(SEP.c_str())) {
        out = "\"" + out + "\"";
    }
    return out;
}

void CSVReports::_insertGroupReportColumnNamesintoCSVFile(
    string outputfile,
    bool prmReport,
    bool includeSetNamesLine)
{
    if (_reportStream.is_open()) {
        QStringList groupReportcolnames;

        groupReportcolnames << "label"
                            << "metaGroupId"
                            << "groupId"
                            << "goodPeakCount"
                            << "medMz"
                            << "medRt"
                            << "maxQuality"
                            << "adductName"
                            << "isotopeLabel"
                            << "compound"
                            << "compoundId"
                            << "formula"
                            << "expectedRtDiff"
                            << "ppmDiff"
                            << "parent";

        // if this is a PRM report, add PRM specific columns
        if (prmReport && !_pollyExport) {
            groupReportcolnames << "ms2EventCount"
                                << "fragNumIonsMatched"
                                << "fragmentFractionMatched"
                                << "TICMatched"
                                << "dotProduct"
                                << "weigtedDotProduct"
                                << "hyperGeomScore"
                                << "spearmanRankCorrelation"
                                << "mzFragmentError"
                                << "ms2Purity";
        }

        int cohort_offset = groupReportcolnames.size() - 1;
        QString header = groupReportcolnames.join(SEP.c_str());
        _reportStream << header.toStdString();
        for (unsigned int i = 0; i < samples.size(); i++) {
            string name = samples[i]->getSampleName();
            _reportStream << SEP << _sanitizeString(name.c_str()).toStdString();
        }
        _reportStream << endl;

        if (includeSetNamesLine) {
            for (int i = 0; i < cohort_offset; i++)
                _reportStream << SEP;

            for (size_t i = 0; i < samples.size(); i++) {
                string name = samples[i]->getSetName();
                _reportStream << SEP
                              << _sanitizeString(name.c_str()).toStdString();
            }
            _reportStream << endl;
        }
    } else {
        errorReport =
            "Unable to write to file \"" + QString::fromStdString(outputfile)
            + "\"\n"
            + "Please check if you have permission to write to the specified "
            + "location or the file is not in use";
    }
}

void CSVReports::_insertPeakReportColumnNamesintoCSVFile()
{
    if (_reportStream.is_open()) {
        QStringList peakReportcolnames;
        peakReportcolnames << "groupId"
                           << "compound"
                           << "compoundId"
                           << "formula"
                           << "sample"
                           << "peakMz"
                           << "mzmin"
                           << "mzmax"
                           << "rt"
                           << "rtmin"
                           << "rtmax"
                           << "quality"
                           << "peakIntensity"
                           << "peakArea"
                           << "peakSplineArea"
                           << "peakAreaTop"
                           << "peakAreaCorrected"
                           << "peakAreaTopCorrected"
                           << "noNoiseObs"
                           << "signalBaseLineRatio"
                           << "fromBlankSample";

        QString header = peakReportcolnames.join(SEP.c_str());
        _reportStream << header.toStdString() << endl;
    }
}

void CSVReports::addGroup(PeakGroup* group)
{
    if (_reportType == ReportType::PeakReport)
        _writePeakInfo(group);

    if (_reportType == ReportType::GroupReport) {
        if (group->getCompound() == NULL || group->childCount() == 0) {
            _writeGroupInfo(group);
        } else {
            _insertIsotopes(group);
        }
    }
}

void CSVReports::_insertIsotopes(PeakGroup* group,
                                 bool userSelectedIsotopesOnly)
{
    if (userSelectedIsotopesOnly) {
        _insertUserSelectedIsotopes(group);
    } else {
        for (auto subGroup : group->children) {
            subGroup.metaGroupId = group->metaGroupId;
            _writeGroupInfo(&subGroup);
        }
    }
}

void CSVReports::_insertUserSelectedIsotopes(PeakGroup* group)
{
    bool C13Flag = getMavenParameters()->C13Labeled_BPE;
    bool N15Flag = getMavenParameters()->N15Labeled_BPE;
    bool S34Flag = getMavenParameters()->S34Labeled_BPE;
    bool D2Flag = getMavenParameters()->D2Labeled_BPE;

    // iterate over all existing subgroups and for each isotope flag
    // check if the subgroup contains the isotope's name as tagstring
    // before writing it to the report. If any of the unselected
    // labels are found, we discard the child group.
    for (auto subGroup : group->children) {
        if (!C13Flag && subGroup.tagString.find("C13") != std::string::npos)
            continue;
        if (!N15Flag && subGroup.tagString.find("N15") != std::string::npos)
            continue;
        if (!S34Flag && subGroup.tagString.find("S34") != std::string::npos)
            continue;
        if (!D2Flag && subGroup.tagString.find("D2") != std::string::npos)
            continue;

        subGroup.metaGroupId = group->metaGroupId;
        _writeGroupInfo(&subGroup);
    }
}

void CSVReports::_writeGroupInfo(PeakGroup* group)
{
    if (!_reportStream.is_open())
        return;
    _groupId++;

    char lab;
    lab = group->userLabel();

    PeakGroup* parentGroup = group->getParent();
    if (parentGroup) {
        if (group->userLabel() == '\0') {
            lab = parentGroup->userLabel();
        }
    } else {
        parentGroup = group;
    }

    if (selectionFlag == 2) {
        if (lab != 'g')
            return;
    } else if (selectionFlag == 3) {
        if (lab != 'b')
            return;
    } else if (selectionFlag == 4) {
        if (lab == 'b')
            return;
    }

    vector<float> yvalues = group->getOrderedIntensityVector(samples, _qtype);

    string tagString = group->srmId + group->tagString;
    // using the new funtionality added - Kiran
    tagString = _sanitizeString(tagString.c_str()).toStdString();

    char label[2];
    sprintf(label, "%c", group->userLabel());

    string adductName = "";
    if (group->getAdduct() != nullptr)
        adductName = group->getAdduct()->getName();

    _reportStream << label << SEP << parentGroup->groupId << SEP << _groupId
                  << SEP << group->goodPeakCount << fixed << SEP
                  << setprecision(6) << group->meanMz << SEP << setprecision(3)
                  << group->meanRt << SEP << setprecision(6)
                  << group->maxQuality << SEP << adductName << SEP << tagString;
    string compoundName = "";
    string compoundID = "";
    string formula = "";
    string categoryString;
    float expectedRtDiff = 0;
    float ppmDist = 0;

    if (group->getCompound() != NULL) {
        compoundName =
            _sanitizeString(group->getCompound()->name.c_str()).toStdString();
        compoundID =
            _sanitizeString(group->getCompound()->id.c_str()).toStdString();
        formula = _sanitizeString(group->getCompound()->formula().c_str())
                      .toStdString();
        if (!group->getCompound()->formula().empty()) {
            int charge = getMavenParameters()->getCharge(group->getCompound());
            if (group->parent != NULL) {
                ppmDist = mzUtils::massCutoffDist(
                    (double)group->getExpectedMz(charge),
                    (double)group->meanMz,
                    getMavenParameters()->massCutoffMerge);
            } else {
                ppmDist = mzUtils::massCutoffDist(
                    (double)group->getCompound()->adjustedMass(charge),
                    (double)group->meanMz,
                    getMavenParameters()->massCutoffMerge);
            }
        } else {
            ppmDist =
                mzUtils::massCutoffDist((double)group->getCompound()->mass,
                                        (double)group->meanMz,
                                        getMavenParameters()->massCutoffMerge);
        }
        expectedRtDiff = group->expectedRtDiff();
        // TODO: Added this while merging this file
    } else {
        // absence of a group compound means this group was created using
        // untargeted detection, we set compound name and ID to {mz}@{rt}
        // strings for untargeted sets.
        compoundName =
            std::to_string(group->meanMz) + "@" + std::to_string(group->meanRt);
        compoundID = compoundName;
    }

    _reportStream << SEP << compoundName << SEP << compoundID << SEP << formula
                  << SEP << setprecision(3) << expectedRtDiff << SEP
                  << setprecision(6) << ppmDist;

    if (group->parent != NULL) {
        _reportStream << SEP << group->parent->meanMz;
    } else {
        _reportStream << SEP << group->meanMz;
    }

    if (group->getCompound()
        && group->getCompound()->type() == Compound::Type::PRM
        && !_pollyExport) {
        auto groupToWrite = group;

        // if this is a C12 PARENT, then all PRM attributes should be taken from
        // its parent group.
        if (group->tagString.find("C12 PARENT") != std::string::npos)
            groupToWrite = group->parent;

        _reportStream << SEP << groupToWrite->ms2EventCount << SEP
                      << groupToWrite->fragMatchScore.numMatches << SEP
                      << groupToWrite->fragMatchScore.fractionMatched << SEP
                      << groupToWrite->fragMatchScore.ticMatched << SEP
                      << groupToWrite->fragMatchScore.dotProduct << SEP
                      << groupToWrite->fragMatchScore.weightedDotProduct << SEP
                      << groupToWrite->fragMatchScore.hypergeomScore << SEP
                      << groupToWrite->fragMatchScore.spearmanRankCorrelation
                      << SEP << groupToWrite->fragMatchScore.mzFragError << SEP
                      << groupToWrite->fragmentationPattern.purity;
    }

    // for intensity values, we only write two digits of floating point
    // precision since these values are supposed to be large (in the order of >
    // 10^3).
    _reportStream << setprecision(2);
    for (unsigned int j = 0; j < samples.size(); j++) {
        for (int i = 0; i < static_cast<int>(group->samples.size()); ++i) {
            if (samples[j]->sampleName == group->samples[i]->sampleName) {
                _reportStream << SEP << yvalues[j];
                break;
            } else if (i == static_cast<int>(group->samples.size()) - 1) {
                _reportStream << SEP << "NA";
            }
        }
    }
    _reportStream << endl;
}

void CSVReports::_writePeakInfo(PeakGroup* group)
{
    string compoundName = "";
    string compoundID = "";
    string formula = "";
    if (group->getCompound() != NULL) {
        compoundName =
            _sanitizeString(group->getCompound()->name.c_str()).toStdString();
        compoundID =
            _sanitizeString(group->getCompound()->id.c_str()).toStdString();
        formula = _sanitizeString(group->getCompound()->formula().c_str())
                      .toStdString();
    } else {
        // absence of a group compound means this group was created using
        // untargeted detection,
        // we set compound name and ID to {mz}@{rt} strings for untargeted sets.
        compoundName =
            std::to_string(group->meanMz) + "@" + std::to_string(group->meanRt);
        compoundID = compoundName;
    }

    if (selectionFlag == 2) {
        if (group->userLabel() != 'g')
            return;
    } else if (selectionFlag == 3) {
        if (group->userLabel() != 'b')
            return;
    } else if (selectionFlag == 4) {
        if (group->userLabel() == 'b')
            return;
    }

    // sort the peaks in the group according to the sample names using a
    // comparison function
    // this ensures that the order in which the peaks are written is same across
    // different systems.
    std::sort(group->peaks.begin(), group->peaks.end(), Peak::compSampleName);

    vector<mzSample*> samplesWithNoPeak = samples;
    for (unsigned int j = 0; j < group->peaks.size(); j++) {
        Peak& peak = group->peaks[j];
        mzSample* sample = peak.getSample();
        string sampleName;
        if (sample != NULL) {
            samplesWithNoPeak.erase(
                remove_if(begin(samplesWithNoPeak),
                          end(samplesWithNoPeak),
                          [sample](mzSample* s) { return s == sample; }),
                end(samplesWithNoPeak));

            sampleName =
                _sanitizeString(sample->sampleName.c_str()).toStdString();
        }

        _reportStream << fixed << setprecision(6) << group->groupId << SEP
                      << compoundName << SEP << compoundID << SEP << formula
                      << SEP << sampleName << SEP << peak.peakMz << SEP
                      << peak.mzmin << SEP << peak.mzmax << setprecision(3)
                      << SEP << peak.rt << SEP << peak.rtmin << SEP
                      << peak.rtmax << SEP
                      << peak.quality
                      // for intensity values, we only write two digits of
                      // floating point precision
                      // since these values are supposed to be large
                      // (in the order of > 10^3).
                      << setprecision(2) << SEP << peak.peakIntensity << SEP
                      << peak.peakArea << SEP << peak.peakSplineArea << SEP
                      << peak.peakAreaTop << SEP << peak.peakAreaCorrected
                      << SEP << peak.peakAreaTopCorrected << SEP
                      << peak.noNoiseObs << SEP << peak.signalBaselineRatio
                      << SEP << peak.fromBlankSample << endl;
    }
    for (auto sample : samplesWithNoPeak) {
        string sampleName = "";
        if (sample != nullptr) {
            sampleName =
                _sanitizeString(sample->sampleName.c_str()).toStdString();
        }
        _reportStream << fixed << setprecision(6) << group->groupId << SEP
                      << compoundName << SEP << compoundID << SEP << formula
                      << SEP << sampleName << SEP << 0.0f << SEP << 0.0f << SEP
                      << 0.0f << setprecision(3) << SEP << 0.0f << SEP << 0.0f
                      << SEP << 0.0f << SEP << 0.0f << setprecision(2) << SEP
                      << 0.0f << SEP << 0.0f << SEP << 0.0f << SEP << 0.0f
                      << SEP << 0.0f << SEP << 0.0f << SEP << 0.0f << SEP
                      << 0.0f << SEP << 0 << endl;
    }
}

bool CSVReports::writeDataForPeakMl(const string& filePath,
                                    const vector<PeakGroup>& groups)
{
    ofstream file(filePath);
    if (file.is_open()) {
        file << "groupId" << ","
             << "sampleName" << ","
             << "cohortName" << ","
             << "peakArea" << ","
             << "peakAreaTop" << ","
             << "peakAreaFractional" << ","
             << "peakIntensity" << ","
             << "peakMz" << ","
             << "peakRt" << ","
             << "peakRtMin" << ","
             << "peakRtMax" << ","
             << "quality" << ","
             << "width" << ","
             << "gaussFitR2" << ","
             << "noNoiseFraction" << ","
             << "symmetry" << ","
             << "signalBaselineRatio" << ","
             << "groupOverlap" << "\n";
        file << fixed << setprecision(6);
        for (auto& group : groups) {
            for (auto& peak : group.peaks) {
                file << group.groupId << ","
                     << "\"" << peak.getSample()->getSampleName() << "\"" << ","
                     << "\"" << peak.getSample()->getSetName() << "\"" << ","
                     << peak.peakArea << ","
                     << peak.peakAreaTop << ","
                     << peak.peakAreaFractional << ","
                     << peak.peakIntensity << ","
                     << peak.peakMz << ","
                     << peak.rt << ","
                     << peak.rtmin << ","
                     << peak.rtmax << ","
                     << peak.quality << ","
                     << peak.width << ","
                     << peak.gaussFitR2 << ","
                     << peak.noNoiseFraction << ","
                     << peak.symmetry << ","
                     << peak.signalBaselineRatio << ","
                     << peak.groupOverlap << "\n";
            }
        }
        file.close();
        return true;
    }
    return false;
}

///////////////////////////Test Cases//////////////////////////////

class CSVReportFixture
{
    private:
    vector<mzSample*> _samples;
    vector<PeakGroup> _allgroups;
    MavenParameters* _mavenparameters;
    std::list<PeakGroup> _isotopeGroups;
    Databases database;

    void _makeSampleList()
    {
        auto sample1 = new mzSample();
        auto sample2 = new mzSample();
        auto sample3 = new mzSample();
        auto sample4 = new mzSample();
        sample1->loadSample("bin/methods/091215_120i.mzXML");
        sample2->loadSample("bin/methods/091215_120M.mzXML");
        sample3->loadSample("bin/methods/091215_240i.mzXML");
        sample4->loadSample("bin/methods/091215_240M.mzXML");

        _samples.push_back(sample1);
        _samples.push_back(sample2);
        _samples.push_back(sample3);
        _samples.push_back(sample4);
    }

    void _loadSamplesAndParameters(vector<mzSample*>& samplesToLoad,
                                   MavenParameters* mavenparameters)
    {
        ClassifierNeuralNet* clsf = new ClassifierNeuralNet();
        string loadmodel = "bin/default.model";
        clsf->loadModel(loadmodel);
        mavenparameters->compoundMassCutoffWindow->setMassCutoffAndType(10,
                                                                        "ppm");
        mavenparameters->clsf = clsf;
        mavenparameters->ionizationMode = -1;
        mavenparameters->matchRtFlag = true;
        mavenparameters->compoundRTWindow = 1;
        mavenparameters->samples = samplesToLoad;
        mavenparameters->eic_smoothingWindow = 10;
        mavenparameters->eic_smoothingAlgorithm = 1;
        mavenparameters->amuQ1 = 0.25;
        mavenparameters->amuQ3 = 0.30;
        mavenparameters->baseline_smoothingWindow = 5;
        mavenparameters->baseline_dropTopX = 80;
    }

    vector<PeakGroup> _getTargetedGroupsFromProcessCompounds()
    {
        const char* loadCompoundDB = "bin/methods/KNOWNS.csv";
        database.loadCompoundCSVFile(loadCompoundDB);
        vector<Compound*> compounds = database.getCompoundsSubset("KNOWNS");
        _loadSamplesAndParameters(_samples, _mavenparameters);
        PeakDetector peakDetector;
        peakDetector.setMavenParameters(_mavenparameters);
        vector<mzSlice*> slices =
            peakDetector.processCompounds(compounds, "compounds");
        peakDetector.processSlices(slices, "compounds");
        return _mavenparameters->allgroups;
    }

    vector<PeakGroup> _getUntargetedGroups()
    {
        _loadSamplesAndParameters(_samples, _mavenparameters);
        PeakDetector peakDetector;
        peakDetector.setMavenParameters(_mavenparameters);
        peakDetector.processMassSlices();
        return _mavenparameters->allgroups;
    }

    public:
    CSVReportFixture()
    {
        _mavenparameters = new MavenParameters();
        _makeSampleList();
    }

    ~CSVReportFixture()
    {
        delete _samples[0];
        delete _samples[1];
        delete _samples[2];
        delete _samples[3];
        delete _mavenparameters;
    }

    void targetedGroup()
    {
        _allgroups = _getTargetedGroupsFromProcessCompounds();
        for (size_t i = 0; i < _allgroups.size(); i++)
            _isotopeGroups.push_back(_allgroups[i]);
        detectIsotopes(_isotopeGroups);
    }

    void untargetedGroup()
    {
        _allgroups = _getUntargetedGroups();
        for (size_t i = 0; i < _allgroups.size(); i++)
            _isotopeGroups.push_back(_allgroups[i]);
        detectIsotopes(_isotopeGroups);
    }
    /**
     * @brief detectIsotopes Detects isotopes in the group.
     * @param isotopeGroups  List of peakGroups.
     */
    void detectIsotopes(list<PeakGroup> isotopeGroups)
    {
        for (auto it = isotopeGroups.begin(); it != isotopeGroups.end(); it++) {
            PeakGroup& parent = *it;
            IsotopeDetection isotopeDetection(mavenparameters(),
                                              IsotopeDetection::PeakDetection,
                                              mavenparameters()->C13Labeled_BPE,
                                              mavenparameters()->N15Labeled_BPE,
                                              mavenparameters()->S34Labeled_BPE,
                                              mavenparameters()->D2Labeled_BPE);
            isotopeDetection.pullIsotopes(&parent);
            _isotopeGroups.push_back(parent);
        }
    }

    /**
     * @brief samples Returns vector of samples loaded.
     * @return
     */
    vector<mzSample*> samples()
    {
        return _samples;
    }

    /**
     * @brief allgroups Returns all peakgroups of the mavenparameters.
     * @return
     */
    vector<PeakGroup> allgroups()
    {
        return _allgroups;
    }

    /**
     * @brief mavenparameters   Returns MavenParameters
     * @return
     */
    MavenParameters* mavenparameters()
    {
        return _mavenparameters;
    }

    /**
     * @brief mavenparameters   Returns isotopesGroups
     * @return
     */
    std::list<PeakGroup> isotopeGroup()
    {
        return _isotopeGroups;
    }
};

TEST_CASE_FIXTURE(CSVReportFixture, "Testing Targeted Groups")
{
    SUBCASE("Testing Group File")
    {
        targetedGroup();
        auto sample = samples();
        auto mavenparameter = mavenparameters();
        string groupReport = "groupReport.csv";
        CSVReports* csvReports =
            new CSVReports(groupReport,
                           CSVReports::ReportType::GroupReport,
                           sample,
                           PeakGroup::AreaTop,
                           false,
                           true,
                           mavenparameter);
        auto allgroup = allgroups();
        for (int i = 0; i < static_cast<int>(allgroups().size()); i++) {
            PeakGroup* peakGroup = new PeakGroup(allgroup[i]);
            csvReports->addGroup(peakGroup);
        }

        ifstream inputGroupFile("groupReport.csv");
        ifstream savedGroupFile(
            "tests/test-libmaven/test_TargetedGroupReport.csv");
        string headerInput;
        getline(inputGroupFile, headerInput);
        getline(inputGroupFile, headerInput);
        string headerSaved;
        getline(savedGroupFile, headerSaved);
        getline(savedGroupFile, headerSaved);

        int cnt = 0;
        while (!inputGroupFile.eof()) {
            cnt++;
            string input;
            getline(inputGroupFile, input);
            if (input.empty())
                continue;

            if (input.size() > 0) {
                vector<string> inputValues;
                mzUtils::splitNew(input, ",", inputValues);
                if (cnt > 1) {
                    savedGroupFile.clear();
                    savedGroupFile.seekg(0, ios::beg);
                    string headerSaved;
                    getline(savedGroupFile, headerSaved);
                    getline(savedGroupFile, headerSaved);
                }

                while (!savedGroupFile.eof()) {
                    string saved;
                    getline(savedGroupFile, saved);
                    if (saved.empty())
                        continue;

                    vector<string> savedValues;
                    mzUtils::splitNew(saved, ",", savedValues);

                    if (string2float(inputValues[4])
                            == doctest::Approx(string2float(savedValues[4]))
                        && string2float(inputValues[5])
                               == doctest::Approx(string2float(savedValues[5]))
                        &&
                        /*epsilon value has to be a greater term i.e 15% as
                          inputValue[12] is parts per millions. Thus, it may
                          show a more deviation that normal */
                        string2float(inputValues[13])
                            == doctest::Approx(string2float(savedValues[13]))
                                   .epsilon(0.15)
                        && inputValues[9] == savedValues[9]) {
                        double inputFloat;
                        double savedFloat;
                        for (int i = 3;
                             i < static_cast<int>(inputValues.size());
                             i++) {
                            if (i == 9 || i == 10 || i == 11) {
                                REQUIRE(inputValues[i] == savedValues[i]);
                            } else if (i == 7) {
                                // adducts
                                REQUIRE (inputValues[i] == savedValues[i]);
                            } else {
                                inputFloat = string2float(inputValues[i]);
                                savedFloat = string2float(savedValues[i]);
                                REQUIRE(inputFloat
                                        == doctest::Approx(savedFloat)
                                               .epsilon(0.15));
                            }
                        }
                        break;
                    }
                }
            }
        }
        inputGroupFile.close();
        savedGroupFile.close();
        remove("groupReport.csv");
    }

    SUBCASE("Testing Peak File")
    {
        targetedGroup();
        string peakReport = "peakReport.csv";
        auto sample = samples();
        auto mavenparameter = mavenparameters();

        CSVReports* csvReports =
            new CSVReports(peakReport,
                           CSVReports::ReportType::PeakReport,
                           sample,
                           PeakGroup::AreaTop,
                           false,
                           true,
                           mavenparameter);

        auto allgroup = allgroups();
        for (int i = 0; i < static_cast<int>(allgroup.size()); i++) {
            PeakGroup* peakGroup = new PeakGroup(allgroup[i]);
            csvReports->addGroup(peakGroup);
        }

        ifstream inputPeakFile("peakReport.csv");
        ifstream savedPeakFile(
            "tests/test-libmaven/test_TargetedPeakReport.csv");

        string headerInput;
        getline(inputPeakFile, headerInput);

        string headerSaved;
        getline(savedPeakFile, headerSaved);

        int cnt = 0;
        while (!inputPeakFile.eof()) {
            cnt++;
            string input;
            getline(inputPeakFile, input);

            if (input.size() > 0) {
                vector<string> inputValues;
                mzUtils::splitNew(input, ",", inputValues);

                if (cnt > 1) {
                    savedPeakFile.clear();
                    savedPeakFile.seekg(0, ios::beg);
                    string headerSaved;
                    getline(savedPeakFile, headerSaved);
                }

                while (!savedPeakFile.eof()) {
                    string saved;
                    getline(savedPeakFile, saved);
                    vector<string> savedValues;
                    mzUtils::splitNew(saved, ",", savedValues);
                    if (string2float(inputValues[8])
                            == doctest::Approx(string2float(savedValues[8]))
                        && string2float(inputValues[12])
                               == doctest::Approx(string2float(savedValues[12]))
                        && inputValues[2] == savedValues[2]) {
                        double inputFloat;
                        double savedFloat;
                        for (int i = 1;
                             i < static_cast<int>(inputValues.size());
                             i++) {
                            if (i == 1 || i == 2 || i == 3) {
                                REQUIRE(inputValues[i] == savedValues[i]);
                            } else if (i == 4)
                                continue;
                            else {
                                inputFloat = string2float(inputValues[i]);
                                savedFloat = string2float(savedValues[i]);
                                REQUIRE(inputFloat
                                        == doctest::Approx(savedFloat)
                                               .epsilon(0.15));
                            }
                        }
                        break;
                    }
                }
            }
        }
        inputPeakFile.close();
        savedPeakFile.close();
        remove("peakReport.csv");
    }

    SUBCASE("Testing Untargeted Group File")
    {
        untargetedGroup();
        auto sample = samples();
        auto mavenparameter = mavenparameters();
        string groupReport = "groupReport.csv";
        CSVReports* csvReports =
            new CSVReports(groupReport,
                           CSVReports::ReportType::GroupReport,
                           sample,
                           PeakGroup::AreaTop,
                           false,
                           true,
                           mavenparameter);
        auto allgroup = allgroups();
        for (int i = 0; i < static_cast<int>(allgroups().size()); i++) {
            PeakGroup* peakGroup = new PeakGroup(allgroup[i]);
            csvReports->addGroup(peakGroup);
        }
        ifstream inputGroupFile("groupReport.csv");
        ifstream savedGroupFile(
            "tests/test-libmaven/test_untargetedGroupReport.csv");
        string headerInput;
        getline(inputGroupFile, headerInput);
        getline(inputGroupFile, headerInput);
        string headerSaved;
        getline(savedGroupFile, headerSaved);
        getline(savedGroupFile, headerSaved);

        int cnt = 0;

        while (!inputGroupFile.eof()) {
            cnt++;
            string input;
            getline(inputGroupFile, input);
            if (input.empty())
                continue;

            if (input.size() > 0) {
                vector<string> inputValues;
                mzUtils::splitNew(input, ",", inputValues);
                if (cnt > 1) {
                    savedGroupFile.clear();
                    savedGroupFile.seekg(0, ios::beg);
                    string headerSaved;
                    getline(savedGroupFile, headerSaved);
                    getline(savedGroupFile, headerSaved);
                }

                while (!savedGroupFile.eof()) {
                    string saved;
                    getline(savedGroupFile, saved);
                    if (saved.empty())
                        continue;

                    vector<string> savedValues;
                    mzUtils::splitNew(saved, ",", savedValues);
                    if (string2float(inputValues[4])
                            == doctest::Approx(string2float(savedValues[4]))
                        && string2float(inputValues[5])
                               == doctest::Approx(string2float(savedValues[5]))
                                      .epsilon(0.01)
                        && inputValues[3] == savedValues[3]) {
                        double inputFloat;
                        double savedFloat;
                        // TODO: why not use column names instead of indexes
                        for (size_t i = 3; i < inputValues.size(); i++) {
                            if (i == 9 || i == 10) {
                                continue;
                            } else if (i == 7) {
                                // adducts
                                REQUIRE (inputValues[i] == savedValues[i]);
                            } else {
                                inputFloat = string2float(inputValues[i]);
                                savedFloat = string2float(savedValues[i]);
                                REQUIRE(inputFloat
                                        == doctest::Approx(savedFloat)
                                               .epsilon(0.15));
                            }
                        }
                        break;
                    }
                }
            }
        }
        inputGroupFile.close();
        savedGroupFile.close();
        remove("groupReport.csv");
    }

    SUBCASE("Testing Untargeted Peak File")
    {
        untargetedGroup();
        string peakReport = "peakReport.csv";
        auto sample = samples();
        auto mavenparameter = mavenparameters();

        CSVReports* csvReports =
            new CSVReports(peakReport,
                           CSVReports::ReportType::PeakReport,
                           sample,
                           PeakGroup::AreaTop,
                           false,
                           true,
                           mavenparameter);

        auto allgroup = allgroups();
        for (int i = 0; i < static_cast<int>(allgroup.size()); i++) {
            PeakGroup* peakGroup = new PeakGroup(allgroup[i]);
            csvReports->addGroup(peakGroup);
        }
        ifstream inputPeakFile("peakReport.csv");
        ifstream savedPeakFile(
            "tests/test-libmaven/test_untargetedPeakReport.csv");

        string headerInput;
        getline(inputPeakFile, headerInput);

        string headerSaved;
        getline(savedPeakFile, headerSaved);

        int cnt = 0;
        while (!inputPeakFile.eof()) {
            cnt++;

            string input;
            getline(inputPeakFile, input);

            if (input.size() > 0) {
                vector<string> inputValues;
                mzUtils::splitNew(input, ",", inputValues);

                if (cnt > 1) {
                    savedPeakFile.clear();
                    savedPeakFile.seekg(0, ios::beg);
                    string headerSaved;
                    getline(savedPeakFile, headerSaved);
                }

                while (!savedPeakFile.eof()) {
                    string saved;
                    getline(savedPeakFile, saved);
                    vector<string> savedValues;
                    mzUtils::splitNew(saved, ",", savedValues);

                    if (string2float(inputValues[16])
                            == doctest::Approx(string2float(savedValues[16]))
                                   .epsilon(0.0005)
                        && string2float(inputValues[12])
                               == doctest::Approx(string2float(savedValues[12]))
                                      .epsilon(0.0005)
                        && inputValues[4] == savedValues[4]
                        && string2float(inputValues[11])
                               == doctest::Approx(string2float(savedValues[11]))
                                      .epsilon(0.0005)) {
                        double inputFloat;
                        double savedFloat;
                        for (size_t i = 3; i < inputValues.size(); i++) {
                            if (i == 4) {
                                REQUIRE(inputValues[i] == savedValues[i]);
                            } else {
                                inputFloat = string2float(inputValues[i]);
                                savedFloat = string2float(savedValues[i]);
                                REQUIRE(inputFloat
                                        == doctest::Approx(savedFloat)
                                               .epsilon(0.15));
                            }
                        }
                        break;
                    }
                }
            }
        }
        inputPeakFile.close();
        savedPeakFile.close();
        remove("peakReport.csv");
    }
}
