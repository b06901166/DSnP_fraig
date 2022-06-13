/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   cout << "==================================================" << "\n";
   stringstream sout;
   sout << "= " << getTypeStr() << "(" << getGateID() << "), line " << getLineNo();
   string ss = sout.str();
   ss.resize(49,' ');
   cout << ss;
   cout << '=' << "\n";
   cout << "==================================================" << "\n";
}

void 
CirGate::printTab(int k) const
{
   for (int i = 0; i < k; ++i){cout << "  " ;}
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   dfs(true, level, 0);
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   dfs(false, level, 0);
}

void
CirGate::dfs(bool in, int level, int searched_level) const
{
   cout << getTypeStr() << " " << getGateID();
   // if(is_marked() && searched_level != level && getTypeInt() != PI_GATE && getTypeInt() != CONST_GATE) { cout << " (*)" << endl; return; }
   // else cout << endl;
   int count = 0;
   bool no_dfs = false;
   if(in)
   {
      if (getFanin() != 0){count++;}
      if (getFanin2() != 0){count++;}
   }
   else count = _fanoutList.size();
   if(!count) no_dfs = true;
   if((!no_dfs) && flag() && getTypeStr() == "AIG" && searched_level != level) { cout << " (*)" << endl; return; }
   else cout << endl;

   if(searched_level == level) return;
   setflag1();
   
   if(in) 
   {
      if (getFanin() != 0)
      {
         for(int j = 0; j < searched_level + 1; ++j) cout << "  ";
         if (getFanininv()) {cout << "!";}
         getFanin()->dfs(in, level, searched_level + 1);
      }
      if (getFanin2() != 0)
      {
         for(int j = 0; j < searched_level + 1; ++j) cout << "  ";
         if (getFanininv2()) {cout << "!";}
         getFanin2()->dfs(in, level, searched_level + 1);
      }
   }
   else
   {
      for(size_t i = 0; i < _fanoutList.size(); ++i)
      {
         for(int j = 0; j < searched_level + 1; ++j) cout << "  ";
         if (_fanoutList[i]->getFanin() != 0 && _fanoutList[i]->getFanin()->getGateID() == getGateID())
         {
            if (_fanoutList[i]->getFanininv()){cout << "!";}
         }
         if (_fanoutList[i]->getFanin2() != 0 && _fanoutList[i]->getFanin2()->getGateID() == getGateID())
         {
            if (_fanoutList[i]->getFanininv2()){cout << "!";}
         }
         _fanoutList[i]->dfs(in, level, searched_level + 1);
      }
   }
}

void
CirGate::dfs(size_t& num) const
{
   if(getTypeStr() == "UNDEF") return;
   if(getFanin() != 0 && !getFanin()->flag()) {getFanin()->dfs(num); }
   if(getFanin2() != 0 && !getFanin2()->flag()) {getFanin2()->dfs(num); }
   cout << "[" << num << "] " << setw(4) << left << getTypeStr() << getGateID();
   if(getTypeStr() == "AIG" || getTypeStr() == "PO")
   {
      if (getFanin() != 0)
      {
         cout << " ";
         if (getFanin()->getTypeStr() == "UNDEF"){cout << "*";}
         if (getFanininv()) {cout << "!";}
         cout << getFanin()->getGateID();
      }
      if (getFanin2() != 0 ) 
      {
         cout << " ";
         if (getFanin2()->getTypeStr() == "UNDEF"){cout << "*";}
         if (getFanininv2()) {cout << "!";}
         cout << getFanin2()->getGateID();
      }      
   }
   if(getSym().size() != 0) {cout << " (" << getSym() << ")";}
   cout << endl;
   setflag1();
   ++num;
}

void
CirGate::dfsNoPrint(bool in) const
{
   int count = 0;
   bool no_dfs = false;
   if(in)
   {
      if (getFanin() != 0){count++;}
      if (getFanin2() != 0){count++;}
   }
   else count = _fanoutList.size();
   if(!count) no_dfs = true;
   if((!no_dfs) && flag() && getTypeStr() == "AIG") {return; }
   setflag1();
   
   if(in) 
   {
      if (getFanin() != 0)
      {
         getFanin()->dfsNoPrint(in);
      }
      if (getFanin2() != 0)
      {
         getFanin2()->dfsNoPrint(in);
      }
   }
   else
   {
      for(size_t i = 0; i < _fanoutList.size(); ++i)
      {
         _fanoutList[i]->dfsNoPrint(in);
      }
   }
}

void 
CirGate::delFanout(int j)
{
   _fanoutList.erase(_fanoutList.begin() + j);
}

void
CirGate::delFanout(CirGate* gate)
{
   for (unsigned int i = 0; i < _fanoutList.size(); ++i)
   {
      if (_fanoutList[i] == gate)
      {
         delFanout(i);
      }
   }
}
