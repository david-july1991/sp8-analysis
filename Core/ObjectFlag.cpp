//
// Created by Daehyun You on 11/27/15.
//

#include <cassert>
#include "ObjectFlag.h"

Analysis::ObjectFlag::ObjectFlag() : Flag() {
  return;
}
void Analysis::ObjectFlag::setWithinMasterRegion() {
  if (get1stDigit() == initFlag) {
    set1stDigit(flagFor1stDigit_withinMasterRegion);
  }
}
void Analysis::ObjectFlag::setOutOfMasterRegion() {
  if (get1stDigit() == initFlag) {
    set1stDigit(flagFor1stDigit_outOfMasterRegion);
  }
}
void Analysis::ObjectFlag::setDead() {
  const unsigned int f0 = get1stDigit();
  if (f0 == flagFor1stDigit_withinMasterRegion || f0 == flagFor1stDigit_outOfMasterRegion) {
    set1stDigit(flagFor1stDigit_dead);
  }
}
void Analysis::ObjectFlag::setInFrameOfAllDataFlag() {
  if (getNthDigit(5) == initFlag) {
    setNthDigit(5, flagFor5thDigit_inFrameOfAllData);
  }
}

void Analysis::ObjectFlag::setOutOfFrameOfMomentumDataFlag() {
  if (flagFor5thDigit_inFrameOfAllData <= getNthDigit(5)
      && getNthDigit(5) < flagFor5thDigit_outOfFrameOfMomentumData) {
    setNthDigit(5, flagFor5thDigit_outOfFrameOfMomentumData);
  }
}

void Analysis::ObjectFlag::setOutOfFrameOfBasicDataFlag() {
  if (flagFor5thDigit_inFrameOfAllData <= getNthDigit(5)
      && getNthDigit(5) < flagFor5thDigit_outOfFrameOfBasicData) {
    setNthDigit(5, flagFor5thDigit_outOfFrameOfBasicData);
  }
}
const bool Analysis::ObjectFlag::isInFrameOfAllData() const {
  return getNthDigit(5) == flagFor5thDigit_inFrameOfAllData;
}
const bool Analysis::ObjectFlag::isDead() const {
  return get1stDigit() == flagFor1stDigit_dead;
}
const bool Analysis::ObjectFlag::isOutOfFrameOfBasicData() const {
  return getNthDigit(5)
      >= flagFor5thDigit_outOfFrameOfBasicData;
}

const bool Analysis::ObjectFlag::isOutOfFrameOfMomentumData() const {
  return getNthDigit(5) >= flagFor5thDigit_outOfFrameOfMomentumData;
}

const bool Analysis::ObjectFlag::isOutOfMasterRegionOrDead() const {
  return (get1stDigit() == flagFor1stDigit_outOfMasterRegion) || (get1stDigit() == flagFor1stDigit_dead);
}
const bool Analysis::ObjectFlag::isWithinMasterRegion() const {
  return get1stDigit() == flagFor1stDigit_withinMasterRegion;
}
void Analysis::ObjectFlag::setResortFlag(const int coboldFlag) {
  setNthNumDigit(3, 2, convertCoboldFlag(coboldFlag));
}
const bool Analysis::ObjectFlag::isResortFlag(const int coboldFlag) const {
  return getNthNumDigit(3, 2) == convertCoboldFlag(coboldFlag);
}
const unsigned int Analysis::ObjectFlag::getResortFlag() const {
  return convertToCoboldFlag(getNthNumDigit(3, 2));
}
unsigned int Analysis::ObjectFlag::convertCoboldFlag(const int coboldFlag) const {
  if(coboldFlag < flagForResort_theRegion1) {
    return flagFor3rd2Digit_lowerThanTheRegion;
  } else if(coboldFlag > flagForResort_theRegion2) {
    return flagFor3rd2Digit_upperThanTheRegion;
  } else {
    return (coboldFlag - flagForResort_theRegion1 + 1);
  }
}
const unsigned int Analysis::ObjectFlag::convertToCoboldFlag(const unsigned int storedFlag) const {
  if(!(storedFlag >= flagFor3rd2Digit_inTheRegion1 && storedFlag <= flagFor3rd2Digit_inTheRegion2)) {
    return flagForResort_outOfTheRegion;
  }
  return storedFlag + flagForResort_theRegion1 - 1;
}
const bool Analysis::ObjectFlag::isMostReliable() const {
  return flagForResort_mostReliableRegion1 <= getResortFlag() &&  getResortFlag() <= flagForResort_mostReliableRegion2;
}
const bool Analysis::ObjectFlag::isMostOrSecondMostReliable() const {
  return flagForResort_mostReliableRegion1 <= getResortFlag() &&  getResortFlag() <= flagForResort_secondMostReliableRegion2;
}
const bool Analysis::ObjectFlag::isRisky() const {
  return flagForResort_riskyRegion1 <= getResortFlag() && getResortFlag() <= flagForResort_outOfTheRegion;
}
Analysis::ObjectFlag::~ObjectFlag() {

}
void Analysis::ObjectFlag::setHavingXYTData() {
  if(get2ndDigit() == initFlag) {
    set2ndDigit(flagFor2ndDigit_hasXYTData);
  }
}
void Analysis::ObjectFlag::setHavingMomentumData() {
  if(get2ndDigit() == flagFor2ndDigit_hasXYTData) {
    set2ndDigit(flagFor2ndDigit_hasMomentumData);
  }
}
void Analysis::ObjectFlag::setHavingProperPzData() {
  const unsigned int f0 = get2ndDigit();
  if(f0 == flagFor2ndDigit_hasXYTData || f0 == flagFor2ndDigit_hasMomentumData) {
    set2ndDigit(flagFor2ndDigit_hasProperPzData);
  }
}
const bool Analysis::ObjectFlag::isHavingXYTData() const {
  const unsigned int f0 = get2ndDigit();
  return (f0 == flagFor2ndDigit_hasXYTData) || (f0 == flagFor2ndDigit_hasMomentumData) || (f0 == flagFor2ndDigit_hasProperPzData);
}
const bool Analysis::ObjectFlag::isHavingMomentumData() const {
  const unsigned int f0 = get2ndDigit();
  return (f0 == flagFor2ndDigit_hasMomentumData) || (f0 == flagFor2ndDigit_hasProperPzData);
}
const bool Analysis::ObjectFlag::isHavingProperPzData() const {
  const unsigned int f0 = get2ndDigit();
  return (f0 == flagFor2ndDigit_hasProperPzData);
}