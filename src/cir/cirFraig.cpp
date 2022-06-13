/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
   vector<vector<CirGate*> > table;
   table.clear();
   for (unsigned int i = 0; i < 3 * _gatelist.size(); ++i)
   {
      vector<CirGate*> tt;
      tt.clear();
      table.push_back(tt);
      for (unsigned int j = 0; j < 3 * _gatelist.size(); ++j)
      {
         table[i].push_back(0);
      }
   }
   bool end = true;
   while (end)
   {
      end = findMerge(table);
		cerr << "**********end find merge********" << endl;
   }
}


bool
CirMgr::findMerge(vector<vector<CirGate*> > &table)
{
   bool end = false;
   for (unsigned int i = 0; i <  _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "AIG")
      {
			//cerr << i << endl;
         int fi1 = _gatelist[i]->getFanin()->getGateID() * 2;
         if (_gatelist[i]->getFanininv()) {fi1 += 1;}
         int fi2 = _gatelist[i]->getFanin2()->getGateID() * 2;
         if (_gatelist[i]->getFanininv2()) {fi2 += 1;}
			if (table[fi1][fi2] == 0) {table[fi1][fi2] = _gatelist[i];}
         if (table[fi2][fi1] == 0) {table[fi1][fi2] = _gatelist[i];}
         if (table[fi1][fi2] != 0 && table[fi1][fi2] != _gatelist[i]) 
			{
				cerr << "******do merge******" << endl;
				mergeGate(table[fi1][fi2], _gatelist[i]); 
				end = true;
            continue;
			}
         if (table[fi2][fi1] != 0 && table[fi2][fi1] != _gatelist[i])
         {
            cerr << "******do merge******" << endl;
				mergeGate(table[fi2][fi1], _gatelist[i]); 
				end = true;
         }
			//cerr << "wwtf" << endl;
      }
   }
   return end;
}

void
CirMgr::mergeGate(CirGate* c1, CirGate* c2)
{
   for (int i = 0; i < c2->getFanoutSize(); ++i)
   {
      c1->pushFanout(c2->getFanout(i));
      if (c2->getFanout(i)->getFanin() != 0 && c2->getFanout(i)->getFanin() == c2)
      {
         c2->getFanout(i)->setFanin(c1);
      }
      else if (c2->getFanout(i)->getFanin2() != 0 && c2->getFanout(i)->getFanin2() == c2)
      {
         c2->getFanout(i)->setFanin2(c1);
      }
   }
   c2->getFanin()->delFanout(c2);
   c2->getFanin2()->delFanout(c2);
   _outlist.push_back(c2);
   delGate(c2);
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/