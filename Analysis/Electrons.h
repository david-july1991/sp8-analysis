//
// Created by Daehyun You on 12/4/15.
//

#ifndef ANALYSIS_ELECTRONS_H
#define ANALYSIS_ELECTRONS_H


#include "Objects.h"
#include "Electron.h"

namespace Analysis {
class Electrons : public Objects {
 private:
  Electron *pElectron[MAXIMUM_OF_HITS];
  Electrons(const Unit &, const JSONReader &, const int &, const int &);

 public:
  Electrons(const Unit &, const JSONReader &);
  Electrons(const Unit &, const JSONReader &, const int &);
  ~Electrons();
  Electron &setElectronMembers(const int &);
  Electron &setDummyElectronMembers(const int &);
  Electron &setRealOrDummyElectronMembers(const int &);
  const Electron &getElectron(const int &) const;
  const Electron &getDummyElectron(const int &) const;
  const Electron &getRealOrDummyElectron(const int &) const;
};
}


#endif //ANALYSIS_ELECTRONS_H