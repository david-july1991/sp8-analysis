//
// Created by Daehyun You on 12/4/15.
//

#include <functional>
#include "Objects.h"

Analysis::Objects::~Objects() {
  for (int i = 0; i < maxNumOfHits; i++) {
    if (ppObject[i]) {
      delete ppObject[i];
      ppObject[i] = nullptr;
    }
  }
  if (ppObject) {
    delete[] ppObject;
    ppObject = nullptr;
  }
}
void Analysis::Objects::setObject(const int &i, Object &object) {
  assert(isRealObject(i));
  if (ppObject[i]) {
    delete ppObject[i];
    ppObject[i] = nullptr;
  }
  ppObject[i] = &object;
}
void Analysis::Objects::setDummyObject(const int &i, Analysis::Object &object) {
  assert(isDummyObject(i));
  if (ppObject[i]) {
    delete ppObject[i];
    ppObject[i] = nullptr;
  }
  ppObject[i] = &object;
}
const int &Analysis::Objects::getNumberOfObjects() const {
  return masterNumOfHits;
}
const double Analysis::Objects::getLocationX() const {
  const int &n = getNumberOfObjects();
  long double sum1 = 0e0, sum2 = 0e0;
  for (int i = 0; i < n; i++) {
    const double tmp = getObject(i).getMass() / getObject(i).getTOF();
    sum1 += tmp;
    sum2 += tmp * getObject(i).getLocationX();
  }
  return double(sum2 / sum1);
}
const double Analysis::Objects::getLocationY() const {
  const int &n = getNumberOfObjects();
  long double sum1 = 0e0, sum2 = 0e0;
  for (int i = 0; i < n; i++) {
    const double tmp = getObject(i).getMass() / getObject(i).getTOF();
    sum1 += tmp;
    sum2 += tmp * getObject(i).getLocationY();
  }
  return double(sum2 / sum1);
}
const double Analysis::Objects::getMomentumX() const {
  const int &n = getNumberOfObjects();
  long double sumOfMomentum = 0e0;
  for (int i = 0; i < n; i++) {
    sumOfMomentum += getObject(i).getMomentumX();
  }
  return double(sumOfMomentum);
}
const double Analysis::Objects::getMomentumY() const {
  const int &n = getNumberOfObjects();
  long double sumOfMomentum = 0e0;
  for (int i = 0; i < n; i++) {
    sumOfMomentum += getObject(i).getMomentumY();
  }
  return double(sumOfMomentum);
}
const double Analysis::Objects::getMomentumZ() const {
  const int &n = getNumberOfObjects();
  long double sumOfMomentum = 0e0;
  for (int i = 0; i < n; i++) {
    sumOfMomentum += getObject(i).getMomentumZ();
  }
  return double(sumOfMomentum);
}
const double Analysis::Objects::getEnergy() const {
  const int &n = getNumberOfObjects();
  long double sumOfEnergy = 0e0;
  for (int i = 0; i < n; i++) {
    sumOfEnergy += getObject(i).getEnergy();
  }
  return double(sumOfEnergy);
}
const Analysis::Object &Analysis::Objects::getObject(const int &i) const {
  assert(isRealObject(i));
  return *ppObject[i];
}
const Analysis::Object &Analysis::Objects::getDummyObject(const int &i) const {
  assert(isDummyObject(i));
  return *ppObject[i];
}

const double Analysis::Objects::getMomentum() const {
  return pow(pow(getMomentumX(), 2e0)
                 + pow(getMomentumY(), 2e0)
                 + pow(getMomentumZ(), 2e0), 0.5e0);
}
const double Analysis::Objects::getMotionalDirectionX() const {
  return acos(getMomentumX() / getMomentum());
}
const double Analysis::Objects::getMotionalDirectionY() const {
  return acos(getMomentumY() / getMomentum());
}
const double Analysis::Objects::getMotionalDirectionZ() const {
  return acos(getMomentumZ() / getMomentum());
}
void Analysis::Objects::resetEventData() {
  const int &n = this->getNumberOfObjects();
  const int &m = this->getNumberOfRealOrDummyObjects();
  for (int i = 0; i < n; i++) {
    this->setObjectMembers(i).resetEventData();
  }
  for (int i = n; i < m; i++) {
    this->setDummyObjectMembers(i).resetEventData();
  }
  return;
}
Analysis::Object &Analysis::Objects::setObjectMembers(const int &i) {
  assert(isRealObject(i));
  return *ppObject[i];
}
Analysis::Object &Analysis::Objects::setDummyObjectMembers(const int &i) {
  assert(isDummyObject(i));
  return *ppObject[i];
}
const int &Analysis::Objects::getNumberOfRealOrDummyObjects() const {
  return maxNumOfHits;
}
const bool Analysis::Objects::isDummyObject(const int &i) const {
  const int &n = getNumberOfObjects();
  const int &m = getNumberOfRealOrDummyObjects();
  return (i >= n) && (i < m);
}
const bool Analysis::Objects::isRealObject(const int &i) const {
  const int &n = getNumberOfObjects();
  return (i >= 0) && (i < n);
}
const int Analysis::Objects::getNumberOfDeadObjects() const {
  int n = 0;
  for (int i = 0; i < masterNumOfHits; i++) {
    if (getObject(i).isFlag(ObjectFlag::Dead)) {
      n += 1;
    }
  }
  return n;
}
const int Analysis::Objects::getNumberOfDeadDummyObjects() const {
  int n = 0;
  for (int i = masterNumOfHits; i < maxNumOfHits; i++) {
    if (getDummyObject(i).isFlag(ObjectFlag::Dead)) {
      n += 1;
    }
  }
  return n;
}
const int Analysis::Objects::getNumberOfDeadRealOrDummyObjects() const {
  return getNumberOfDeadObjects() + getNumberOfDeadDummyObjects();
}
const double Analysis::Objects::getMomentumXY() const {
  return pow(pow(getMomentumX(), 2e0)
                 + pow(getMomentumY(), 2e0), 0.5e0);
}
const int Analysis::Objects::getNumberOfDummyObject() const {
  return getNumberOfRealOrDummyObjects() - getNumberOfObjects();
}
const double Analysis::Objects::getMomentumYZ() const {
  return pow(pow(getMomentumY(), 2e0) + pow(getMomentumZ(), 2e0),
             0.5e0);
}
const double Analysis::Objects::getMomentumZX() const {
  return pow(pow(getMomentumZ(), 2e0) + pow(getMomentumX(), 2e0),
             0.5e0);
}
const double Analysis::Objects::getMotionalDirectionXY() const {
  return atan2(getMomentumY(), getMomentumX());
}
const double Analysis::Objects::getMotionalDirectionYZ() const {
  return atan2(getMomentumZ(), getMomentumY());
}
const double Analysis::Objects::getMotionalDirectionZX() const {
  return atan2(getMomentumX(), getMomentumZ());
}
const double Analysis::Objects::getSumOfTOF(const int &i1,
                                            const int &i2) const {
  return getRealOrDummyObject(i1).getTOF() + getRealOrDummyObject(i2).getTOF();
}
const Analysis::Object &Analysis::Objects::getRealOrDummyObject(const int &i) const {
  const Object *pObject;
  if (isRealObject(i)) {
    pObject = &getObject(i);
  } else if (isDummyObject(i)) {
    pObject = &getDummyObject(i);
  } else {
    assert(false);
  }
  return *pObject;
}
Analysis::Object &Analysis::Objects::setRealOrDummyObjectMembers(const int &i) {
  Object *pObject;
  if (isRealObject(i)) {
    pObject = &setObjectMembers(i);
  } else if (isDummyObject(i)) {
    pObject = &setDummyObjectMembers(i);
  } else {
    assert(false);
  }
  return *pObject;
}

const bool Analysis::Objects::isRealOrDummyObject(const int &i) const {
  return isRealObject(i) || isDummyObject(i);
}
const double Analysis::Objects::getLocationXY() const {
  return pow(pow(getLocationX(), 2e0) + pow(getLocationY(), 2e0), 0.5e0);
}
const double Analysis::Objects::getLocation() const {
  return getLocationXY();
}
const double Analysis::Objects::getLocationalDirectionX() const {
  return acos(getLocationX() / getLocation());
}
const double Analysis::Objects::getLocationalDirectionY() const {
  return acos(getLocationY() / getLocation());
}
const double Analysis::Objects::getLocationalDirectionXY() const {
  return atan2(getLocationY(), getLocationX());
}
const double Analysis::Objects::getLocationalDirectionYX() const {
  return atan2(getLocationX(), getLocationY());
}
const double Analysis::Objects::getMotionalDirectionXZ() const {
  return atan2(getMomentumZ(), getMomentumX());
}
const double Analysis::Objects::getMotionalDirectionYX() const {
  return atan2(getMomentumX(), getMomentumY());
}
const double Analysis::Objects::getMotionalDirectionZY() const {
  return atan2(getMomentumY(), getMomentumZ());
}
const double Analysis::Objects::getSumOfTOF(const int &i1,
                                            const int &i2,
                                            const int &i3) const {
  return getRealOrDummyObject(i1).getTOF()
      + getRealOrDummyObject(i2).getTOF()
      + getRealOrDummyObject(i3).getTOF();
}

const double Analysis::Objects::getSumOfTOF() const {
  const int& n = getNumberOfObjects();
  long double sum = 0;
  for(int i=0; i<n; i++) {
    sum += getObject(i).getTOF();
  }
    return double(sum);
}

const double Analysis::Objects::getDiffOfTOF(const int& i1, const int& i2) const
{ 
  return fabs(getRealOrDummyObject(i1).getTOF() - getRealOrDummyObject(i2).getTOF());
}

const bool Analysis::Objects::areAllFlag(const ObjectFlag::FlagName flagName,
                                         const OptName optName) const {
  int n1=0, n2=0;
  switch(optName) {
    case Real:
      n1 = 0;
      n2 = masterNumOfHits;
      break;
    case Dummy:
      n1 = masterNumOfHits;
      n2 = maxNumOfHits;
      break;
    case RealOrDummy:
      n1 = 0;
      n2 = maxNumOfHits;
      break;
    default:
      assert(false);
  }
  if(n1>=n2) { return false; }
  bool output = true;
  for(int i=n1; i<n2; i++) {
    output = output && getRealOrDummyObject(i).isFlag(flagName);
  }
  return output;
}
const bool Analysis::Objects::existFlag(const ObjectFlag::FlagName flagName,
                                        const OptName optName) const {
  int n1=0, n2=0;
  switch(optName) {
    case Real:
      n1 = 0;
      n2 = masterNumOfHits;
      break;
    case Dummy:
      n1 = masterNumOfHits;
      n2 = maxNumOfHits;
      break;
    case RealOrDummy:
      n1 = 0;
      n2 = maxNumOfHits;
      break;
    default:
      assert(false);
  }
  if(n1>=n2) { return false; }
  bool output = false;
  for(int i=n1; i<n2; i++) {
    output = output || getRealOrDummyObject(i).isFlag(flagName);
  }
  return output;
}
void Analysis::Objects::setAllFlag(const ObjectFlag::FlagName flagName,
                                   const OptName optName) {
  int n1=0, n2=0;
  switch(optName) {
    case Real:
      n1 = 0;
      n2 = masterNumOfHits;
      break;
    case Dummy:
      n1 = masterNumOfHits;
      n2 = maxNumOfHits;
      break;
    case RealOrDummy:
      n1 = 0;
      n2 = maxNumOfHits;
      break;
    default:
      assert(false);
  }
  for(int i=n1; i<n2; i++) {
    setRealOrDummyObjectMembers(i).setFlag(flagName);
  }
}

// output
std::shared_ptr<double> Analysis::Objects::outputCOMLocX() const
{
	if (areAllFlag(ObjectFlag::HavingXYTData))
      return std::make_shared<double>(kUnit.writeMilliMeter(getLocationX()));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputCOMLocY() const
{
	if (areAllFlag(ObjectFlag::HavingXYTData))
      return std::make_shared<double>(kUnit.writeMilliMeter(getLocationY()));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputCOMLocR() const
{
	if (areAllFlag(ObjectFlag::HavingXYTData))
      return std::make_shared<double>(kUnit.writeMilliMeter(getLocation()));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputCOMLocDir() const
{
	if (areAllFlag(ObjectFlag::HavingXYTData))
      return std::make_shared<double>(kUnit.writeDegree(getLocationalDirectionXY()));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputTotalTOF() const
{
	if (areAllFlag(ObjectFlag::HavingXYTData))
      return std::make_shared<double>(kUnit.writeNanoSec(getSumOfTOF()));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputSumOf2TOFs(const int i1, const int i2) const
{
	if ((getRealOrDummyObject(i1).isFlag(ObjectFlag::HavingXYTData))
		&& (getRealOrDummyObject(i2).isFlag(ObjectFlag::HavingXYTData)))
		return std::make_shared<double>(kUnit.writeNanoSec(getSumOfTOF(i1, i2)));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputSumOf3TOFs(const int i1, const int i2, const int i3) const
{
	if ((getRealOrDummyObject(i1).isFlag(ObjectFlag::HavingXYTData))
		&& (getRealOrDummyObject(i2).isFlag(ObjectFlag::HavingXYTData))
		&& (getRealOrDummyObject(i3).isFlag(ObjectFlag::HavingXYTData)))
		return std::make_shared<double>(kUnit.writeNanoSec(getSumOfTOF(i1, i2, i3)));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputDiffOfTOFs(const int i1, const int i2) const
{
	if ((getRealOrDummyObject(i1).isFlag(ObjectFlag::HavingXYTData))
		&& (getRealOrDummyObject(i2).isFlag(ObjectFlag::HavingXYTData)))
		return std::make_shared<double>(kUnit.writeNanoSec(getDiffOfTOF(i1, i2)));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPX() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeAuMomentum(getMomentumX()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPY() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeAuMomentum(getMomentumY()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPZ() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeAuMomentum(getMomentumZ()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPXY() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeAuMomentum(getMomentumXY()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPYZ() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeAuMomentum(getMomentumYZ()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPZX() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeAuMomentum(getMomentumZX()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputP() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeAuMomentum(getMomentum()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirX() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionX()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirY() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionY()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirZ() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionZ()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputE() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeElectronVolt(getEnergy()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputCosPDirZ() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(cos(getMotionalDirectionZ()));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirXY() const
{
	if (areAllFlag(ObjectFlag::HavingMomentumData))
      return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionXY()));
	return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirXZ() const
{
  if (areAllFlag(ObjectFlag::HavingMomentumData))
    return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionXZ()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirYX() const
{
  if (areAllFlag(ObjectFlag::HavingMomentumData))
    return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionYX()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirYZ() const
{
  if (areAllFlag(ObjectFlag::HavingMomentumData))
    return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionYZ()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirZX() const
{
  if (areAllFlag(ObjectFlag::HavingMomentumData))
    return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionZX()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputPDirZY() const
{
  if (areAllFlag(ObjectFlag::HavingMomentumData))
    return std::make_shared<double>(kUnit.writeDegree(getMotionalDirectionZY()));
  return nullptr;
}
Analysis::Objects::Objects(ObjsType tp,
                           const int maxNum,
                           const JSONReader &reader,
                           const std::string prefix)
    : type(tp), maxNumOfHits(maxNum) {
  if (tp == ions) {
    masterNumOfHits = reader.getIntAt(prefix+"number_of_hits");
    assert(0 <= masterNumOfHits && masterNumOfHits <= maxNumOfHits);
    ppObject = new Object *[maxNumOfHits];
    for (int i = 0; i < masterNumOfHits; i++)
      ppObject[i] = new Object(ObjectFlag::IonObject, reader, prefix + getStrNum(i) + "_hit.");
    for (int i = masterNumOfHits; i < maxNumOfHits; i++)
      ppObject[i] = new Object(ObjectFlag::DummyObject, ObjectFlag::IonObject);
  } else if (tp == elecs) {
    masterNumOfHits = reader.getIntAt(prefix+"number_of_hits");
    assert(0 <= masterNumOfHits && masterNumOfHits <= maxNumOfHits);
    ppObject = new Object *[maxNumOfHits];
    for (int i = 0; i < masterNumOfHits; i++)
      ppObject[i] = new Object(ObjectFlag::ElecObject, reader, prefix);
    for (int i = masterNumOfHits; i < maxNumOfHits; i++)
      ppObject[i] = new Object(ObjectFlag::DummyObject, ObjectFlag::ElecObject);
  } else {
    assert(false);
  }
  auto read2DoublesIfItIs = [](const JSONReader &rd, std::function<double(double)> unit) {
    return [&rd, unit](const std::string pos, double &v0, double &v1) {
      if (rd.hasMember(pos)) {
        v0 = unit(rd.getDoubleAt(pos, 0));
        v1 = unit(rd.getDoubleAt(pos, 1));
      } else {
        v0 = 0;
        v1 = 0;
      }
    };
  };
  auto readAt = read2DoublesIfItIs(reader, [&](double v)->double{ return kUnit.readAuMomentum(v); });
  const auto str = "conservation_raw.";
  readAt(prefix + str + "x", frPx, toPx);
  readAt(prefix + str + "y", frPy, toPy);
  readAt(prefix + str + "z", frPz, toPz);
  readAt(prefix + str + "r", frPr, toPr);
  readAt(prefix + str + "e", frE, toE);
}
const std::string Analysis::Objects::getStrNum(int i) const {
  i += 1;
  assert(i > 0);
  const int firstDigit = i % 10;
  const int secondDigit = (i / 10) % 10;
  const std::string str = std::to_string(i);
  if (secondDigit == 1) { return str + "th"; }
  else {
    if (firstDigit == 1) { return str + "st"; }
    else if (firstDigit == 2) { return str + "nd"; }
    else if (firstDigit == 3) { return str + "rd"; }
    else { return str + "th"; }
  }
}
bool Analysis::Objects::isMomentumAndEnergyConserved() const {
  auto isBtw = [](double v, double fr, double to) {
      if (fr == 0 && to == 0) return true;
      else return fr <= v && v <= to;
  };
  auto isValidPx = [=](double p) { return isBtw(p, frPx, toPx); };
  auto isValidPy = [=](double p) { return isBtw(p, frPy, toPy); };
  auto isValidPz = [=](double p) { return isBtw(p, frPz, toPz); };
  auto isValidPr = [=](double p) { return isBtw(p, frPr, toPr); };
  auto isValidE = [=](double e) { return isBtw(e, frE, toE); };
  return isValidPx(getMomentumX())
         && isValidPy(getMomentumY())
         && isValidPz(getMomentumZ())
         && isValidPr(getMomentum())
         && isValidE(getEnergy());
}

std::shared_ptr<double> Analysis::Objects::outputCosPDirX() const {
  if (areAllFlag(ObjectFlag::HavingMomentumData))
    return std::make_shared<double>(cos(getMotionalDirectionX()));
  return nullptr;
}
std::shared_ptr<double> Analysis::Objects::outputCosPDirY() const {
  if (areAllFlag(ObjectFlag::HavingMomentumData))
    return std::make_shared<double>(cos(getMotionalDirectionY()));
  return nullptr;
}
bool Analysis::Objects::isType(const Analysis::Objects::ObjsType objsType) const {
  return type==objsType;
}
