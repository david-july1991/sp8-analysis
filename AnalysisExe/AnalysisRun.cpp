//
// Created by Daehyun You on 12/30/15.
//

#include "AnalysisRun.h"

Analysis::AnalysisRun::AnalysisRun(const Analysis::JSONReader &configReader)
    : Hist(false, numberOfHists) {

  // Setup writer
  pLogWriter = new Analysis::LogWriter(
      configReader.getStringAt("setup_output.filename_prefix"));

  // Setup input ROOT files
  std::cout << "Setting up input root files... ";
  pEventChain = new TChain(configReader.getStringAt("setup_input.tree_name").c_str());
  pEventChain->Add(configReader.getStringAt("setup_input.filenames").c_str());
  pLogWriter->write() << "Filenames: "
                      << configReader.getStringAt("setup_input.filenames").c_str()
                      << std::endl;
  maxNumOfIonHits = configReader.getIntAt("setup_input.max_number_of_ion_hits");
  maxNumOfElecHits = configReader.getIntAt("setup_input.max_number_of_electron_hits");
  pEventReader = new Analysis::EventDataReader(maxNumOfIonHits, maxNumOfElecHits);
  if (configReader.getBoolAtIfItIs("setup_input.is_having_number_of_hits", false)) {
    for (EventDataReader::TreeName name : {EventDataReader::IonNum,
                                           EventDataReader::ElecNum}) {
      pEventChain->SetBranchAddress(
          EventDataReader::getTreeName(name).c_str(),
          &(pEventReader->setNumObjs(name)));
    }
  }
  for (int i = 0; i < maxNumOfIonHits; i++) {
    for (EventDataReader::TreeName name : {EventDataReader::IonX,
                                           EventDataReader::IonY,
                                           EventDataReader::IonT}) {
      pEventChain->SetBranchAddress(
          EventDataReader::getTreeName(name, i).c_str(),
          &(pEventReader->setEventDataAt(name, i)));
    }
    {
      EventDataReader::TreeName name = EventDataReader::IonFlag;
      pEventChain->SetBranchAddress(
          EventDataReader::getTreeName(name, i).c_str(),
          &(pEventReader->setFlagDataAt(name, i)));
    }
  }
  for (int i = 0; i < maxNumOfElecHits; i++) {
    for (EventDataReader::TreeName name : {EventDataReader::ElecX,
                                           EventDataReader::ElecY,
                                           EventDataReader::ElecT}) {
      pEventChain->SetBranchAddress(
          EventDataReader::getTreeName(name, i).c_str(),
          &(pEventReader->setEventDataAt(name, i)));
    }
    {
      EventDataReader::TreeName name = EventDataReader::ElecFlag;
      pEventChain->SetBranchAddress(
          EventDataReader::getTreeName(name, i).c_str(),
          &(pEventReader->setFlagDataAt(name, i)));
    }
  }
  std::cout << "ok" << std::endl;

  // Make analysis tools, ions, and electrons
  pTools = new Analysis::AnalysisTools(kUnit, configReader);
  pIons = new Analysis::Objects(Objects::ions,
                                maxNumOfIonHits,
                                configReader,
                                "ions.");
  pElectrons = new Analysis::Objects(Objects::elecs,
                                     maxNumOfElecHits,
                                     configReader,
                                     "electrons.");
  pLogWriter->logAnalysisTools(kUnit, *pTools, *pIons, *pElectrons);

  // Open ROOT file
  std::cout << "open a root file... ";
  std::string rootFilename = pLogWriter->getFilename() + ".root";
  openRootFile(rootFilename.c_str(), "NEW");
  createHists();
  std::cout << "ok" << std::endl;

  // Initialization is done
  pLogWriter->write() << "Initialization is done." << std::endl;
  pLogWriter->write() << std::endl;
}

Analysis::AnalysisRun::~AnalysisRun() {
  // counter
  pLogWriter->write() << "Event count: " << pTools->getEventNumber()
                      << std::endl;

  // flush ROOT file
  flushRootFile();

  // finalization is done
  if (pElectrons) {
    delete pElectrons;
    pElectrons = nullptr;
  }
  if (pIons) {
    delete pIons;
    pIons = nullptr;
  }
  if (pTools) {
    delete pTools;
    pTools = nullptr;
  }
  if (pEventReader) {
    delete pEventReader;
    pEventReader = nullptr;
  }
  if (pEventChain) {
    delete pEventChain;
    pEventChain = nullptr;
  }

  pLogWriter->write() << "Finalization is done." << std::endl;
  pLogWriter->write() << std::endl;
  if (pLogWriter) {
    delete pLogWriter;
    pLogWriter = nullptr;
  }
}

void Analysis::AnalysisRun::processEvent(const long raw) {
  // Setup event chain
  pEventChain->GetEntry(raw);

  // Count event
  pTools->loadEventCounter();

  // make sure ion and electron data is empty, and reset resortElecFlags
  pIons->resetEventData();
  pElectrons->resetEventData();

  // input event data
  pTools->loadEventDataInputer(*pIons, *pEventReader);
  pTools->loadEventDataInputer(*pElectrons, *pEventReader);

  // resort option
  if (pIons->areAllFlag(ObjectFlag::MostOrSecondMostReliable)
      && pElectrons->areAllFlag(ObjectFlag::MostOrSecondMostReliable)) {

    pTools->loadMomentumCalculator(*pIons);
    pTools->loadMomentumCalculator(*pElectrons);
    fillHists();
  }
}

const long Analysis::AnalysisRun::getEntries() const {
  return (long) pEventChain->GetEntries();
}

void Analysis::AnalysisRun::createHists() {
  // IonImage
#define __IONIMAGE__ "IonImage"
#define __IONIMAGE1__ "Location X [mm]", "Location Y [mm]", 400, -50, 50, 400, -50, 50, __IONIMAGE__
#define __IONIMAGE2__ "Direction [degree]", "Location R [mm]", 360, -180, 180, 200, 0, 50, __IONIMAGE__
#define __CREATEIONIMAGE__(X) \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hImage_ ## X), __IONIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hImage_ ## X), __IONIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hImage_ ## X), __IONIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hImage_ ## X), __IONIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hImage_ ## X), __IONIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iCOMImage_ ## X), __IONIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hImageDirDist_ ## X), __IONIMAGE2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hImageDirDist_ ## X), __IONIMAGE2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hImageDirDist_ ## X), __IONIMAGE2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hImageDirDist_ ## X), __IONIMAGE2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hImageDirDist_ ## X), __IONIMAGE2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iCOMImageDirDist_ ## X), __IONIMAGE2__);
  __CREATEIONIMAGE__(always)
  __CREATEIONIMAGE__(iMaster)
  __CREATEIONIMAGE__(master)


  // IonTOF
#define __IONTOF__ "IonTOF"
#define __IONTOF1__ "TOF [ns]", 4000, 0, 20000, __IONTOF__
#define __IONTOF2__ "TOF [ns]", 8000, 0, 40000, __IONTOF__
#define __IONTOF3__ "TOF [ns]", "TOF [ns]", 1000, 0, 20000, 1000, 0, 20000, __IONTOF__
#define __IONTOF4__ "Sum of TOFs [ns]", "Diff of TOFs [ns]", 2000, 0, 40000, 1000, 0, 20000, __IONTOF__
#define __IONTOF5__ "TOF [ns]", "TOF [ns]", 2000, 0, 40000, 1000, 0, 20000, __IONTOF__
#define __CREATEIONTOF__(X) \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i1hTOF_ ## X), __IONTOF1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i2hTOF_ ## X), __IONTOF1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i3hTOF_ ## X), __IONTOF1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i4hTOF_ ## X), __IONTOF1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i5hTOF_ ## X), __IONTOF1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_iTotalTOF_ ## X), __IONTOF2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2hPIPICO_ ## X), __IONTOF3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3hPIPICO_ ## X), __IONTOF3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3h4hPIPICO_ ## X), __IONTOF3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4h5hPIPICO_ ## X), __IONTOF3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2hRotPIPICO_ ## X), __IONTOF4__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3hRotPIPICO_ ## X), __IONTOF4__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3h4hRotPIPICO_ ## X), __IONTOF4__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4h5hRotPIPICO_ ## X), __IONTOF4__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2h3h3PICO_ ## X), __IONTOF5__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3h4h3PICO_ ## X), __IONTOF5__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3h4h5h3PICO_ ## X), __IONTOF5__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2h3h4h4PICO_ ## X), __IONTOF5__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3h4h5h4PICO_ ## X), __IONTOF5__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2h3h4h5h5PICO_ ## X), __IONTOF5__);
  __CREATEIONTOF__(always)
  __CREATEIONTOF__(iMaster)
  __CREATEIONTOF__(master)

  // IonFISH
#define __IONFISH__ "IonFish"
#define __IONFISH1__ "TOF [ns]", "Location [mm]", 1000, 0, 20000, 400, -50, 50, __IONFISH__
#define __IONFISH2__ "TOF [ns]", "Location [mm]", 1000, 0, 20000, 200, 0, 50, __IONFISH__
#define __CREATEIONFISH__(X) \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hXFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hYFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hRFish_ ## X), __IONFISH2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hXFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hYFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hRFish_ ## X), __IONFISH2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hXFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hYFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hRFish_ ## X), __IONFISH2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hXFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hYFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hRFish_ ## X), __IONFISH2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hXFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hYFish_ ## X), __IONFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hRFish_ ## X), __IONFISH2__);
  __CREATEIONFISH__(always)
  __CREATEIONFISH__(iMaster)
  __CREATEIONFISH__(master)

  // IonMomentum
#define __IONMOMENTUM__ "IonMomentum"
#define __IONMOMENTUM1__ "Momentum [au]", 4000, -500, 500, __IONMOMENTUM__
#define __IONMOMENTUM2__ "Momentum [au]", "Momentum [au]", 800, -500, 500, 800, -500, 500, __IONMOMENTUM__
#define __IONMOMENTUM3__ "Momentum [au]", 2000, 0, 500, __IONMOMENTUM__
#define __IONMOMENTUM4__ "Momentum [au]", "Momentum [au]",  400, 0, 500, 400, 0, 500, __IONMOMENTUM__
#define __CREATEIONMOMENTUM__(X) \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i1hPX_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i1hPY_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i1hPZ_ ## X), __IONMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPXY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPYZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPZX_ ## X), __IONMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i1hP_ ## X), __IONMOMENTUM3__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i2hPX_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i2hPY_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i2hPZ_ ## X), __IONMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPXY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPYZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPZX_ ## X), __IONMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i2hP_ ## X), __IONMOMENTUM3__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i3hPX_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i3hPY_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i3hPZ_ ## X), __IONMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPXY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPYZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPZX_ ## X), __IONMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i3hP_ ## X), __IONMOMENTUM3__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i4hPX_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i4hPY_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i4hPZ_ ## X), __IONMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPXY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPYZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPZX_ ## X), __IONMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i4hP_ ## X), __IONMOMENTUM3__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i5hPX_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i5hPY_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i5hPZ_ ## X), __IONMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPXY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPYZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPZX_ ## X), __IONMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i5hP_ ## X), __IONMOMENTUM3__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_iTotalPX_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_iTotalPY_ ## X), __IONMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_iTotalPZ_ ## X), __IONMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPXY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPYZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPZX_ ## X), __IONMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_iTotalP_ ## X), __IONMOMENTUM3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2hPX_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2hPY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2hPZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2hP_ ## X), __IONMOMENTUM4__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3hPX_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3hPY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3hPZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3hP_ ## X), __IONMOMENTUM4__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3h4hPX_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3h4hPY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3h4hPZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3h4hP_ ## X), __IONMOMENTUM4__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4h5hPX_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4h5hPY_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4h5hPZ_ ## X), __IONMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4h5hP_ ## X), __IONMOMENTUM4__);
  __CREATEIONMOMENTUM__(always)
  __CREATEIONMOMENTUM__(iMaster)
  __CREATEIONMOMENTUM__(master)

  // IonMomentumAngDist 
#define __IONMOMENTUMANGDIST__ "IonMomentumAngDist"
#define __IONMOMENTUMANGDIST1__ "Cos Direction [degree]", "Momentum [au]", 400, -1, 1, 400, 0, 500, __IONMOMENTUMANGDIST__
#define __IONMOMENTUMANGDIST2__ "Direction [degree]", "Momentum [au]", 360, -180, 180, 400, 0, 500, __IONMOMENTUMANGDIST__
#define __IONMOMENTUMANGDIST3__ "Direction XY [degree]", "Cos Direction Z [1]", 360, -180, 180, 400, -1, 1, __IONMOMENTUMANGDIST__
#define __CREATEIONMOMENTUMANGDIST__(X) \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistX_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistY_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistZ_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistXY_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistYZ_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistZX_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistXY_PDirZIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistYZ_PDirXIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDistZX_PDirYIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1hPDirDist_ ## X), __IONMOMENTUMANGDIST3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistX_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistY_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistZ_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistXY_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistYZ_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistZX_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistXY_PDirZIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistYZ_PDirXIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDistZX_PDirYIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2hPDirDist_ ## X), __IONMOMENTUMANGDIST3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistX_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistY_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistZ_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistXY_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistYZ_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistZX_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistXY_PDirZIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistYZ_PDirXIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDistZX_PDirYIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3hPDirDist_ ## X), __IONMOMENTUMANGDIST3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistX_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistY_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistZ_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistXY_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistYZ_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistZX_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistXY_PDirZIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistYZ_PDirXIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDistZX_PDirYIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4hPDirDist_ ## X), __IONMOMENTUMANGDIST3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistX_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistY_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistZ_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistXY_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistYZ_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistZX_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistXY_PDirZIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistYZ_PDirXIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDistZX_PDirYIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i5hPDirDist_ ## X), __IONMOMENTUMANGDIST3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistX_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistY_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistZ_ ## X), __IONMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistXY_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistYZ_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistZX_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistXY_PDirZIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistYZ_PDirXIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDistZX_PDirYIs90_ ## X), __IONMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iTotalPDirDist_ ## X), __IONMOMENTUMANGDIST3__);
  __CREATEIONMOMENTUMANGDIST__(always)
  __CREATEIONMOMENTUMANGDIST__(iMaster)
  __CREATEIONMOMENTUMANGDIST__(master)
  
  // IonEnergy
#define __IONENERGY__ "IonEnergy"
#define __IONENERGY1__ "Energy [eV]", 1000, 0, 50, __IONENERGY__
#define __IONENERGY2__ "Energy [eV]", "Energy [eV]", 500, 0, 50, 500, 0, 50, __IONENERGY__
#define __CREATEIONENERGY__(X) \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i1hE_ ## X), __IONENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i2hE_ ## X), __IONENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i3hE_ ## X), __IONENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i4hE_ ## X), __IONENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_i5hE_ ## X), __IONENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_iTotalE_ ## X), __IONENERGY1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i1h2hE_ ## X), __IONENERGY2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3hE_ ## X), __IONENERGY2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i3h4hE_ ## X), __IONENERGY2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_i4h5hE_ ## X), __IONENERGY2__);
  __CREATEIONENERGY__(always)
  __CREATEIONENERGY__(iMaster)
  __CREATEIONENERGY__(master)

  // ElecImage
#define __ELECIMAGE__ "ElecImage"
#define __ELECIMAGE1__ "Location X [mm]", "Location Y [mm]", 600, -75, 75, 600, -75, 75,__ELECIMAGE__
#define __ELECIMAGE2__ "Direction [degree]", "Location R [mm]", 360, -180, 180, 300, 0, 75,__ELECIMAGE__
#define __CREATEELECIMAGE__(X) \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hImage_ ## X), __ELECIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hImage_ ## X), __ELECIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hImage_ ## X), __ELECIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hImage_ ## X), __ELECIMAGE1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hImageDirDist_ ## X), __ELECIMAGE2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hImageDirDist_ ## X), __ELECIMAGE2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hImageDirDist_ ## X), __ELECIMAGE2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hImageDirDist_ ## X), __ELECIMAGE2__); 
  __CREATEELECIMAGE__(always)
  __CREATEELECIMAGE__(eMaster)
  __CREATEELECIMAGE__(master)

  // ElecTOF
#define __ELECTOF__ "ElecTOF"
#define __ELECTOF1__ "TOF [ns]", 1000, 0, 100, __ELECTOF__
#define __ELECTOF2__ "TOF [ns]", "TOF [ns]", 400, 0, 100, 400, 0, 100, __ELECTOF__
#define __CREATEELECTOF__(X) \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e1hTOF_ ## X), __ELECTOF1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e2hTOF_ ## X), __ELECTOF1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e3hTOF_ ## X), __ELECTOF1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e4hTOF_ ## X), __ELECTOF1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1h2hPEPECO_ ## X), __ELECTOF2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2h3hPEPECO_ ## X), __ELECTOF2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3h4hPEPECO_ ## X), __ELECTOF2__); 
  __CREATEELECTOF__(always)
  __CREATEELECTOF__(eMaster)
  __CREATEELECTOF__(master)

  // ElecFish
#define __ELECFISH__ "ElecFish"
#define __ELECFISH1__ "TOF [ns]", "Location [mm]", 400, 0, 100, 600, -75, 75, __ELECFISH__
#define __ELECFISH2__ "TOF [ns]", "Location [mm]", 400, 0, 100, 300, 0, 75, __ELECFISH__
#define __CREATEELECFISH__(X) \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hXFish_ ## X), __ELECFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hYFish_ ## X), __ELECFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hRFish_ ## X), __ELECFISH2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hXFish_ ## X), __ELECFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hYFish_ ## X), __ELECFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hRFish_ ## X), __ELECFISH2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hXFish_ ## X), __ELECFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hYFish_ ## X), __ELECFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hRFish_ ## X), __ELECFISH2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hXFish_ ## X), __ELECFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hYFish_ ## X), __ELECFISH1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hRFish_ ## X), __ELECFISH2__);
  __CREATEELECFISH__(always)
  __CREATEELECFISH__(eMaster)
  __CREATEELECFISH__(master)

  // ElecMomentum
#define __ELECMOMENTUM__ "ElecMomentum"
#define __ELECMOMENTUM1__ "Momentum [au]", 4000, -4, 4, __ELECMOMENTUM__
#define __ELECMOMENTUM2__ "Momentum [au]", "Momentum [au]", 1000, -4, 4, 1000, -4, 4, __ELECMOMENTUM__
#define __ELECMOMENTUM3__ "Momentum [au]", 2000, 0, 4, __ELECMOMENTUM__
#define __CREATEELECMOMENTUM_(X) \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e1hPX_ ## X), __ELECMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e1hPY_ ## X), __ELECMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e1hPZ_ ## X), __ELECMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPXY_ ## X), __ELECMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPYZ_ ## X), __ELECMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPZX_ ## X), __ELECMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e1hP_ ## X), __ELECMOMENTUM3__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e2hPX_ ## X), __ELECMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e2hPY_ ## X), __ELECMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e2hPZ_ ## X), __ELECMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPXY_ ## X), __ELECMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPYZ_ ## X), __ELECMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPZX_ ## X), __ELECMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e2hP_ ## X), __ELECMOMENTUM3__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e3hPX_ ## X), __ELECMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e3hPY_ ## X), __ELECMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e3hPZ_ ## X), __ELECMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPXY_ ## X), __ELECMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPYZ_ ## X), __ELECMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPZX_ ## X), __ELECMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e3hP_ ## X), __ELECMOMENTUM3__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e4hPX_ ## X), __ELECMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e4hPY_ ## X), __ELECMOMENTUM1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e4hPZ_ ## X), __ELECMOMENTUM1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPXY_ ## X), __ELECMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPYZ_ ## X), __ELECMOMENTUM2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPZX_ ## X), __ELECMOMENTUM2__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e4hP_ ## X), __ELECMOMENTUM3__); 
  __CREATEELECMOMENTUM_(always)
  __CREATEELECMOMENTUM_(eMaster)
  __CREATEELECMOMENTUM_(master)

  // ElecMomentumAngDist 
#define __ELECMOMENTUMANGDIST__ "ElecMomentumAngDist"
#define __ELECMOMENTUMANGDIST1__ "Cos Direction [degree]", "Momentum [au]", 400, -1, 1, 400, 0, 4, __ELECMOMENTUMANGDIST__
#define __ELECMOMENTUMANGDIST2__ "Direction [degree]", "Momentum [au]", 360, -180, 180, 500, 0, 4, __ELECMOMENTUMANGDIST__
#define __ELECMOMENTUMANGDIST3__ "Direction XY [degree]", "Direction Z [1]", 360, -180, 180, 400, -1, 1, __ELECMOMENTUMANGDIST__
#define __CREATEELECMOMENTUMANGDIST__(X) \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistX_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistY_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistZ_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistXY_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistYZ_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistZX_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistXY_PDirZIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistYZ_PDirXIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDistZX_PDirYIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hPDirDist_ ## X), __ELECMOMENTUMANGDIST3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistX_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistY_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistZ_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistXY_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistYZ_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistZX_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistXY_PDirZIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistYZ_PDirXIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDistZX_PDirYIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2hPDirDist_ ## X), __ELECMOMENTUMANGDIST3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistX_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistY_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistZ_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistXY_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistYZ_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistZX_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistXY_PDirZIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistYZ_PDirXIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDistZX_PDirYIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3hPDirDist_ ## X), __ELECMOMENTUMANGDIST3__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistX_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistY_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistZ_ ## X), __ELECMOMENTUMANGDIST1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistXY_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistYZ_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistZX_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistXY_PDirZIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistYZ_PDirXIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDistZX_PDirYIs90_ ## X), __ELECMOMENTUMANGDIST2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e4hPDirDist_ ## X), __ELECMOMENTUMANGDIST3__);
  __CREATEELECMOMENTUMANGDIST__(always)
  __CREATEELECMOMENTUMANGDIST__(eMaster)
  __CREATEELECMOMENTUMANGDIST__(master)

  // ElecEnergy
#define __ELECENERGY__ "ElecEnergy"
#define __ELECENERGY1__ "Energy [eV]", 1000, 0, 50, __ELECENERGY__
#define __ELECENERGY2__ "Energy [eV]", "Energy [eV]", 500, 0, 50, 500, 0, 50, __ELECENERGY__
#define __CREATEELECENERGY__(X) \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e1hE_ ## X), __ELECENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e2hE_ ## X), __ELECENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e3hE_ ## X), __ELECENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_e4hE_ ## X), __ELECENERGY1__); \
  create1d(SAME_TITLE_WITH_VALNAME(h1_eTotalE_ ## X), __ELECENERGY1__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1h2hE_ ## X), __ELECENERGY2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e2h3hE_ ## X), __ELECENERGY2__); \
  create2d(SAME_TITLE_WITH_VALNAME(h2_e3h4hE_ ## X), __ELECENERGY2__);
  __CREATEELECENERGY__(always)
  __CREATEELECENERGY__(eMaster)
  __CREATEELECENERGY__(master)

// IonElecCorr
#define __IONELECCORR__ "KER [eV]", "Electron Kinetic Energy [eV]", 400, 0, 100, 500, 0, 50, "IonElecCorr"
#define __CREATEIONELECCORR__(X) \
  create2d(SAME_TITLE_WITH_VALNAME(h2_iKER_e1hE_ ## X), __IONELECCORR__);
  __CREATEIONELECCORR__(always)
  __CREATEIONELECCORR__(master)

  // Others
#define __OTHERS__ "Others"
  create1d(SAME_TITLE_WITH_VALNAME(h1_i1hTOF_i2h3hNotDead),
           "TOF [ns]", 2000, 0, 10000, __OTHERS__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_i2h3hPIPICO_i1hMaster),
           "TOF 1 [ns]", "TOF 2 [ns]",
           500, 0, 10000, 500, 0, 10000,
           __OTHERS__);
  create2d(SAME_TITLE_WITH_VALNAME(h2_e1hE_iTotalTOF_master),
           "Energy [eV]", "Sum of TOFs [ns]",
           500, 0, 50, 1000, 0, 20000,
           __OTHERS__);
}

void Analysis::AnalysisRun::fillHists() {
  const bool always = true;

  const bool iDead1 = pIons->getRealOrDummyObject(0).isFlag(ObjectFlag::Dead);
  const bool iDead2 = pIons->getRealOrDummyObject(1).isFlag(ObjectFlag::Dead);
  const bool iDead3 = pIons->getRealOrDummyObject(2).isFlag(ObjectFlag::Dead);
  const bool iDead4 = pIons->getRealOrDummyObject(3).isFlag(ObjectFlag::Dead);
  const bool iDead5 = pIons->getRealOrDummyObject(4).isFlag(ObjectFlag::Dead);
  const bool eDead1 = pElectrons->getRealOrDummyObject(0).isFlag(ObjectFlag::Dead);
  const bool eDead2 = pElectrons->getRealOrDummyObject(1).isFlag(ObjectFlag::Dead);
  const bool eDead3 = pElectrons->getRealOrDummyObject(2).isFlag(ObjectFlag::Dead);
  const bool eDead4 = pElectrons->getRealOrDummyObject(3).isFlag(ObjectFlag::Dead);

  const bool iMaster1 = pIons->getRealOrDummyObject(0).isFlag(ObjectFlag::WithinMasterRegion);
  const bool iMaster2 = pIons->getRealOrDummyObject(1).isFlag(ObjectFlag::WithinMasterRegion);
  const bool iMaster3 = pIons->getRealOrDummyObject(2).isFlag(ObjectFlag::WithinMasterRegion);
  const bool iMaster4 = pIons->getRealOrDummyObject(3).isFlag(ObjectFlag::WithinMasterRegion);
  const bool iMaster5 = pIons->getRealOrDummyObject(4).isFlag(ObjectFlag::WithinMasterRegion);
  const bool iMaster = pIons->areAllFlag(ObjectFlag::WithinMasterRegion);
  const bool eMaster1 = pElectrons->getRealOrDummyObject(0).isFlag(ObjectFlag::WithinMasterRegion);
  const bool eMaster2 = pElectrons->getRealOrDummyObject(1).isFlag(ObjectFlag::WithinMasterRegion);
  const bool eMaster3 = pElectrons->getRealOrDummyObject(2).isFlag(ObjectFlag::WithinMasterRegion);
  const bool eMaster4 = pElectrons->getRealOrDummyObject(3).isFlag(ObjectFlag::WithinMasterRegion);
  const bool eMaster = pElectrons->areAllFlag(ObjectFlag::WithinMasterRegion);
  const bool master = iMaster && eMaster;

  auto iX1 = pIons->getRealOrDummyObject(0).outputLocX();
  auto iY1 = pIons->getRealOrDummyObject(0).outputLocY();
  auto iR1 = pIons->getRealOrDummyObject(0).outputLocR();
  auto iDir1 = pIons->getRealOrDummyObject(0).outputLocDir();
  auto iT1 = pIons->getRealOrDummyObject(0).outputTOF();
  auto iX2 = pIons->getRealOrDummyObject(1).outputLocX();
  auto iY2 = pIons->getRealOrDummyObject(1).outputLocY();
  auto iDir2 = pIons->getRealOrDummyObject(1).outputLocDir();
  auto iR2 = pIons->getRealOrDummyObject(1).outputLocR();
  auto iT2 = pIons->getRealOrDummyObject(1).outputTOF();
  auto iX3 = pIons->getRealOrDummyObject(2).outputLocX();
  auto iY3 = pIons->getRealOrDummyObject(2).outputLocY();
  auto iDir3 = pIons->getRealOrDummyObject(2).outputLocDir();
  auto iR3 = pIons->getRealOrDummyObject(2).outputLocR();
  auto iT3 = pIons->getRealOrDummyObject(2).outputTOF();
  auto iX4 = pIons->getRealOrDummyObject(3).outputLocX();
  auto iY4 = pIons->getRealOrDummyObject(3).outputLocY();
  auto iDir4 = pIons->getRealOrDummyObject(3).outputLocDir();
  auto iR4 = pIons->getRealOrDummyObject(3).outputLocR();
  auto iT4 = pIons->getRealOrDummyObject(3).outputTOF();
  auto iX5 = pIons->getRealOrDummyObject(4).outputLocX();
  auto iY5 = pIons->getRealOrDummyObject(4).outputLocY();
  auto iDir5 = pIons->getRealOrDummyObject(4).outputLocDir();
  auto iR5 = pIons->getRealOrDummyObject(4).outputLocR();
  auto iT5 = pIons->getRealOrDummyObject(4).outputTOF();

  auto iCOMX = pIons->outputCOMLocX();
  auto iCOMY = pIons->outputCOMLocY();
  auto iCOMR = pIons->outputCOMLocR();
  auto iCOMDir = pIons->outputCOMLocDir();
  auto iTTotal = pIons->outputTotalTOF();
  auto iT12 = pIons->outputSumOf2TOFs(0, 1);
  auto iT23 = pIons->outputSumOf2TOFs(1, 2);
  auto iT34 = pIons->outputSumOf2TOFs(2, 3);
  auto iT45 = pIons->outputSumOf2TOFs(3, 4);
  auto iTDiff12 = pIons->outputDiffOfTOFs(0, 1);
  auto iTDiff23 = pIons->outputDiffOfTOFs(1, 2);
  auto iTDiff34 = pIons->outputDiffOfTOFs(2, 3);
  auto iTDiff45 = pIons->outputDiffOfTOFs(3, 4);

  auto eX1 = pElectrons->getRealOrDummyObject(0).outputLocX();
  auto eY1 = pElectrons->getRealOrDummyObject(0).outputLocY();
  auto eR1 = pElectrons->getRealOrDummyObject(0).outputLocR();
  auto eDir1 = pElectrons->getRealOrDummyObject(0).outputLocDir();
  auto eT1 = pElectrons->getRealOrDummyObject(0).outputTOF();
  auto eX2 = pElectrons->getRealOrDummyObject(1).outputLocX();
  auto eY2 = pElectrons->getRealOrDummyObject(1).outputLocY();
  auto eR2 = pElectrons->getRealOrDummyObject(1).outputLocR();
  auto eDir2 = pElectrons->getRealOrDummyObject(1).outputLocDir();
  auto eT2 = pElectrons->getRealOrDummyObject(1).outputTOF();
  auto eX3 = pElectrons->getRealOrDummyObject(2).outputLocX();
  auto eY3 = pElectrons->getRealOrDummyObject(2).outputLocY();
  auto eR3 = pElectrons->getRealOrDummyObject(2).outputLocR();
  auto eDir3 = pElectrons->getRealOrDummyObject(2).outputLocDir();
  auto eT3 = pElectrons->getRealOrDummyObject(2).outputTOF();
  auto eX4 = pElectrons->getRealOrDummyObject(3).outputLocX();
  auto eY4 = pElectrons->getRealOrDummyObject(3).outputLocY();
  auto eR4 = pElectrons->getRealOrDummyObject(3).outputLocR();
  auto eDir4 = pElectrons->getRealOrDummyObject(3).outputLocDir();
  auto eT4 = pElectrons->getRealOrDummyObject(3).outputTOF();

  auto eCOMX = pElectrons->outputCOMLocX();
  auto eCOMY = pElectrons->outputCOMLocY();
  auto eT12 = pElectrons->outputSumOf2TOFs(0, 1);
  auto eT23 = pElectrons->outputSumOf2TOFs(1, 2);
  auto eT34 = pElectrons->outputSumOf2TOFs(2, 3);
  auto eT45 = pElectrons->outputSumOf2TOFs(3, 4);
  auto eTDiff12 = pElectrons->outputDiffOfTOFs(0, 1);
  auto eTDiff23 = pElectrons->outputDiffOfTOFs(1, 2);
  auto eTDiff34 = pElectrons->outputDiffOfTOFs(2, 3);
  auto eTDiff45 = pElectrons->outputDiffOfTOFs(3, 4);

  auto iPX1 = pIons->getRealOrDummyObject(0).outputPX();
  auto iPY1 = pIons->getRealOrDummyObject(0).outputPY();
  auto iPZ1 = pIons->getRealOrDummyObject(0).outputPZ();
  auto iPXY1 = pIons->getRealOrDummyObject(0).outputPXY();
  auto iPYZ1 = pIons->getRealOrDummyObject(0).outputPYZ();
  auto iPZX1 = pIons->getRealOrDummyObject(0).outputPZX();
  auto iP1 = pIons->getRealOrDummyObject(0).outputP();
  auto iPDirX1 = pIons->getRealOrDummyObject(0).outputPDirX();
  auto iPDirY1 = pIons->getRealOrDummyObject(0).outputPDirY();
  auto iPDirZ1 = pIons->getRealOrDummyObject(0).outputPDirZ();
  auto iPDirXY1 = pIons->getRealOrDummyObject(0).outputPDirXY();
  auto iPDirYZ1 = pIons->getRealOrDummyObject(0).outputPDirYZ();
  auto iPDirZX1 = pIons->getRealOrDummyObject(0).outputPDirZX();
  auto iCosPDirX1 = pIons->getRealOrDummyObject(0).outputCosPDirX();
  auto iCosPDirY1 = pIons->getRealOrDummyObject(0).outputCosPDirY();
  auto iCosPDirZ1 = pIons->getRealOrDummyObject(0).outputCosPDirZ();
  const bool iPDirX1Is90 = iPDirX1 ? (85 <= *iPDirX1 && *iPDirX1 <= 95) : false;
  const bool iPDirY1Is90 = iPDirY1 ? (85 <= *iPDirY1 && *iPDirY1 <= 95) : false;
  const bool iPDirZ1Is90 = iPDirZ1 ? (85 <= *iPDirZ1 && *iPDirZ1 <= 95) : false;

  auto iPX2 = pIons->getRealOrDummyObject(1).outputPX();
  auto iPY2 = pIons->getRealOrDummyObject(1).outputPY();
  auto iPZ2 = pIons->getRealOrDummyObject(1).outputPZ();
  auto iPXY2 = pIons->getRealOrDummyObject(1).outputPXY();
  auto iPYZ2 = pIons->getRealOrDummyObject(1).outputPYZ();
  auto iPZX2 = pIons->getRealOrDummyObject(1).outputPZX();
  auto iP2 = pIons->getRealOrDummyObject(1).outputP();
  auto iPDirX2 = pIons->getRealOrDummyObject(1).outputPDirX();
  auto iPDirY2 = pIons->getRealOrDummyObject(1).outputPDirY();
  auto iPDirZ2 = pIons->getRealOrDummyObject(1).outputPDirZ();
  auto iPDirXY2 = pIons->getRealOrDummyObject(1).outputPDirXY();
  auto iPDirYZ2 = pIons->getRealOrDummyObject(1).outputPDirYZ();
  auto iPDirZX2 = pIons->getRealOrDummyObject(1).outputPDirZX();
  auto iCosPDirX2 = pIons->getRealOrDummyObject(1).outputCosPDirX();
  auto iCosPDirY2 = pIons->getRealOrDummyObject(1).outputCosPDirY();
  auto iCosPDirZ2 = pIons->getRealOrDummyObject(1).outputCosPDirZ();
  const bool iPDirX2Is90 = iPDirX2 ? (85 <= *iPDirX2 && *iPDirX2 <= 95) : false;
  const bool iPDirY2Is90 = iPDirY2 ? (85 <= *iPDirY2 && *iPDirY2 <= 95) : false;
  const bool iPDirZ2Is90 = iPDirZ2 ? (85 <= *iPDirZ2 && *iPDirZ2 <= 95) : false;

  auto iPX3 = pIons->getRealOrDummyObject(2).outputPX();
  auto iPY3 = pIons->getRealOrDummyObject(2).outputPY();
  auto iPZ3 = pIons->getRealOrDummyObject(2).outputPZ();
  auto iPXY3 = pIons->getRealOrDummyObject(2).outputPXY();
  auto iPYZ3 = pIons->getRealOrDummyObject(2).outputPYZ();
  auto iPZX3 = pIons->getRealOrDummyObject(2).outputPZX();
  auto iP3 = pIons->getRealOrDummyObject(2).outputP();
  auto iPDirX3 = pIons->getRealOrDummyObject(2).outputPDirX();
  auto iPDirY3 = pIons->getRealOrDummyObject(2).outputPDirY();
  auto iPDirZ3 = pIons->getRealOrDummyObject(2).outputPDirZ();
  auto iPDirXY3 = pIons->getRealOrDummyObject(2).outputPDirXY();
  auto iPDirYZ3 = pIons->getRealOrDummyObject(2).outputPDirYZ();
  auto iPDirZX3 = pIons->getRealOrDummyObject(2).outputPDirZX();
  auto iCosPDirX3 = pIons->getRealOrDummyObject(2).outputCosPDirX();
  auto iCosPDirY3 = pIons->getRealOrDummyObject(2).outputCosPDirY();
  auto iCosPDirZ3 = pIons->getRealOrDummyObject(2).outputCosPDirZ();
  const bool iPDirX3Is90 = iPDirX3 ? (85 <= *iPDirX3 && *iPDirX3 <= 95) : false;
  const bool iPDirY3Is90 = iPDirY3 ? (85 <= *iPDirY3 && *iPDirY3 <= 95) : false;
  const bool iPDirZ3Is90 = iPDirZ3 ? (85 <= *iPDirZ3 && *iPDirZ3 <= 95) : false;

  auto iPX4 = pIons->getRealOrDummyObject(3).outputPX();
  auto iPY4 = pIons->getRealOrDummyObject(3).outputPY();
  auto iPZ4 = pIons->getRealOrDummyObject(3).outputPZ();
  auto iPXY4 = pIons->getRealOrDummyObject(3).outputPXY();
  auto iPYZ4 = pIons->getRealOrDummyObject(3).outputPYZ();
  auto iPZX4 = pIons->getRealOrDummyObject(3).outputPZX();
  auto iP4 = pIons->getRealOrDummyObject(3).outputP();
  auto iPDirX4 = pIons->getRealOrDummyObject(3).outputPDirX();
  auto iPDirY4 = pIons->getRealOrDummyObject(3).outputPDirY();
  auto iPDirZ4 = pIons->getRealOrDummyObject(3).outputPDirZ();
  auto iPDirXY4 = pIons->getRealOrDummyObject(3).outputPDirXY();
  auto iPDirYZ4 = pIons->getRealOrDummyObject(3).outputPDirYZ();
  auto iPDirZX4 = pIons->getRealOrDummyObject(3).outputPDirZX();
  auto iCosPDirX4 = pIons->getRealOrDummyObject(3).outputCosPDirX();
  auto iCosPDirY4 = pIons->getRealOrDummyObject(3).outputCosPDirY();
  auto iCosPDirZ4 = pIons->getRealOrDummyObject(3).outputCosPDirZ();
  const bool iPDirX4Is90 = iPDirX4 ? (85 <= *iPDirX4 && *iPDirX4 <= 95) : false;
  const bool iPDirY4Is90 = iPDirY4 ? (85 <= *iPDirY4 && *iPDirY4 <= 95) : false;
  const bool iPDirZ4Is90 = iPDirZ4 ? (85 <= *iPDirZ4 && *iPDirZ4 <= 95) : false;

  auto iPX5 = pIons->getRealOrDummyObject(4).outputPX();
  auto iPY5 = pIons->getRealOrDummyObject(4).outputPY();
  auto iPZ5 = pIons->getRealOrDummyObject(4).outputPZ();
  auto iPXY5 = pIons->getRealOrDummyObject(4).outputPXY();
  auto iPYZ5 = pIons->getRealOrDummyObject(4).outputPYZ();
  auto iPZX5 = pIons->getRealOrDummyObject(4).outputPZX();
  auto iP5 = pIons->getRealOrDummyObject(4).outputP();
  auto iPDirX5 = pIons->getRealOrDummyObject(4).outputPDirX();
  auto iPDirY5 = pIons->getRealOrDummyObject(4).outputPDirY();
  auto iPDirZ5 = pIons->getRealOrDummyObject(4).outputPDirZ();
  auto iPDirXY5 = pIons->getRealOrDummyObject(4).outputPDirXY();
  auto iPDirYZ5 = pIons->getRealOrDummyObject(4).outputPDirYZ();
  auto iPDirZX5 = pIons->getRealOrDummyObject(4).outputPDirZX();
  auto iCosPDirX5 = pIons->getRealOrDummyObject(4).outputCosPDirX();
  auto iCosPDirY5 = pIons->getRealOrDummyObject(4).outputCosPDirY();
  auto iCosPDirZ5 = pIons->getRealOrDummyObject(4).outputCosPDirZ();
  const bool iPDirX5Is90 = iPDirX5 ? (85 <= *iPDirX5 && *iPDirX5 <= 95) : false;
  const bool iPDirY5Is90 = iPDirY5 ? (85 <= *iPDirY5 && *iPDirY5 <= 95) : false;
  const bool iPDirZ5Is90 = iPDirZ5 ? (85 <= *iPDirZ5 && *iPDirZ5 <= 95) : false;

  auto iPXTotal = pIons->outputPX();
  auto iPYTotal = pIons->outputPY();
  auto iPZTotal = pIons->outputPZ();
  auto iPXYTotal = pIons->outputPXY();
  auto iPYZTotal = pIons->outputPYZ();
  auto iPZXTotal = pIons->outputPZX();
  auto iPTotal = pIons->outputP();
  auto iPDirXTotal = pIons->outputPDirX();
  auto iPDirYTotal = pIons->outputPDirY();
  auto iPDirZTotal = pIons->outputPDirZ();
  auto iPDirXYTotal = pIons->outputPDirXY();
  auto iPDirYZTotal = pIons->outputPDirYZ();
  auto iPDirZXTotal = pIons->outputPDirZX();
  auto iCosPDirXTotal = pIons->outputCosPDirX();
  auto iCosPDirYTotal = pIons->outputCosPDirY();
  auto iCosPDirZTotal = pIons->outputCosPDirZ();
  const bool iPDirXTotalIs90 = iPDirXTotal ? (85 <= *iPDirXTotal && *iPDirXTotal <= 95) : false;
  const bool iPDirYTotalIs90 = iPDirYTotal ? (85 <= *iPDirYTotal && *iPDirYTotal <= 95) : false;
  const bool iPDirZTotalIs90 = iPDirZTotal ? (85 <= *iPDirZTotal && *iPDirZTotal <= 95) : false;

  auto ePX1 = pElectrons->getRealOrDummyObject(0).outputPX();
  auto ePY1 = pElectrons->getRealOrDummyObject(0).outputPY();
  auto ePZ1 = pElectrons->getRealOrDummyObject(0).outputPZ();
  auto ePXY1 = pElectrons->getRealOrDummyObject(0).outputPXY();
  auto ePYZ1 = pElectrons->getRealOrDummyObject(0).outputPYZ();
  auto ePZX1 = pElectrons->getRealOrDummyObject(0).outputPZX();
  auto eP1 = pElectrons->getRealOrDummyObject(0).outputP();
  auto ePDirX1 = pElectrons->getRealOrDummyObject(0).outputPDirX();
  auto ePDirY1 = pElectrons->getRealOrDummyObject(0).outputPDirY();
  auto ePDirZ1 = pElectrons->getRealOrDummyObject(0).outputPDirZ();
  auto ePDirXY1 = pElectrons->getRealOrDummyObject(0).outputPDirXY();
  auto ePDirYZ1 = pElectrons->getRealOrDummyObject(0).outputPDirYZ();
  auto ePDirZX1 = pElectrons->getRealOrDummyObject(0).outputPDirZX();
  auto eCosPDirX1 = pElectrons->getRealOrDummyObject(0).outputCosPDirX();
  auto eCosPDirY1 = pElectrons->getRealOrDummyObject(0).outputCosPDirY();
  auto eCosPDirZ1 = pElectrons->getRealOrDummyObject(0).outputCosPDirZ();
  const bool ePDirX1Is90 = ePDirX1 ? (85 <= *ePDirX1 && *ePDirX1 <= 95) : false;
  const bool ePDirY1Is90 = ePDirY1 ? (85 <= *ePDirY1 && *ePDirY1 <= 95) : false;
  const bool ePDirZ1Is90 = ePDirZ1 ? (85 <= *ePDirZ1 && *ePDirZ1 <= 95) : false;

  auto ePX2 = pElectrons->getRealOrDummyObject(1).outputPX();
  auto ePY2 = pElectrons->getRealOrDummyObject(1).outputPY();
  auto ePZ2 = pElectrons->getRealOrDummyObject(1).outputPZ();
  auto ePDirX2 = pElectrons->getRealOrDummyObject(1).outputPDirX();
  auto ePDirY2 = pElectrons->getRealOrDummyObject(1).outputPDirY();
  auto ePDirZ2 = pElectrons->getRealOrDummyObject(1).outputPDirZ();
  auto ePXY2 = pElectrons->getRealOrDummyObject(1).outputPXY();
  auto ePYZ2 = pElectrons->getRealOrDummyObject(1).outputPYZ();
  auto ePZX2 = pElectrons->getRealOrDummyObject(1).outputPZX();
  auto eP2 = pElectrons->getRealOrDummyObject(1).outputP();
  auto ePDirXY2 = pElectrons->getRealOrDummyObject(1).outputPDirXY();
  auto ePDirYZ2 = pElectrons->getRealOrDummyObject(1).outputPDirYZ();
  auto ePDirZX2 = pElectrons->getRealOrDummyObject(1).outputPDirZX();
  auto eCosPDirX2 = pElectrons->getRealOrDummyObject(1).outputCosPDirX();
  auto eCosPDirY2 = pElectrons->getRealOrDummyObject(1).outputCosPDirY();
  auto eCosPDirZ2 = pElectrons->getRealOrDummyObject(1).outputCosPDirZ();
  const bool ePDirX2Is90 = ePDirX2 ? (85 <= *ePDirX2 && *ePDirX2 <= 95) : false;
  const bool ePDirY2Is90 = ePDirY2 ? (85 <= *ePDirY2 && *ePDirY2 <= 95) : false;
  const bool ePDirZ2Is90 = ePDirZ2 ? (85 <= *ePDirZ2 && *ePDirZ2 <= 95) : false;

  auto ePX3 = pElectrons->getRealOrDummyObject(2).outputPX();
  auto ePY3 = pElectrons->getRealOrDummyObject(2).outputPY();
  auto ePZ3 = pElectrons->getRealOrDummyObject(2).outputPZ();
  auto ePXY3 = pElectrons->getRealOrDummyObject(2).outputPXY();
  auto ePYZ3 = pElectrons->getRealOrDummyObject(2).outputPYZ();
  auto ePZX3 = pElectrons->getRealOrDummyObject(2).outputPZX();
  auto eP3 = pElectrons->getRealOrDummyObject(2).outputP();
  auto ePDirX3 = pElectrons->getRealOrDummyObject(2).outputPDirX();
  auto ePDirY3 = pElectrons->getRealOrDummyObject(2).outputPDirY();
  auto ePDirZ3 = pElectrons->getRealOrDummyObject(2).outputPDirZ();
  auto ePDirXY3 = pElectrons->getRealOrDummyObject(2).outputPDirXY();
  auto ePDirYZ3 = pElectrons->getRealOrDummyObject(2).outputPDirYZ();
  auto ePDirZX3 = pElectrons->getRealOrDummyObject(2).outputPDirZX();
  auto eCosPDirX3 = pElectrons->getRealOrDummyObject(2).outputCosPDirX();
  auto eCosPDirY3 = pElectrons->getRealOrDummyObject(2).outputCosPDirY();
  auto eCosPDirZ3 = pElectrons->getRealOrDummyObject(2).outputCosPDirZ();
  const bool ePDirX3Is90 = ePDirX3 ? (85 <= *ePDirX3 && *ePDirX3 <= 95) : false;
  const bool ePDirY3Is90 = ePDirY3 ? (85 <= *ePDirY3 && *ePDirY3 <= 95) : false;
  const bool ePDirZ3Is90 = ePDirZ3 ? (85 <= *ePDirZ3 && *ePDirZ3 <= 95) : false;

  auto ePX4 = pElectrons->getRealOrDummyObject(3).outputPX();
  auto ePY4 = pElectrons->getRealOrDummyObject(3).outputPY();
  auto ePZ4 = pElectrons->getRealOrDummyObject(3).outputPZ();
  auto ePXY4 = pElectrons->getRealOrDummyObject(3).outputPXY();
  auto ePYZ4 = pElectrons->getRealOrDummyObject(3).outputPYZ();
  auto ePZX4 = pElectrons->getRealOrDummyObject(3).outputPZX();
  auto eP4 = pElectrons->getRealOrDummyObject(3).outputP();
  auto ePDirX4 = pElectrons->getRealOrDummyObject(3).outputPDirX();
  auto ePDirY4 = pElectrons->getRealOrDummyObject(3).outputPDirY();
  auto ePDirZ4 = pElectrons->getRealOrDummyObject(3).outputPDirZ();
  auto ePDirXY4 = pElectrons->getRealOrDummyObject(3).outputPDirXY();
  auto ePDirYZ4 = pElectrons->getRealOrDummyObject(3).outputPDirYZ();
  auto ePDirZX4 = pElectrons->getRealOrDummyObject(3).outputPDirZX();
  auto eCosPDirX4 = pElectrons->getRealOrDummyObject(3).outputCosPDirX();
  auto eCosPDirY4 = pElectrons->getRealOrDummyObject(3).outputCosPDirY();
  auto eCosPDirZ4 = pElectrons->getRealOrDummyObject(3).outputCosPDirZ();
  const bool ePDirX4Is90 = ePDirX4 ? (85 <= *ePDirX4 && *ePDirX4 <= 95) : false;
  const bool ePDirY4Is90 = ePDirY4 ? (85 <= *ePDirY4 && *ePDirY4 <= 95) : false;
  const bool ePDirZ4Is90 = ePDirZ4 ? (85 <= *ePDirZ4 && *ePDirZ4 <= 95) : false;

  auto iE1 = pIons->getRealOrDummyObject(0).outputE();
  auto iE2 = pIons->getRealOrDummyObject(1).outputE();
  auto iE3 = pIons->getRealOrDummyObject(2).outputE();
  auto iE4 = pIons->getRealOrDummyObject(3).outputE();
  auto iE5 = pIons->getRealOrDummyObject(4).outputE();
  auto iETotal = pIons->outputE();

  auto eE1 = pElectrons->getRealOrDummyObject(0).outputE();
  auto eE2 = pElectrons->getRealOrDummyObject(1).outputE();
  auto eE3 = pElectrons->getRealOrDummyObject(2).outputE();
  auto eE4 = pElectrons->getRealOrDummyObject(3).outputE();
  auto eETotal = pElectrons->outputE();

  // functions
  auto sum2doubles = [](const auto d0, const auto d1)->std::shared_ptr<double> {
    if(d0 && d1) return std::make_shared<double>(*d0 + *d1);
    else return nullptr;
  };
  auto sumDoubles = [=](const int n, auto const ds[])->std::shared_ptr<double> {
    return std::accumulate(ds, ds+n, std::make_shared<double>(0), sum2doubles);
  };

  // IonImage
#define __FILLIONIMAGE__(X) \
  if (X) { \
  fill2d(h2_i1hImage_ ## X, iX1, iY1); \
  fill2d(h2_i2hImage_ ## X, iX2, iY2); \
  fill2d(h2_i3hImage_ ## X, iX3, iY3); \
  fill2d(h2_i4hImage_ ## X, iX4, iY4); \
  fill2d(h2_i5hImage_ ## X, iX5, iY5); \
  fill2d(h2_iCOMImage_ ## X, iCOMX, iCOMY); \
  fill2d(h2_i1hImageDirDist_ ## X, iDir1, iR1); \
  fill2d(h2_i2hImageDirDist_ ## X, iDir2, iR2); \
  fill2d(h2_i3hImageDirDist_ ## X, iDir3, iR3); \
  fill2d(h2_i4hImageDirDist_ ## X, iDir4, iR4); \
  fill2d(h2_i5hImageDirDist_ ## X, iDir5, iR5); \
  fill2d(h2_iCOMImageDirDist_ ## X, iCOMDir, iCOMR); \
}
  __FILLIONIMAGE__(always)
  __FILLIONIMAGE__(iMaster)
  __FILLIONIMAGE__(master)

  // IonTOF
#define __FILLIONTOF__(X) \
  if (X) { \
  fill1d(h1_i1hTOF_ ## X, iT1); \
  fill1d(h1_i2hTOF_ ## X, iT2); \
  fill1d(h1_i3hTOF_ ## X, iT3); \
  fill1d(h1_i4hTOF_ ## X, iT4); \
  fill1d(h1_i5hTOF_ ## X, iT5); \
  fill1d(h1_iTotalTOF_ ## X, iTTotal); \
  fill2d(h2_i1h2hPIPICO_ ## X, iT1, iT2); \
  fill2d(h2_i2h3hPIPICO_ ## X, iT2, iT3); \
  fill2d(h2_i3h4hPIPICO_ ## X, iT3, iT4); \
  fill2d(h2_i4h5hPIPICO_ ## X, iT4, iT5); \
  fill2d(h2_i1h2hRotPIPICO_ ## X, iT12, iTDiff12); \
  fill2d(h2_i2h3hRotPIPICO_ ## X, iT23, iTDiff23); \
  fill2d(h2_i3h4hRotPIPICO_ ## X, iT34, iTDiff34); \
  fill2d(h2_i4h5hRotPIPICO_ ## X, iT45, iTDiff45); \
  std::shared_ptr<double> iT12[] = {iT1, iT2}; \
  std::shared_ptr<double> iT23[] = {iT2, iT3}; \
  std::shared_ptr<double> iT34[] = {iT3, iT4}; \
  std::shared_ptr<double> iT123[] = {iT1, iT2, iT3}; \
  std::shared_ptr<double> iT234[] = {iT2, iT3, iT4}; \
  std::shared_ptr<double> iT1234[] = {iT1, iT2, iT3, iT4}; \
  fill2d(h2_i1h2h3h3PICO_ ## X, sumDoubles(2, iT12), iT3); \
  fill2d(h2_i2h3h4h3PICO_ ## X, sumDoubles(2, iT23), iT4); \
  fill2d(h2_i3h4h5h3PICO_ ## X, sumDoubles(2, iT34), iT5); \
  fill2d(h2_i1h2h3h4h4PICO_ ## X, sumDoubles(3, iT123), iT4); \
  fill2d(h2_i2h3h4h5h4PICO_ ## X, sumDoubles(3, iT234), iT5); \
  fill2d(h2_i1h2h3h4h5h5PICO_ ## X, sumDoubles(4, iT1234), iT5); \
}
  __FILLIONTOF__(always)
  __FILLIONTOF__(iMaster)
  __FILLIONTOF__(master)

// IonFish
#define __FILLIONFISH__(X) \
  if (X) { \
  fill2d(h2_i1hXFish_ ## X, iT1, iX1); \
  fill2d(h2_i1hYFish_ ## X, iT1, iY1); \
  fill2d(h2_i1hRFish_ ## X, iT1, iR1); \
  fill2d(h2_i2hXFish_ ## X, iT2, iX2); \
  fill2d(h2_i2hYFish_ ## X, iT2, iY2); \
  fill2d(h2_i2hRFish_ ## X, iT2, iR2); \
  fill2d(h2_i3hXFish_ ## X, iT3, iX3); \
  fill2d(h2_i3hYFish_ ## X, iT3, iY3); \
  fill2d(h2_i3hRFish_ ## X, iT3, iR3); \
  fill2d(h2_i4hXFish_ ## X, iT4, iX4); \
  fill2d(h2_i4hYFish_ ## X, iT4, iY4); \
  fill2d(h2_i4hRFish_ ## X, iT4, iR4); \
  fill2d(h2_i5hXFish_ ## X, iT5, iX5); \
  fill2d(h2_i5hYFish_ ## X, iT5, iY5); \
  fill2d(h2_i5hRFish_ ## X, iT5, iR5); \
}
  __FILLIONFISH__(always)
  __FILLIONFISH__(iMaster)
  __FILLIONFISH__(master)

// IonMomentum
#define __FILLIONMOMENTUM__(X) \
  if (X) { \
    fill1d(h1_i1hPX_ ## X, iPX1); \
    fill1d(h1_i1hPY_ ## X, iPY1); \
    fill1d(h1_i1hPZ_ ## X, iPZ1); \
    fill2d(h2_i1hPXY_ ## X, iPX1, iPY1); \
    fill2d(h2_i1hPYZ_ ## X, iPY1, iPZ1); \
    fill2d(h2_i1hPZX_ ## X, iPZ1, iPX1); \
    fill1d(h1_i1hP_ ## X, iP1); \
    fill1d(h1_i2hPX_ ## X, iPX2); \
    fill1d(h1_i2hPY_ ## X, iPY2); \
    fill1d(h1_i2hPZ_ ## X, iPZ2); \
    fill2d(h2_i2hPXY_ ## X, iPX2, iPY2); \
    fill2d(h2_i2hPYZ_ ## X, iPY2, iPZ2); \
    fill2d(h2_i2hPZX_ ## X, iPZ2, iPX2); \
    fill1d(h1_i2hP_ ## X, iP2); \
    fill1d(h1_i3hPX_ ## X, iPX3); \
    fill1d(h1_i3hPY_ ## X, iPY3); \
    fill1d(h1_i3hPZ_ ## X, iPZ3); \
    fill2d(h2_i3hPXY_ ## X, iPX3, iPY3); \
    fill2d(h2_i3hPYZ_ ## X, iPY3, iPZ3); \
    fill2d(h2_i3hPZX_ ## X, iPZ3, iPX3); \
    fill1d(h1_i3hP_ ## X, iP3); \
    fill1d(h1_i4hPX_ ## X, iPX4); \
    fill1d(h1_i4hPY_ ## X, iPY4); \
    fill1d(h1_i4hPZ_ ## X, iPZ4); \
    fill2d(h2_i4hPXY_ ## X, iPX4, iPY4); \
    fill2d(h2_i4hPYZ_ ## X, iPY4, iPZ4); \
    fill2d(h2_i4hPZX_ ## X, iPZ4, iPX4); \
    fill1d(h1_i4hP_ ## X, iP4); \
    fill1d(h1_i5hPX_ ## X, iPX5); \
    fill1d(h1_i5hPY_ ## X, iPY5); \
    fill1d(h1_i5hPZ_ ## X, iPZ5); \
    fill2d(h2_i5hPXY_ ## X, iPX5, iPY5); \
    fill2d(h2_i5hPYZ_ ## X, iPY5, iPZ5); \
    fill2d(h2_i5hPZX_ ## X, iPZ5, iPX5); \
    fill1d(h1_i5hP_ ## X, iP5); \
    fill1d(h1_iTotalPX_ ## X, iPXTotal); \
    fill1d(h1_iTotalPY_ ## X, iPYTotal); \
    fill1d(h1_iTotalPZ_ ## X, iPZTotal); \
    fill2d(h2_iTotalPXY_ ## X, iPXTotal, iPYTotal); \
    fill2d(h2_iTotalPYZ_ ## X, iPYTotal, iPZTotal); \
    fill2d(h2_iTotalPZX_ ## X, iPZTotal, iPXTotal); \
    fill1d(h1_iTotalP_ ## X, iPTotal); \
    fill2d(h2_i1h2hPX_ ## X, iPX1, iPX2); \
    fill2d(h2_i1h2hPY_ ## X, iPY1, iPY2); \
    fill2d(h2_i1h2hPZ_ ## X, iPZ1, iPZ2); \
    fill2d(h2_i1h2hP_ ## X, iP1, iP2); \
    fill2d(h2_i2h3hPX_ ## X, iPX2, iPX3); \
    fill2d(h2_i2h3hPY_ ## X, iPY2, iPY3); \
    fill2d(h2_i2h3hPZ_ ## X, iPZ2, iPZ3); \
    fill2d(h2_i2h3hP_ ## X, iP2, iP3); \
    fill2d(h2_i3h4hPX_ ## X, iPX3, iPX4); \
    fill2d(h2_i3h4hPY_ ## X, iPY3, iPY4); \
    fill2d(h2_i3h4hPZ_ ## X, iPZ3, iPZ4); \
    fill2d(h2_i3h4hP_ ## X, iP3, iP4); \
    fill2d(h2_i4h5hPX_ ## X, iPX4, iPX5); \
    fill2d(h2_i4h5hPY_ ## X, iPY4, iPY5); \
    fill2d(h2_i4h5hPZ_ ## X, iPZ4, iPZ5); \
    fill2d(h2_i4h5hP_ ## X, iP4, iP5); \
  }
  __FILLIONMOMENTUM__(always)
  __FILLIONMOMENTUM__(iMaster)
  __FILLIONMOMENTUM__(master)

// IonMomentumAngDist
#define __FILLIONMOMENTUMANGDIST__(X) \
  if (X) { \
    fill2d(h2_i1hPDirDistX_ ## X, iCosPDirX1, iP1); \
    fill2d(h2_i1hPDirDistY_ ## X, iCosPDirY1, iP1); \
    fill2d(h2_i1hPDirDistZ_ ## X, iCosPDirZ1, iP1); \
    fill2d(h2_i1hPDirDistXY_ ## X, iPDirXY1, iPXY1); \
    fill2d(h2_i1hPDirDistYZ_ ## X, iPDirYZ1, iPYZ1); \
    fill2d(h2_i1hPDirDistZX_ ## X, iPDirZX1, iPZX1); \
    if (iPDirZ1Is90) fill2d(h2_i1hPDirDistXY_PDirZIs90_ ## X, iPDirXY1, iPXY1); \
    if (iPDirX1Is90) fill2d(h2_i1hPDirDistYZ_PDirXIs90_ ## X, iPDirYZ1, iPYZ1); \
    if (iPDirY1Is90) fill2d(h2_i1hPDirDistZX_PDirYIs90_ ## X, iPDirZX1, iPZX1); \
    fill2d(h2_i1hPDirDist_ ## X, iPDirXY1, iCosPDirZ1); \
    fill2d(h2_i2hPDirDistX_ ## X, iCosPDirX2, iP2); \
    fill2d(h2_i2hPDirDistY_ ## X, iCosPDirY2, iP2); \
    fill2d(h2_i2hPDirDistZ_ ## X, iCosPDirZ2, iP2); \
    fill2d(h2_i2hPDirDistXY_ ## X, iPDirXY2, iPXY2); \
    fill2d(h2_i2hPDirDistYZ_ ## X, iPDirYZ2, iPYZ2); \
    fill2d(h2_i2hPDirDistZX_ ## X, iPDirZX2, iPZX2); \
    if (iPDirZ2Is90) fill2d(h2_i2hPDirDistXY_PDirZIs90_ ## X, iPDirXY2, iPXY2); \
    if (iPDirX2Is90) fill2d(h2_i2hPDirDistYZ_PDirXIs90_ ## X, iPDirYZ2, iPYZ2); \
    if (iPDirY2Is90) fill2d(h2_i2hPDirDistZX_PDirYIs90_ ## X, iPDirZX2, iPZX2); \
    fill2d(h2_i2hPDirDist_ ## X, iPDirXY2, iCosPDirZ2); \
    fill2d(h2_i3hPDirDistX_ ## X, iCosPDirX3, iP3); \
    fill2d(h2_i3hPDirDistY_ ## X, iCosPDirY3, iP3); \
    fill2d(h2_i3hPDirDistZ_ ## X, iCosPDirZ3, iP3); \
    fill2d(h2_i3hPDirDistXY_ ## X, iPDirXY3, iPXY3); \
    fill2d(h2_i3hPDirDistYZ_ ## X, iPDirYZ3, iPYZ3); \
    fill2d(h2_i3hPDirDistZX_ ## X, iPDirZX3, iPZX3); \
    if (iPDirZ3Is90) fill2d(h2_i3hPDirDistXY_PDirZIs90_ ## X, iPDirXY3, iPXY3); \
    if (iPDirX3Is90) fill2d(h2_i3hPDirDistYZ_PDirXIs90_ ## X, iPDirYZ3, iPYZ3); \
    if (iPDirY3Is90) fill2d(h2_i3hPDirDistZX_PDirYIs90_ ## X, iPDirZX3, iPZX3); \
    fill2d(h2_i3hPDirDist_ ## X, iPDirXY3, iCosPDirZ3); \
    fill2d(h2_i4hPDirDistX_ ## X, iCosPDirX4, iP4); \
    fill2d(h2_i4hPDirDistY_ ## X, iCosPDirY4, iP4); \
    fill2d(h2_i4hPDirDistZ_ ## X, iCosPDirZ4, iP4); \
    fill2d(h2_i4hPDirDistXY_ ## X, iPDirXY4, iPXY4); \
    fill2d(h2_i4hPDirDistYZ_ ## X, iPDirYZ4, iPYZ4); \
    fill2d(h2_i4hPDirDistZX_ ## X, iPDirZX4, iPZX4); \
    if (iPDirZ4Is90) fill2d(h2_i4hPDirDistXY_PDirZIs90_ ## X, iPDirXY4, iPXY4); \
    if (iPDirX4Is90) fill2d(h2_i4hPDirDistYZ_PDirXIs90_ ## X, iPDirYZ4, iPYZ4); \
    if (iPDirY4Is90) fill2d(h2_i4hPDirDistZX_PDirYIs90_ ## X, iPDirZX4, iPZX4); \
    fill2d(h2_i4hPDirDist_ ## X, iPDirXY4, iCosPDirZ4); \
    fill2d(h2_i5hPDirDistX_ ## X, iCosPDirX5, iP5); \
    fill2d(h2_i5hPDirDistY_ ## X, iCosPDirY5, iP5); \
    fill2d(h2_i5hPDirDistZ_ ## X, iCosPDirZ5, iP5); \
    fill2d(h2_i5hPDirDistXY_ ## X, iPDirXY5, iPXY5); \
    fill2d(h2_i5hPDirDistYZ_ ## X, iPDirYZ5, iPYZ5); \
    fill2d(h2_i5hPDirDistZX_ ## X, iPDirZX5, iPZX5); \
    if (iPDirZ5Is90) fill2d(h2_i5hPDirDistXY_PDirZIs90_ ## X, iPDirXY5, iPXY5); \
    if (iPDirX5Is90) fill2d(h2_i5hPDirDistYZ_PDirXIs90_ ## X, iPDirYZ5, iPYZ5); \
    if (iPDirY5Is90) fill2d(h2_i5hPDirDistZX_PDirYIs90_ ## X, iPDirZX5, iPZX5); \
    fill2d(h2_i5hPDirDist_ ## X, iPDirXY5, iCosPDirZ5); \
    fill2d(h2_iTotalPDirDistX_ ## X, iCosPDirXTotal, iPTotal); \
    fill2d(h2_iTotalPDirDistY_ ## X, iCosPDirYTotal, iPTotal); \
    fill2d(h2_iTotalPDirDistZ_ ## X, iCosPDirZTotal, iPTotal); \
    fill2d(h2_iTotalPDirDistXY_ ## X, iPDirXYTotal, iPXYTotal); \
    fill2d(h2_iTotalPDirDistYZ_ ## X, iPDirYZTotal, iPYZTotal); \
    fill2d(h2_iTotalPDirDistZX_ ## X, iPDirZXTotal, iPZXTotal); \
    if (iPDirZTotalIs90) fill2d(h2_iTotalPDirDistXY_PDirZIs90_ ## X, iPDirXYTotal, iPXYTotal); \
    if (iPDirXTotalIs90) fill2d(h2_iTotalPDirDistYZ_PDirXIs90_ ## X, iPDirYZTotal, iPYZTotal); \
    if (iPDirYTotalIs90) fill2d(h2_iTotalPDirDistZX_PDirYIs90_ ## X, iPDirZXTotal, iPZXTotal); \
    fill2d(h2_iTotalPDirDist_ ## X, iPDirXYTotal, iCosPDirZTotal); \
  }
  __FILLIONMOMENTUMANGDIST__(always)
  __FILLIONMOMENTUMANGDIST__(iMaster)
  __FILLIONMOMENTUMANGDIST__(master)

// IonEnergy
#define __FILLIONENERGY__(X) \
  if (X) { \
    fill1d(h1_i1hE_ ## X, iE1); \
    fill1d(h1_i2hE_ ## X, iE2); \
    fill1d(h1_i3hE_ ## X, iE3); \
    fill1d(h1_i4hE_ ## X, iE4); \
    fill1d(h1_i5hE_ ## X, iE5); \
    fill1d(h1_iTotalE_ ## X, iETotal); \
    fill2d(h2_i1h2hE_ ## X, iE1, iE2); \
    fill2d(h2_i2h3hE_ ## X, iE2, iE3); \
    fill2d(h2_i3h4hE_ ## X, iE3, iE4); \
    fill2d(h2_i4h5hE_ ## X, iE4, iE5); \
  }
  __FILLIONENERGY__(always)
  __FILLIONENERGY__(iMaster)
  __FILLIONENERGY__(master)

  // ElecImage
  #define __FILLELECIMAGE__(X) \
  if (X) { \
    fill2d(h2_e1hImage_ ## X, eX1, eY1); \
    fill2d(h2_e2hImage_ ## X, eX2, eY2); \
    fill2d(h2_e3hImage_ ## X, eX3, eY3); \
    fill2d(h2_e4hImage_ ## X, eX4, eY4); \
    fill2d(h2_e1hImageDirDist_ ## X, eDir1, eR1); \
    fill2d(h2_e2hImageDirDist_ ## X, eDir2, eR2); \
    fill2d(h2_e3hImageDirDist_ ## X, eDir3, eR3); \
    fill2d(h2_e4hImageDirDist_ ## X, eDir4, eR4); \
  }
  __FILLELECIMAGE__(always)
  __FILLELECIMAGE__(eMaster)
  __FILLELECIMAGE__(master)

  // ElecTOF
  #define __FILLELECTOF__(X) \
  if (X) { \
    fill1d(h1_e1hTOF_ ## X, eT1); \
    fill1d(h1_e2hTOF_ ## X, eT2); \
    fill1d(h1_e3hTOF_ ## X, eT3); \
    fill1d(h1_e4hTOF_ ## X, eT4); \
    fill2d(h2_e1h2hPEPECO_ ## X, eT1, eT2); \
    fill2d(h2_e2h3hPEPECO_ ## X, eT2, eT3); \
    fill2d(h2_e3h4hPEPECO_ ## X, eT3, eT4); \
  }
  __FILLELECTOF__(always)
  __FILLELECTOF__(eMaster)
  __FILLELECTOF__(master)

  // ElecFish
  #define __FILLELECFISH__(X) \
  if (X) { \
    fill2d(h2_e1hXFish_ ## X, eT1, eX1); \
    fill2d(h2_e1hYFish_ ## X, eT1, eY1); \
    fill2d(h2_e1hRFish_ ## X, eT1, eR1); \
    fill2d(h2_e2hXFish_ ## X, eT2, eX2); \
    fill2d(h2_e2hYFish_ ## X, eT2, eY2); \
    fill2d(h2_e2hRFish_ ## X, eT2, eR2); \
    fill2d(h2_e3hXFish_ ## X, eT3, eX3); \
    fill2d(h2_e3hYFish_ ## X, eT3, eY3); \
    fill2d(h2_e3hRFish_ ## X, eT3, eR3); \
    fill2d(h2_e4hXFish_ ## X, eT4, eX4); \
    fill2d(h2_e4hYFish_ ## X, eT4, eY4); \
    fill2d(h2_e4hRFish_ ## X, eT4, eR4); \
  }
  __FILLELECFISH__(always)
  __FILLELECFISH__(eMaster)
  __FILLELECFISH__(master)

  // ElecMomentum
  #define __FILLELECMOMENTUM__(X) \
  if (X) { \
    fill1d(h1_e1hPX_ ## X, ePX1); \
    fill1d(h1_e1hPY_ ## X, ePY1); \
    fill1d(h1_e1hPZ_ ## X, ePZ1); \
    fill2d(h2_e1hPXY_ ## X, ePX1, ePY1); \
    fill2d(h2_e1hPYZ_ ## X, ePY1, ePZ1); \
    fill2d(h2_e1hPZX_ ## X, ePZ1, ePX1); \
    fill1d(h1_e1hP_ ## X, eP1); \
    fill1d(h1_e2hPX_ ## X, ePX2); \
    fill1d(h1_e2hPY_ ## X, ePY2); \
    fill1d(h1_e2hPZ_ ## X, ePZ2); \
    fill2d(h2_e2hPXY_ ## X, ePX2, ePY2); \
    fill2d(h2_e2hPYZ_ ## X, ePY2, ePZ2); \
    fill2d(h2_e2hPZX_ ## X, ePZ2, ePX2); \
    fill1d(h1_e2hP_ ## X, eP2); \
    fill1d(h1_e3hPX_ ## X, ePX3); \
    fill1d(h1_e3hPY_ ## X, ePY3); \
    fill1d(h1_e3hPZ_ ## X, ePZ3); \
    fill2d(h2_e3hPXY_ ## X, ePX3, ePY3); \
    fill2d(h2_e3hPYZ_ ## X, ePY3, ePZ3); \
    fill2d(h2_e3hPZX_ ## X, ePZ3, ePX3); \
    fill1d(h1_e3hP_ ## X, eP3); \
    fill1d(h1_e4hPX_ ## X, ePX4); \
    fill1d(h1_e4hPY_ ## X, ePY4); \
    fill1d(h1_e4hPZ_ ## X, ePZ4); \
    fill2d(h2_e4hPXY_ ## X, ePX4, ePY4); \
    fill2d(h2_e4hPYZ_ ## X, ePY4, ePZ4); \
    fill2d(h2_e4hPZX_ ## X, ePZ4, ePX4); \
    fill1d(h1_e4hP_ ## X, eP4); \
  }
  __FILLELECMOMENTUM__(always)
  __FILLELECMOMENTUM__(eMaster)
  __FILLELECMOMENTUM__(master)

  // ElecMomentumAngDist 
  #define __FILLELECMOMENTUMANGDIST__(X) \
  if (X) { \
    fill2d(h2_e1hPDirDistX_ ## X, eCosPDirX1, eP1); \
    fill2d(h2_e1hPDirDistY_ ## X, eCosPDirY1, eP1); \
    fill2d(h2_e1hPDirDistZ_ ## X, eCosPDirZ1, eP1); \
    fill2d(h2_e1hPDirDistXY_ ## X, ePDirXY1, ePXY1); \
    fill2d(h2_e1hPDirDistYZ_ ## X, ePDirYZ1, ePYZ1); \
    fill2d(h2_e1hPDirDistZX_ ## X, ePDirZX1, ePZX1); \
    if (ePDirZ1Is90) fill2d(h2_e1hPDirDistXY_PDirZIs90_ ## X, ePDirXY1, ePXY1); \
    if (ePDirX1Is90) fill2d(h2_e1hPDirDistYZ_PDirXIs90_ ## X, ePDirYZ1, ePYZ1); \
    if (ePDirY1Is90) fill2d(h2_e1hPDirDistZX_PDirYIs90_ ## X, ePDirZX1, ePZX1); \
    fill2d(h2_e1hPDirDist_ ## X, ePDirXY1, eCosPDirZ1); \
    fill2d(h2_e2hPDirDistX_ ## X, eCosPDirX2, eP2); \
    fill2d(h2_e2hPDirDistY_ ## X, eCosPDirY2, eP2); \
    fill2d(h2_e2hPDirDistZ_ ## X, eCosPDirZ2, eP2); \
    fill2d(h2_e2hPDirDistXY_ ## X, ePDirXY2, ePXY2); \
    fill2d(h2_e2hPDirDistYZ_ ## X, ePDirYZ2, ePYZ2); \
    fill2d(h2_e2hPDirDistZX_ ## X, ePDirZX2, ePZX2); \
    if (ePDirZ2Is90) fill2d(h2_e2hPDirDistXY_PDirZIs90_ ## X, ePDirXY2, ePXY2); \
    if (ePDirX2Is90) fill2d(h2_e2hPDirDistYZ_PDirXIs90_ ## X, ePDirYZ2, ePYZ2); \
    if (ePDirY2Is90) fill2d(h2_e2hPDirDistZX_PDirYIs90_ ## X, ePDirZX2, ePZX2); \
    fill2d(h2_e2hPDirDist_ ## X, ePDirXY2, eCosPDirZ2); \
    fill2d(h2_e3hPDirDistX_ ## X, eCosPDirX3, eP3); \
    fill2d(h2_e3hPDirDistY_ ## X, eCosPDirY3, eP3); \
    fill2d(h2_e3hPDirDistZ_ ## X, eCosPDirZ3, eP3); \
    fill2d(h2_e3hPDirDistXY_ ## X, ePDirXY3, ePXY3); \
    fill2d(h2_e3hPDirDistYZ_ ## X, ePDirYZ3, ePYZ3); \
    fill2d(h2_e3hPDirDistZX_ ## X, ePDirZX3, ePZX3); \
    if (ePDirZ3Is90) fill2d(h2_e3hPDirDistXY_PDirZIs90_ ## X, ePDirXY3, ePXY3); \
    if (ePDirX3Is90) fill2d(h2_e3hPDirDistYZ_PDirXIs90_ ## X, ePDirYZ3, ePYZ3); \
    if (ePDirY3Is90) fill2d(h2_e3hPDirDistZX_PDirYIs90_ ## X, ePDirZX3, ePZX3); \
    fill2d(h2_e3hPDirDist_ ## X, ePDirXY3, eCosPDirZ3); \
    fill2d(h2_e4hPDirDistX_ ## X, eCosPDirX4, eP4); \
    fill2d(h2_e4hPDirDistY_ ## X, eCosPDirY4, eP4); \
    fill2d(h2_e4hPDirDistZ_ ## X, eCosPDirZ4, eP4); \
    fill2d(h2_e4hPDirDistXY_ ## X, ePDirXY4, ePXY4); \
    fill2d(h2_e4hPDirDistYZ_ ## X, ePDirYZ4, ePYZ4); \
    fill2d(h2_e4hPDirDistZX_ ## X, ePDirZX4, ePZX4); \
    if (ePDirZ4Is90) fill2d(h2_e4hPDirDistXY_PDirZIs90_ ## X, ePDirXY4, ePXY4); \
    if (ePDirX4Is90) fill2d(h2_e4hPDirDistYZ_PDirXIs90_ ## X, ePDirYZ4, ePYZ4); \
    if (ePDirY4Is90) fill2d(h2_e4hPDirDistZX_PDirYIs90_ ## X, ePDirZX4, ePZX4); \
    fill2d(h2_e4hPDirDist_ ## X, ePDirXY4, eCosPDirZ4); \
  }
  __FILLELECMOMENTUMANGDIST__(always)
  __FILLELECMOMENTUMANGDIST__(eMaster)
  __FILLELECMOMENTUMANGDIST__(master)

  // ElecEnergy
#define __FILLELECENERGY__(X) \
 if (X) { \
    fill1d(h1_e1hE_ ## X, eE1); \
    fill1d(h1_e2hE_ ## X, eE2); \
    fill1d(h1_e3hE_ ## X, eE3); \
    fill1d(h1_e4hE_ ## X, eE4); \
    fill1d(h1_eTotalE_ ## X, eETotal); \
    fill2d(h2_e1h2hE_ ## X, eE1, eE2); \
    fill2d(h2_e2h3hE_ ## X, eE2, eE3); \
    fill2d(h2_e3h4hE_ ## X, eE3, eE4); \
  }
  __FILLELECENERGY__(always)
  __FILLELECENERGY__(eMaster)
  __FILLELECENERGY__(master)

  // IonElecCorr
  #define __FILLIONELECCORR__(X) \
  if (X) { \
    fill2d(h2_iKER_e1hE_ ## X, iETotal, eE1); \
  }
  __FILLIONELECCORR__(always)
  __FILLIONELECCORR__(master)

  // Others
  if ((!iDead2) && (!iDead3)) {
    fill1d(h1_i1hTOF_i2h3hNotDead, iT1);
  }
  if (iMaster1) {
    fill2d(h2_i2h3hPIPICO_i1hMaster, iT2, iT3);
  }
  if (master) {
    fill2d(h2_e1hE_iTotalTOF_master, eE1, iTTotal);
  }
}