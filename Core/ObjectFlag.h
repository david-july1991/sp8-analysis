//
// Created by Daehyun You on 11/27/15.
//

#ifndef ANALYSIS_OBJECTFLAG_H
#define ANALYSIS_OBJECTFLAG_H

#include "Flag.h"

namespace Analysis {
class ObjectFlag: protected Flag {
 private:
  const unsigned int initialFlag = 0;
 public:
  ObjectFlag();
  ~ObjectFlag();
  void resetFlag();
  const bool isInitial() const;
  // todo: make only one pulic method using string option

 private:
  const unsigned int flagFor1stDigit_withinMasterRegion = 1;
  const unsigned int flagFor1stDigit_outOfMasterRegion = 2;
  const unsigned int flagFor1stDigit_dead = 3;
//  ┌─whole data────────────────────────────────┐
//  │                                           │
//  │ ┌─master region─┐┌─out of master region─┐ │
//  │ │               ││                      │ │
//  │ │               ││         ┌─dead────┐  │ │
//  │ │               ││         │         │  │ │
//  │ │ flag #1       ││ flag #2 │ flag #3 │  │ │
//  │ │               ││         │         │  │ │
//  │ │               ││         └─────────┘  │ │
//  │ └───────────────┘└──────────────────────┘ │
//  │                                           │
//  │ initial data (not input TOF data)         │
//  │ flag #0                                   │
//  │                                           │
//  └───────────────────────────────────────────┘
 public:
  void setWithinMasterRegionFlag(); // only can set when it's initial flag(0)
  void setOutOfMasterRegionFlag(); // only can set when it's initial flag(0)
  void setDeadFlag();
  const bool isDead() const;
  const bool isOutOfMasterRegion() const;
  const bool isWithinMasterRegion() const;

 private:
  const unsigned int flagFor2ndDigit_inFrameOfAllData = 1;
  const unsigned int flagFor2ndDigit_outOfFrameOfMomentumData = 2;
  const unsigned int flagFor2ndDigit_outOfFrameOfBasicData = 3;
//  ┌─whole data───────────────────────────────────────────────────────┐
//  │┌─in frame of all data─┐┌─out of frame of momentum data──────────┐│
//  ││                      ││                 ~~~~~~~~~~~~~          ││
//  ││                      ││                 (Px,Py,Pz,E)           ││
//  ││                      ││         ┌─out of frame of basic data──┐││
//  ││                      ││         │                 ~~~~~~~~~~  │││
//  ││ flag #1              ││ flag #2 │ flag #3         (X,Y,TOF,   │││
//  ││                      ││         │                  Px,Py,Pz,E)│││
//  ││                      ││         └─────────────────────────────┘││
//  │└──────────────────────┘└────────────────────────────────────────┘│
//  │ initial data (not inputed)                                       │
//  │ flag #0                                                          │
//  └──────────────────────────────────────────────────────────────────┘
 public:
  void setInFrameOfAllDataFlag(); // only can set when it's initial flag(0)
  void setOutOfFrameOfMomentumDataFlag
      (); // only can set when it's in-frame-of-all-data flag(1)
  void setOutOfFrameOfBasicDataFlag
      (); // only can set when it's in-frame-of-all-data flag(1), or out-of-frame-of-momentum-data flag(2)
  const bool isInFrameOfAllData() const;
  const bool isOutOfFrameOfMomentumData() const;
  const bool isOutOfFrameOfBasicData() const;

 private:
  const unsigned int flagForResort_theRegion1 = 0;
  const unsigned int flagForResort_theRegion2 = 20;
  const unsigned int flagForResort_outOfTheRegion = 21;
  const unsigned int flagForResort_mostReliableRegion1 = 0;
  const unsigned int flagForResort_mostReliableRegion2 = 3;
  const unsigned int flagForResort_secondMostReliableRegion1 = 4;
  const unsigned int flagForResort_secondMostReliableRegion2 = 14;
  const unsigned int flagForResort_riskyRegion1 = 15;
  const unsigned int flagForResort_riskyRegion2 = 20;
  // most reliable region: 0 to 3
  // second most reliable region: 4 to 14
  // risky region: 15 to 20
  const unsigned int flagFor3rd2Digit_init = 0;
  const unsigned int flagFor3rd2Digit_inTheRegion1 = 1;
  const unsigned int flagFor3rd2Digit_inTheRegion2 = flagForResort_theRegion2 - flagForResort_theRegion1
      + 1; // 21
  const unsigned int flagFor3rd2Digit_lowerThanTheRegion = flagFor3rd2Digit_inTheRegion2 + 1;
  const unsigned int flagFor3rd2Digit_upperThanTheRegion = flagFor3rd2Digit_inTheRegion2 + 2;
  unsigned int convertCoboldFlag(const int coboldFlag) const;
  const unsigned int convertToCoboldFlag(const unsigned int storedFlag) const;
 public:
  // flag for 3rd digit
  void setResortFlag(const int coboldFlag);
  const bool isResortFlag(const int coboldFlag) const;
  const bool isMostReliable() const;
  const bool isMostOrSecondMostReliable() const;
  const bool isRisky() const;
  const unsigned int getResortFlag() const;
};
}

#endif //ANALYSIS_FLAGLIST_H
