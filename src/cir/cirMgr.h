/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr();
   ~CirMgr();
   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const ;

   // Member functions about circuit construction
   bool readCircuit(const string&);

   bool readhead(fstream &fin);
   bool read_symbol(fstream &fin);
   bool read_comment(fstream &fin);
   bool checkdif(int i);
   void connect();

   void count_PI();
   void count_AIG();
   
   // Member functions about circuit optimization
   void sweep();
   void optimize();
   void constOpt();
   bool isConstOpt();
   void OPTM();
   bool isOPTM();
   void delGate(int i);
   void delGate(CirGate* gate);

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();
   void mergeGate(CirGate* c1, CirGate* c2);
   bool findMerge(vector<vector<CirGate*> > &table);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printgate(CirGate* cig, int* count) const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;
   void resetAllFlag() const;

   // Member functions about my HW6


private:
   ofstream           *_simLog;
   size_t _M;
   size_t _I;
   size_t _L;
   size_t _O;
   size_t _A;
   vector<CirGate*>  _gatelist;
   vector<int>        _diflist;
   vector<int>      _undeflist;
   CirGate*         _constGate;
   vector<CirGate*>   _outlist;
};

#endif // CIR_MGR_H
