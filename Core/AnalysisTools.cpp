#include "AnalysisTools.h"

Analysis::AnalysisTools::AnalysisTools(const EquipmentParameters &equip,
                                       const IonParameters &ion,
                                       const ElectronParameters &elec,
                                       const std::string ID)
    : equipmentParameters(equip),
      ionParameters(ion),
      electronParameters(elec),
      ID(ID) {
  resetCounter();
  return;
}
Analysis::AnalysisTools::~AnalysisTools() { return; }
void Analysis::AnalysisTools::resetCounter() {
  this->eventNumber = 0;
  return;
}
const double Analysis::AnalysisTools::calculateTOF(const Ion &ion,
                                                   const double &p0) const {
  const double &len1 = getEquipmentParameters().getLengthOfL1();
  const double &len2 = getEquipmentParameters().getLengthOfL2();
  const double &len3 = getEquipmentParameters().getLengthOfL3();
  const double &E1 = getEquipmentParameters().getElectricFiledOfL1();
  const double &E2 = getEquipmentParameters().getElectricFiledOfL2();
  const double &E3 = getEquipmentParameters().getElectricFiledOfL3();
  const double &m = ion.getMass();
  const double &q = ion.getCharge();
  const double p1 = pow(pow(p0, 2e0) + 2e0 * m * q * E1 * len1, 0.5e0);
  const double p2 = pow(pow(p1, 2e0) + 2e0 * m * q * E2 * len2, 0.5e0);
  const double p3 = pow(pow(p2, 2e0) + 2e0 * m * q * E3 * len3, 0.5e0);
  const double t1 = (p1 - p0) / q / E1;
  const double t2 = (p2 - p1) / q / E2;
  const double t3 = (p3 - p2) / q / E3;
  return t1 + t2 + t3;
}
const double Analysis::AnalysisTools::calculateDiffTOF(const Ion &ion,
                                                       const double &p0) const {
  const double &len1 = getEquipmentParameters().getLengthOfL1();
  const double &len2 = getEquipmentParameters().getLengthOfL2();
  const double &len3 = getEquipmentParameters().getLengthOfL3();
  const double &E1 = getEquipmentParameters().getElectricFiledOfL1();
  const double &E2 = getEquipmentParameters().getElectricFiledOfL2();
  const double &E3 = getEquipmentParameters().getElectricFiledOfL3();
  const double &m = ion.getMass();
  const double &q = ion.getCharge();
  return
      -1.0 * pow(p0, 1.0)
          * pow(2.0 * E2 * len2 * m * q
                    + pow(2.0 * E1 * len1 * m * q + pow(p0, 2.0), 1.0), -0.5)
          / (E3 * q)
          + 1.0 * pow(p0, 1.0)
              * pow(2.0 * E3 * len3 * m * q
                        + pow(2.0 * E2 * len2 * m * q
                                  + pow(2.0 * E1 * len1 * m * q
                                            + pow(p0, 2.0), 1.0), 1.0), -0.5)
              / (E3 * q)
          - 1.0 * pow(p0, 1.0)
              * pow(2.0 * E1 * len1 * m * q + pow(p0, 2.0), -0.5)
              / (E2 * q)
          + 1.0 * pow(p0, 1.0)
              * pow(2.0 * E2 * len2 * m * q
                        + pow(2.0 * E1 * len1 * m * q
                                  + pow(p0, 2.0), 1.0), -0.5) / (E2 * q)
          + 1.0 * pow(p0, 1.0)
              * pow(2.0 * E1 * len1 * m * q + pow(p0, 2.0), -0.5)
              / (E1 * q) - 1 / (E1 * q);
}
const double Analysis::AnalysisTools::calculateMomentumZ(const Ion &ion,
                                                         bool &info) const {
  double x0, x1, dx, f, df;
  const double &f0 = ion.getTOF();
  x1 = 0e0;
  info = false;
  for (int i = 0; i < LIMITATION_NUMBER_OF_LOOP; i++) {
    x0 = x1;
    f = calculateTOF(ion, x0) - f0;
    df = calculateDiffTOF(ion, x0);
    dx = -f / df;
    x1 = x0 + dx;
    if (x0 == x1) {
      info = true;
      return x1;
    }
  }
  return 0e0;
}
const double Analysis::AnalysisTools::calculateTOF(
    const Analysis::Electron &electron, const double &p0) const {
  const double &len1 = getEquipmentParameters().getLengthOfD1();
  const double &len2 = getEquipmentParameters().getLengthOfD2();
  const double &E1 = getEquipmentParameters().getElectricFiledOfD1();
  const double &m = electron.getMass();
  const double &q = electron.getCharge();
  const double p1 = pow(pow(p0, 2e0) + 2e0 * m * q * E1 * len1, 0.5e0);
  const double t1 = (p1 - p0) / q / E1;
  const double t2 = len2 * m / p1;
  return t1 + t2;
}
const double Analysis::AnalysisTools::calculateDiffTOF
    (const Electron &elec, const double &p0) const {
  const double &len1 = getEquipmentParameters().getLengthOfD1();
  const double &len2 = getEquipmentParameters().getLengthOfD2();
  const double &E1 = getEquipmentParameters().getElectricFiledOfD1();
  const double &m = elec.getMass();
  const double &q = elec.getCharge();
  return -1.0 * len2 * m * pow(p0, 1.0)
      * pow(2.0 * E1 * len1 * m * q + pow(p0, 2.0), -1.5)
      + 1.0 * pow(p0, 1.0) * pow(2.0 * E1 * len1 * m * q + pow(p0, 2.0), -0.5)
          / (E1 * q)
      - 1 / (E1 * q);
}
const double Analysis::AnalysisTools::calculateMomentumZ
    (const Electron &elec, bool &info) const {
  double x0, x1, dx, f, df;
  const double &f0 = elec.getTOF();
  x1 = 0e0;
  info = false;
  for (int i = 0; i < LIMITATION_NUMBER_OF_LOOP; i++) {
    x0 = x1;
    f = calculateTOF(elec, x0) - f0;
    df = calculateDiffTOF(elec, x0);
    dx = -f / df;
    x1 = x0 + dx;
    if (x0 == x1) {
      info = true;
      return x1;
    }
  }
  return 0e0;
}
const Analysis::EquipmentParameters
&Analysis::AnalysisTools::getEquipmentParameters() const {
  return this->equipmentParameters;
}

const Analysis::IonParameters &Analysis::AnalysisTools::getIonParameters()
const {
  return this->ionParameters;
}

const Analysis::ElectronParameters
&Analysis::AnalysisTools::getElectronParameters() const {
  return this->electronParameters;
}
void Analysis::AnalysisTools::loadEventCounter() { this->eventNumber += 1; }
Analysis::AnalysisTools::AnalysisTools(const Analysis::Unit &unit,
                                       const Analysis::JSONReader &reader)
    : AnalysisTools(Analysis::EquipmentParameters(unit, reader),
                    Analysis::IonParameters(unit, reader),
                    Analysis::ElectronParameters(unit, reader),
                    reader.hasMember("ID") ? reader.getStringAt("ID") : "") {
  return;
}
const Analysis::XY Analysis::AnalysisTools::calculateMomentumXY(
    const Object &object) const {
  const double pi = atan2(0e0, -1e0);
  const double &m = object.getMass();
  const double &q = object.getCharge();
  const double &t = object.getTOF();
  const double &x = object.getLocationX();
  const double &y = object.getLocationY();
  const double &B = getEquipmentParameters().getMagneticFiled();
  if (float(calculateFrequencyOfCycle(object))
      == 0e0) { // small magnetic filed
    return {m * x / t, m * y / t};
  } else { // big magnetic filed 
    const double period = calculatePeriodOfCycle(object);
    double theta = t / period;
    theta = 2e0 * pi * (theta - int(theta));
    theta = 0.5e0 * theta;
    const double p0 = fabs(q * B / sin(theta) / 2e0);
    return calculateRotation({p0 * x, p0 * y}, theta);
  }
}
void Analysis::AnalysisTools::loadEventDataInputer(Analysis::Ion &ion,
                                                   const double &x1,
                                                   const double &y1,
                                                   const double &t1,
                                                   const int &f1) const {
  const double &theta = getIonParameters().getAngleOfDetector();
  const double &dx = getIonParameters().getPixelSizeOfX();
  const double &dy = getIonParameters().getPixelSizeOfY();
  const double &deadTime = getIonParameters().getDeadTime();
  const double &x0 = getIonParameters().getXZeroOfCOM();
  const double &y0 = getIonParameters().getYZeroOfCOM();
  const double &t0 = getIonParameters().getTimeZeroOfTOF();
  const XY xy = calculateRotation({x1 * dx, y1 * dy}, theta);
  const double x = xy.x - x0;
  const double y = xy.y - y0;
  const double t = t1 - t0;
  ion.setLocationX(x);
  ion.setLocationY(y);
  ion.setTOF(t);
  ion.setFlag(ObjectFlag::HavingXYTData);
  ion.setFlag(ObjectFlag::ResortFlag, f1);
  if (ion.getTOF() > deadTime || ion.getTOF() < 0e0) {
    ion.setFlag(ObjectFlag::Dead);
  }
}
void Analysis::AnalysisTools::loadEventDataInputer(Analysis::Electron &electron,
                                                   const double &x1,
                                                   const double &y1,
                                                   const double &t1,
                                                   const int &f1) const {
  const double &theta = getElectronParameters().getAngleOfDetector();
  const double &dx = getElectronParameters().getPixelSizeOfX();
  const double &dy = getElectronParameters().getPixelSizeOfY();
  const double &deadTime = getElectronParameters().getDeadTime();
  const double &x0 = getElectronParameters().getXZeroOfCOM();
  const double &y0 = getElectronParameters().getYZeroOfCOM();
  const double &t0 = getElectronParameters().getTimeZeroOfTOF();
  const XY xy = calculateRotation({x1 * dx, y1 * dy}, theta);
  const double x = xy.x - x0;
  const double y = xy.y - y0;
  const double t = t1 - t0;
  electron.setLocationX(x);
  electron.setLocationY(y);
  electron.setTOF(t);
  electron.setFlag(ObjectFlag::HavingXYTData);
  electron.setFlag(ObjectFlag::ResortFlag, f1);
  if (electron.getTOF() > deadTime || electron.getTOF() < 0e0) {
    electron.setFlag(ObjectFlag::Dead);
  }
}
void Analysis::AnalysisTools::loadMomentumCalculator(Ion &ion, const OptName opt) const {
  if(ion.isFlag(ObjectFlag::DummyObject)) {
    return;
  } else {
    bool isTarget;
    switch (opt) {
      case OnlyWithinMasterRegion:
        isTarget = ion.isFlag(ObjectFlag::WithinMasterRegion);
        break;
      case OnlyNotDead:
        isTarget = !(ion.isFlag(ObjectFlag::Dead));
        break;
      case AllRegion:
        isTarget = true;
        break;
      default:
        assert(false);
        isTarget = false;
    }
    if(isTarget) {
      bool isHavingProperPz;
      const XY &pxy = calculateMomentumXY(ion);
      const double &pz = calculateMomentumZ(ion, isHavingProperPz);
      ion.setMomentumX(pxy.x);
      ion.setMomentumY(pxy.y);
      ion.setFlag(ObjectFlag::HavingMomentumData);
      if (isHavingProperPz) {
        ion.setMomentumZ(pz);
        ion.setFlag(ObjectFlag::HavingProperPzData);
      }
    }
  }
}
void Analysis::AnalysisTools::loadMomentumCalculator(Electron &elec, const OptName opt) const {
  if(elec.isFlag(ObjectFlag::DummyObject)) {
    return;
  } else {
    bool isTarget;
    switch (opt) {
      case OnlyWithinMasterRegion:
        isTarget = elec.isFlag(ObjectFlag::WithinMasterRegion);
        break;
      case OnlyNotDead:
        isTarget = !(elec.isFlag(ObjectFlag::Dead));
        break;
      case AllRegion:
        isTarget = true;
        break;
      default:
        assert(false);
        isTarget = false;
    }
    if (isTarget) {
      bool isHavingProperPz;
      const XY &pxy = calculateMomentumXY(elec);
      const double &pz = calculateMomentumZ(elec, isHavingProperPz);
      elec.setMomentumX(pxy.x);
      elec.setMomentumY(pxy.y);
      elec.setFlag(ObjectFlag::HavingMomentumData);
      if (isHavingProperPz) {
        elec.setMomentumZ(pz);
        elec.setFlag(ObjectFlag::HavingProperPzData);
      }
    }
  }
}
const int &Analysis::AnalysisTools::getEventNumber() const {
  return eventNumber;
}
const double Analysis::AnalysisTools::calculatePeriodOfCycle(
    const Object &object) const {
  return calculatePeriodOfCycle(object.getMass(),
                                object.getCharge(),
                                getEquipmentParameters().getMagneticFiled());
}
void Analysis::AnalysisTools::loadEventDataInputer(Analysis::Ion &ion,
                                                   const Unit &unit,
                                                   const double &x,
                                                   const double &y,
                                                   const double &t,
                                                   const int &f) const {
  this->loadEventDataInputer(ion,
                             unit.readMilliMeter(x),
                             unit.readMilliMeter(y),
                             unit.readNanoSec(t),
                             f);
}
void Analysis::AnalysisTools::loadEventDataInputer(Analysis::Electron &elec,
                                                   const Unit &unit,
                                                   const double &x,
                                                   const double &y,
                                                   const double &t,
                                                   const int &f) const {
  this->loadEventDataInputer(elec,
                             unit.readMilliMeter(x),
                             unit.readMilliMeter(y),
                             unit.readNanoSec(t),
                             f);
}
void Analysis::AnalysisTools::loadMomentumCalculator(Ions &ions, const OptName opt) const {
  const int &n = ions.getNumberOfObjects();
  for (int i = 0; i < n; i++) {
    loadMomentumCalculator(ions.setIonMembers(i), opt);
  }
}
void Analysis::AnalysisTools::loadMomentumCalculator(Electrons &elecs, const OptName opt) const {
  const int &n = elecs.getNumberOfObjects();
  for (int i = 0; i < n; i++) {
    loadMomentumCalculator(elecs.setElectronMembers(i), opt);
  }
}

void Analysis::AnalysisTools::loadEventDataInputer(Analysis::Ion &ion,
                                                   const Unit &unit,
                                                   const EventDataReader &reader,
                                                   const int &iHit) const {
  loadEventDataInputer(ion,
                       unit,
                       reader.getEventDataAt(iHit, "IonX"),
                       reader.getEventDataAt(iHit, "IonY"),
                       reader.getEventDataAt(iHit, "IonT"),
                       reader.getFlagDataAt(iHit, "IonFlag"));
}
void Analysis::AnalysisTools::loadEventDataInputer(Analysis::Electron &elec,
                                                   const Unit &unit,
                                                   const EventDataReader &reader,
                                                   const int &iHit) const {
  loadEventDataInputer(elec,
                       unit,
                       reader.getEventDataAt(iHit, "ElecX"),
                       reader.getEventDataAt(iHit, "ElecY"),
                       reader.getEventDataAt(iHit, "ElecT"),
                       reader.getFlagDataAt(iHit, "ElecFlag"));
}
void Analysis::AnalysisTools::loadEventDataInputer(Analysis::Ions &ions,
                                                   const Unit &unit,
                                                   const EventDataReader &reader) const {
  const int &n = ions.getNumberOfObjects();
  const int &m = ions.getNumberOfRealOrDummyObjects();
  for (int i = 0; i < n; i++) {
    loadEventDataInputer(ions.setIonMembers(i), unit, reader, i);
  }
  for (int i = n; i < m; i++) {
    loadEventDataInputer(ions.setDummyIonMembers(i), unit, reader, i);
  }
}
void Analysis::AnalysisTools::loadEventDataInputer(Analysis::Electrons &electrons,
                                                   const Unit &unit,
                                                   const EventDataReader &reader) const {
  const int &n = electrons.getNumberOfObjects();
  const int &m = electrons.getNumberOfRealOrDummyObjects();
  for (int i = 0; i < n; i++) {
    loadEventDataInputer(electrons.setElectronMembers(i), unit, reader, i);
  }
  for (int i = n; i < m; i++) {
    loadEventDataInputer(electrons.setDummyElectronMembers(i), unit, reader, i);
  }
}

const double Analysis::AnalysisTools::calculateTOF(const Analysis::Unit &unit,
                                                   const Analysis::Ion &ion,
                                                   const double &d) const {
  return unit.writeNanoSec(this->calculateTOF(ion, unit.readAuMomentum(d)));
}
const double Analysis::AnalysisTools::calculateTOF(const Analysis::Unit &unit,
                                                   const Analysis::Electron &elec,
                                                   const double &d) const {
  return unit.writeNanoSec(this->calculateTOF(elec,
                                              unit.readAuMomentum(d)));
}

const double Analysis::AnalysisTools::calculateFrequencyOfCycle(const double &m,
                                                                const double &q,
                                                                const double &B) const {
  const double pi = atan2(0e0, -1e0);
  return 0.5e0 / pi / m * B * q;
}

const double Analysis::AnalysisTools::calculateFrequencyOfCycle(const Object &object) const {
  return calculateFrequencyOfCycle(object.getMass(),
                                   object.getCharge(),
                                   getEquipmentParameters().getMagneticFiled());
}

const double Analysis::AnalysisTools::calculatePeriodOfCycle(const double &m,
                                                             const double &q,
                                                             const double &B) const {
  const double pi = atan2(0e0, -1e0);
  return 2e0 * pi * m / B / q;
}

const double Analysis::AnalysisTools::calculatePeriodOfCycle(
    const Analysis::Unit &unit,
    const Analysis::Object &object) const {
  return unit.writeNanoSec(calculatePeriodOfCycle(object));
}
const Analysis::XY Analysis::AnalysisTools::calculateRotation(
    const XY &xy,
    const double &theta) const {
  return {xy.x * cos(theta) - xy.y * sin(theta),
          xy.x * sin(theta) + xy.y * cos(theta)};
}
const std::string &Analysis::AnalysisTools::getID() const {
  return ID;
}
