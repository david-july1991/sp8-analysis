#include "SortRun.h"

Analysis::Regions<double> Analysis::readBunchMaskRm(const Analysis::JSONReader &reader, const std::string prefix) {
  Regions<double> mask;
  if (reader.hasMember(prefix)) {
    {
      auto pFr = reader.getOpt<double>(prefix + ".0");
      auto pTo = reader.getOpt<double>(prefix + ".1");
      if (pFr && pTo) {
        mask.regions.push_back({*pFr, *pTo});
        return mask;
      }
    }

    const int n = reader.getArrSize(prefix);
    for (int i=0; i < n; i++) {
      const int m = reader.getArrSize(prefix + "." + std::to_string(i));
      if (m != 2) throw std::invalid_argument("The array must have 2 elements!");
      auto fr = reader.get<double>(prefix + "." + std::to_string(i) + ".0");
      auto to = reader.get<double>(prefix + "." + std::to_string(i) + ".1");
      mask.regions.push_back({fr, to});
    }
    return mask;
  } else return mask;
}
bool Analysis::SortRun::isFileExist(const char *fileName) {
  std::ifstream file(fileName);
  return file.good();
}

Analysis::SortRun::~SortRun() {
  printf("writing root file... ");
  closeC1();
  closeC2();
  closeTree();
  flushRootFile();
  printf("ok\n");

  if (pIonDataSet) delete[] pIonDataSet;
  if (pElecDataSet) delete[] pElecDataSet;
}

Analysis::SortRun::SortRun(const std::string prfx, const int iNum, const int eNum)
    : Hist(false, numHists),
      prefix(prfx), maxNumOfIons(iNum), maxNumOfElecs(eNum) {
  // Create id
  for (int i = 0; i < 10000; i++) {
    sprintf(id, "%04d", i);
    rootFilename = prfx;
    rootFilename += id;
    rootFilename += ".root";
    if (isFileExist(rootFilename.c_str())) {
      continue;
    }
    break;
  }

  // Setup ROOT
  openRootFile(rootFilename.c_str(), "NEW");
  createTree();
  createHists();
}

void Analysis::SortRun::fillTree(const int ionHitNum, const DataSet *pIons,
                                 const int elecHitNum, const DataSet *pElecs) {
  numOfIons = ionHitNum < maxNumOfIons ? ionHitNum : maxNumOfIons;
  for (int i = 0; i < numOfIons; i++) pIonDataSet[i] = pIons[i];
  for (int i = numOfIons; i < maxNumOfIons; i++) pIonDataSet[i] = dumpData;
  numOfElecs = elecHitNum < maxNumOfElecs ? elecHitNum : maxNumOfElecs;
  for (int i = 0; i < numOfElecs; i++) pElecDataSet[i] = pElecs[i];
  for (int i = numOfElecs; i < maxNumOfElecs; i++) pElecDataSet[i] = dumpData;
  if (existTree()) pRootTree->Fill();
}
const bool Analysis::SortRun::existTree() const {
  return pRootTree != nullptr;
}
void Analysis::SortRun::createTree() {
  closeTree();
  pRootTree = new TTree("resortedData", "Resorted Data");
  std::string str;
  // Ion setup
  str = "Ion";
  pIonDataSet = new DataSet[maxNumOfIons];
  pRootTree->Branch((str + "Num").c_str(), &numOfIons, (str + "Num/I").c_str());
  for (int i = 0; i < maxNumOfIons; i++) {
    char ch[2];
    sprintf(ch, "%01d", i);
    pRootTree->Branch((str + "X" + ch).c_str(), &pIonDataSet[i].x, (str + "X" + ch + "/D").c_str());
    pRootTree->Branch((str + "Y" + ch).c_str(), &pIonDataSet[i].y, (str + "Y" + ch + "/D").c_str());
    pRootTree->Branch((str + "T" + ch).c_str(), &pIonDataSet[i].t, (str + "T" + ch + "/D").c_str());
    pRootTree->Branch((str + "Flag" + ch).c_str(), &pIonDataSet[i].flag, (str + "Flag" + ch + "/I").c_str());
  }
  // Electron setup
  str = "Elec";
  pElecDataSet = new DataSet[maxNumOfIons];
  pRootTree->Branch((str + "Num").c_str(), &numOfElecs, (str + "Num/I").c_str());
  for (int i = 0; i < maxNumOfElecs; i++) {
    char ch[2];
    sprintf(ch, "%01d", i);
    pRootTree->Branch((str + "X" + ch).c_str(), &pElecDataSet[i].x, (str + "X" + ch + "/D").c_str());
    pRootTree->Branch((str + "Y" + ch).c_str(), &pElecDataSet[i].y, (str + "Y" + ch + "/D").c_str());
    pRootTree->Branch((str + "T" + ch).c_str(), &pElecDataSet[i].t, (str + "T" + ch + "/D").c_str());
    pRootTree->Branch((str + "Flag" + ch).c_str(), &pElecDataSet[i].flag, (str + "Flag" + ch + "/I").c_str());
  }
}
TCanvas *Analysis::SortRun::createCanvas(std::string name,
                                         std::string titel,
                                         int xposition,
                                         int yposition,
                                         int pixelsx,
                                         int pixelsy) {
  TCanvas *canvaspointer;
  canvaspointer = new TCanvas(name.c_str(), titel.c_str(), xposition, yposition, pixelsx, pixelsy);
  return canvaspointer;
}
void Analysis::SortRun::createHists() {
#define __TIMESUM_TITLE_BIN_REGION__ "Time [ns]", 1000, -25, 25, "timesum"
#define __TIMEDELAY_TITLE_BIN_REGION__ "Time [ns]", 1000, -250, 250, "timesum"
#define __TIMESUMDIFF_TITLE_BIN_REGION__ "Time diff [ns]", "Time sum [ns]", 500, -250, 250, 500, -25, 25, "timesum"
#define __AFTERCALIB_TITLE_BIN_REGION__(X) "Time1 [ns]", "Time2 [ns]", X*2, -X, X, X*2, -X, X, "timesum"
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimesumU_beforeSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimesumV_beforeSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimesumW_beforeSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimediffU_beforeSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimediffV_beforeSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimediffW_beforeSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionTimesumDiffU_beforeSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionTimesumDiffV_beforeSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionTimesumDiffW_beforeSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimesumU_afterSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimesumV_afterSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimesumW_afterSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimediffU_afterSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimediffV_afterSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_ionTimediffW_afterSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionTimesumDiffU_afterSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionTimesumDiffV_afterSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionTimesumDiffW_afterSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionXYRaw), __AFTERCALIB_TITLE_BIN_REGION__(60));
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionXY), __AFTERCALIB_TITLE_BIN_REGION__(60));
  create2d(SAME_TITLE_WITH_VALNAME(h2_ionXYDev), __AFTERCALIB_TITLE_BIN_REGION__(100));
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimesumU_beforeSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimesumV_beforeSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimesumW_beforeSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimediffU_beforeSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimediffV_beforeSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimediffW_beforeSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecTimesumDiffU_beforeSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecTimesumDiffV_beforeSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecTimesumDiffW_beforeSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimesumU_afterSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimesumV_afterSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimesumW_afterSort), __TIMESUM_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimediffU_afterSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimediffV_afterSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_elecTimediffW_afterSort), __TIMEDELAY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecTimesumDiffU_afterSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecTimesumDiffV_afterSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecTimesumDiffW_afterSort), __TIMESUMDIFF_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecXYRaw), __AFTERCALIB_TITLE_BIN_REGION__(60));
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecXY), __AFTERCALIB_TITLE_BIN_REGION__(60));
  create2d(SAME_TITLE_WITH_VALNAME(h2_elecXYDev), __AFTERCALIB_TITLE_BIN_REGION__(100));

#define __TIMESTAMP__TITLE__BIN__REGION__ "Time [s]", 4000, 0, 4000
  create1d(SAME_TITLE_WITH_VALNAME(h1_timestamp), __TIMESTAMP__TITLE__BIN__REGION__);

#define __TDC_NS__ "Time [ns]", 1000, -500, 500, "TDC"
#define __BUNCHMARKER__ "Time [ns]", 2000, -7500, 2500, "TDC"
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC01), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC02), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC03), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC04), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC05), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC06), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC07), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC08), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC09), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC10), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC11), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC12), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC13), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC14), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC15), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_TDC16), __TDC_NS__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_bunchMarker_beforeRm), __BUNCHMARKER__);
  create1d(SAME_TITLE_WITH_VALNAME(h1_bunchMarker_afterRm), __BUNCHMARKER__);

#define __ION_FISH_TITLE_BIN_REGION__ "TIME [ns]", "Location [mm]", 500, -3000, 12000, 200, -100, 100, "ion"
#define __ION_XY_TITLE_BIN_REGION__ "Location X [mm]", "Location Y [mm]", 200, -100, 100, 200, -100, 100, "ion"
#define __ION_PIPICO_TITLE_BIN_REGION__ "Time 1 [ns]", "Time 2 [ns]", 500, -3000, 12000, 500, -3000, 12000, "ion"
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion1hitXFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion1hitYFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion1hitXY), __ION_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion2hitXFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion2hitYFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion2hitXY), __ION_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion3hitXFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion3hitYFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion3hitXY), __ION_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion4hitXFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion4hitYFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion4hitXY), __ION_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion5hitXFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion5hitYFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion5hitXY), __ION_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion6hitXFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion6hitYFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion6hitXY), __ION_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion7hitXFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion7hitYFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion7hitXY), __ION_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion8hitXFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion8hitYFish), __ION_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion8hitXY), __ION_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion1hit2hitPIPICO), __ION_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion2hit3hitPIPICO), __ION_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion3hit4hitPIPICO), __ION_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion4hit5hitPIPICO), __ION_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion5hit6hitPIPICO), __ION_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion6hit7hitPIPICO), __ION_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_ion7hit8hitPIPICO), __ION_PIPICO_TITLE_BIN_REGION__);

#define __ELEC_FISH_TITLE_BIN_REGION__ "TIME [ns]", "Location [mm]", 1500, -1000, 500, 200, -100, 100, "electron"
#define __ELEC_XY_TITLE_BIN_REGION__ "Location X [mm]", "Location Y [mm]", 200, -100, 100, 200, -100, 100, "electron"
#define __ELEC_PIPICO_TITLE_BIN_REGION__ "Time 1 [ns]", "Time 2 [ns]", 1500, -1000, 500, 1500, -1000, 500, "electron"
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec1hitXFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec1hitYFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec1hitXY), __ELEC_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec2hitXFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec2hitYFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec2hitXY), __ELEC_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec3hitXFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec3hitYFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec3hitXY), __ELEC_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec4hitXFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec4hitYFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec4hitXY), __ELEC_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec5hitXFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec5hitYFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec5hitXY), __ELEC_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec6hitXFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec6hitYFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec6hitXY), __ELEC_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec7hitXFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec7hitYFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec7hitXY), __ELEC_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec8hitXFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec8hitYFish), __ELEC_FISH_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec8hitXY), __ELEC_XY_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec1hit2hitPEPECO), __ELEC_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec2hit3hitPEPECO), __ELEC_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec3hit4hitPEPECO), __ELEC_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec4hit5hitPEPECO), __ELEC_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec5hit6hitPEPECO), __ELEC_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec6hit7hitPEPECO), __ELEC_PIPICO_TITLE_BIN_REGION__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_elec7hit8hitPEPECO), __ELEC_PIPICO_TITLE_BIN_REGION__);
}
void Analysis::SortRun::createC1() {
  closeC1();
  pC1 = createCanvas("ion_canvas", "ion_canvas", 10, 10, 910, 910);
  pC1->Divide(3, 3);
  pC1->cd(1);
  getHist1d(h1_ionTimesumU_afterSort)->Draw();
  pC1->cd(2);
  getHist1d(h1_ionTimesumV_afterSort)->Draw();
  pC1->cd(3);
  getHist1d(h1_ionTimesumW_afterSort)->Draw();
  pC1->cd(4);
  getHist1d(h1_ionTimediffU_afterSort)->Draw();
  pC1->cd(5);
  getHist1d(h1_ionTimediffV_afterSort)->Draw();
  pC1->cd(6);
  getHist1d(h1_ionTimediffW_afterSort)->Draw();
  pC1->cd(7);
  getHist2d(h2_ionXYRaw)->Draw();
  pC1->cd(8);
  getHist2d(h2_ionXY)->Draw();
  pC1->cd(9);
  getHist2d(h2_ionXYDev)->Draw();
}
void Analysis::SortRun::createC2() {
  closeC2();
  pC1 = createCanvas("elec_canvas", "elec_canvas", 10, 10, 910, 910);
  pC1->Divide(3, 3);
  pC1->cd(1);
  getHist1d(h1_elecTimesumU_afterSort)->Draw();
  pC1->cd(2);
  getHist1d(h1_elecTimesumV_afterSort)->Draw();
  pC1->cd(3);
  getHist1d(h1_elecTimesumW_afterSort)->Draw();
  pC1->cd(4);
  getHist1d(h1_elecTimediffU_afterSort)->Draw();
  pC1->cd(5);
  getHist1d(h1_elecTimediffV_afterSort)->Draw();
  pC1->cd(6);
  getHist1d(h1_elecTimediffW_afterSort)->Draw();
  pC1->cd(7);
  getHist2d(h2_elecXYRaw)->Draw();
  pC1->cd(8);
  getHist2d(h2_elecXY)->Draw();
  pC1->cd(9);
  getHist2d(h2_elecXYDev)->Draw();
}
const bool Analysis::SortRun::existC1() const {
  return pC1 != nullptr;
}
const bool Analysis::SortRun::existC2() const {
  return pC2 != nullptr;
}
void Analysis::SortRun::updateC1() {
  if (existC1()) {
    for (int i = 1; i <= 9; i++) {
      pC1->cd(i)->Update();
    }
  }
}
void Analysis::SortRun::updateC1(const bool mdf) {
  if (existC1()) {
    for (int i = 1; i <= 9; i++) {
      pC1->cd(i)->Modified(mdf);
      pC1->cd(i)->Update();
    }
  }
}
void Analysis::SortRun::updateC2() {
  if (existC2()) {
    for (int i = 1; i <= 9; i++) {
      pC2->cd(i)->Update();
    }
  }
}
void Analysis::SortRun::updateC2(const bool mdf) {
  if (existC2()) {
    for (int i = 1; i <= 9; i++) {
      pC2->cd(i)->Modified(mdf);
      pC2->cd(i)->Update();
    }
  }
}
void Analysis::SortRun::closeC1() {
  if (existC1()) {
    updateC1();
    pC1->Close();
    delete pC1;
    pC1 = nullptr;
  }
}
void Analysis::SortRun::closeC2() {
  if (existC2()) {
    updateC1();
    pC2->Close();
    delete pC2;
    pC2 = nullptr;
  }
}
void Analysis::SortRun::closeTree() {
  if (existTree()) {
    pRootTree->Write();
    delete pRootTree;
    pRootTree = nullptr;
  }
}
