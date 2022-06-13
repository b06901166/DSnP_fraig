/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
	cerr << "sweep" << endl;
   resetAllFlag();
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PO")
      {
         _gatelist[i]->dfsNoPrint(true);
      }
   }
   _constGate->setflag1();
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->flag() == 0)
      {
			if (_gatelist[i]->getFanin() != 0)
			{
				for (int j = 0; j < _gatelist[i]->getFanin()->getFanoutSize(); ++j)
				{
					if (_gatelist[i]->getFanin()->getFanout(j)->getGateID() == _gatelist[i]->getGateID())
					{
						_gatelist[i]->getFanin()->delFanout(j);
						//break;
					}
				}
			}
			if (_gatelist[i]->getFanin2() != 0)
			{
				for (int j = 0; j < _gatelist[i]->getFanin2()->getFanoutSize(); ++j)
				{
					if (_gatelist[i]->getFanin2()->getFanout(j)->getGateID() == _gatelist[i]->getGateID())
					{
						_gatelist[i]->getFanin2()->delFanout(j);
						//break;
					}
				}
			}
			cout << "Sweeping: " << _gatelist[i]->getTypeStr() << '(' << _gatelist[i]->getGateID() << ") removed..." << endl;
         _gatelist.erase(_gatelist.begin() + i);
         --i;
      }
   }
	count_PI();
	count_AIG();
	resetAllFlag();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
	if (!isConstOpt()){constOpt(); }
	if (!isOPTM()){OPTM();}
	if (!isConstOpt() || !isOPTM()) {optimize();}
	count_AIG();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

void
CirMgr::constOpt()
{
	for (int i = 0; i < _constGate->getFanoutSize(); ++i)
	{
		int fi = 0;
		if (_constGate->getFanout(i)->getTypeStr() != "PO")
		{
			if (_constGate->getFanout(i)->getFanin() == _constGate){fi = 1;}
			else {fi = 2;}
		}
		if (fi == 1)
		{
			if (_constGate->getFanout(i)->getFanininv()) // fanin1 == 0
			{
				_constGate->getFanout(i)->getFanin2()->delFanout(_constGate->getFanout(i));
				for (int j = 0; j < _constGate->getFanout(i)->getFanoutSize(); ++j)
				{
					_constGate->pushFanout(_constGate->getFanout(i)->getFanout(j));
					if (_constGate->getFanout(i)->getFanout(j)->getFanin() == _constGate->getFanout(i)) 
					{
						_constGate->getFanout(i)->getFanout(j)->setFanin(_constGate);
					}
					if (_constGate->getFanout(i)->getFanout(j)->getFanin2() == _constGate->getFanout(i)) 
					{
						_constGate->getFanout(i)->getFanout(j)->setFanin2(_constGate);
					}
					_constGate->pushFanout(_constGate->getFanout(i)->getFanout(j));
				}
				delGate(_constGate->getFanout(i));
				_constGate->delFanout(_constGate->getFanout(i));
				--i;
			}
			else //fanin1 == 1
			{
				CirGate* x = _constGate->getFanout(i)->getFanin2();
				for (int j = 0; j < _constGate->getFanout(i)->getFanoutSize(); ++j)
				{
					if (_constGate->getFanout(i)->getFanout(j)->getFanin() == _constGate->getFanout(i)) 
					{
						_constGate->getFanout(i)->getFanout(j)->setFanin(x);
					}
					if (_constGate->getFanout(i)->getFanout(j)->getFanin2() == _constGate->getFanout(i)) 
					{
						_constGate->getFanout(i)->getFanout(j)->setFanin2(x);
					}
					x->pushFanout(_constGate->getFanout(i)->getFanout(j));
				}
				delGate(_constGate->getFanout(i));
				x->delFanout(_constGate->getFanout(i));
			}
		}
		else if (fi == 2)
		{
			if (_constGate->getFanout(i)->getFanininv2()) //fanin2 == 0
			{
				_constGate->getFanout(i)->getFanin()->delFanout(_constGate->getFanout(i));
				for (int j = 0; j < _constGate->getFanout(i)->getFanoutSize(); ++j)
				{
					_constGate->pushFanout(_constGate->getFanout(i)->getFanout(j));
					if (_constGate->getFanout(i)->getFanout(j)->getFanin() == _constGate->getFanout(i)) 
					{
						_constGate->getFanout(i)->getFanout(j)->setFanin(_constGate);
					}
					if (_constGate->getFanout(i)->getFanout(j)->getFanin2() == _constGate->getFanout(i)) 
					{
						_constGate->getFanout(i)->getFanout(j)->setFanin2(_constGate);
					}
					_constGate->pushFanout(_constGate->getFanout(i)->getFanout(j));
				}
				delGate(_constGate->getFanout(i));
				_constGate->delFanout(_constGate->getFanout(i));
				--i;
			}
			else // fanin2 == 1
			{
				CirGate* x = _constGate->getFanout(i)->getFanin();
				for (int j = 0; j < _constGate->getFanout(i)->getFanoutSize(); ++j)
				{
					if (_constGate->getFanout(i)->getFanout(j)->getFanin() == _constGate->getFanout(i)) 
					{
						_constGate->getFanout(i)->getFanout(j)->setFanin(x);
					}
					if (_constGate->getFanout(i)->getFanout(j)->getFanin2() == _constGate->getFanout(i)) 
					{
						_constGate->getFanout(i)->getFanout(j)->setFanin2(x);
					}
					x->pushFanout(_constGate->getFanout(i)->getFanout(j));
				}
				delGate(_constGate->getFanout(i));
				x->delFanout(_constGate->getFanout(i));
			}
		}
	}
	if (!isConstOpt()){constOpt();}
}

bool
CirMgr::isConstOpt()
{
	for (int i = 0; i < _constGate->getFanoutSize(); ++i)
	{
		if (_constGate->getFanout(i)->getTypeStr() == "AIG"){return false;}
	}
	return true;
}

void
CirMgr::OPTM()
{
	for (unsigned int i = 0; i < _gatelist.size(); ++i)
	{
		if (_gatelist[i]->getTypeStr() == "AIG" && _gatelist[i]->getFanin() == _gatelist[i]->getFanin2())
		{
			if (_gatelist[i]->getFanininv() != _gatelist[i]->getFanininv2()) // 0 && 1
			{
				for (unsigned int j = 0; j < _gatelist[i]->getFanoutSize(); ++j)
				{
					if (_gatelist[i]->getFanout(j)->getFanin() == _gatelist[i])
					{
						_gatelist[i]->getFanout(j)->setFanin(_constGate);
					}
					else if (_gatelist[i]->getFanout(j)->getFanin2() == _gatelist[i])
					{
						_gatelist[i]->getFanout(j)->setFanin2(_constGate);
					}
					_constGate->pushFanout(_gatelist[i]->getFanout(j));
				}
				_gatelist[i]->getFanin()->delFanout(_gatelist[i]);
				_gatelist[i]->getFanin()->delFanout(_gatelist[i]);
				delGate(i);
				--i;
			}
			else
			{
				bool inv = _gatelist[i]->getFanininv();
				CirGate* x = _gatelist[i]->getFanin();
				for (unsigned int j = 0; j < _gatelist[i]->getFanoutSize(); ++j)
				{
					if (_gatelist[i]->getFanout(j)->getFanin() == _gatelist[i])
					{
						_gatelist[i]->getFanout(j)->setFanin(x);
						_gatelist[i]->getFanout(j)->setFanininv(inv ^ _gatelist[i]->getFanout(j)->getFanininv());
					}
					else if (_gatelist[i]->getFanout(j)->getFanin2() == _gatelist[i])
					{
						_gatelist[i]->getFanout(j)->setFanin2(x);
						_gatelist[i]->getFanout(j)->setFanininv2(inv ^ _gatelist[i]->getFanout(j)->getFanininv2());
					}
					x->pushFanout(_gatelist[i]->getFanout(j));
				}
				_gatelist[i]->getFanin()->delFanout(_gatelist[i]);
				_gatelist[i]->getFanin()->delFanout(_gatelist[i]);
				delGate(i);
				--i;
			}
		}
	}
	if (!isOPTM()){OPTM();}
}

bool
CirMgr::isOPTM()
{
	for (int i = 0; i < _gatelist.size(); ++i)
	{
		if (_gatelist[i]->getTypeStr() == "AIG")
		{
			if (_gatelist[i]->getFanin() == _gatelist[i]->getFanin2())
			{
				return false;
			}
		}
	}
	return true;
}
