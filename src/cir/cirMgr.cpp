/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
void
CirMgr::resetAllFlag() const
{
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      _gatelist[i]->resetflag();
   }
}
CirGate* 
CirMgr::getGate(unsigned gid) const 
{
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      _gatelist[i]->resetflag();
   }
   int idx = gid;
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getGateID() == idx)
      {
         if (_gatelist[i]->getTypeStr() == "UNDEF"){return 0;}
         return _gatelist[i];
      }
   }
   return 0;
}

CirMgr::CirMgr()
{
   _M = _I = _L = _O = _A = 0;
   CirGate* CONST = new CirConstGate();
   _constGate = CONST;
}

CirMgr::~CirMgr() {}

bool CirMgr::read_symbol(fstream &fin)
{
   string sym;
   string nick;
   string prev;
   int id;
   sym = "gg";
   while (sym != "" || sym != "c")
   {
      fin >> sym; fin >> nick;
      if (sym == prev){return true; }
      prev = sym;
      //cout << sym << ' ' << nick << "\n";
      if (sym == "" || sym == "c") {return true; }
      else if ((sym[0] == 'i'))
      {
         sym = sym.substr(1);
         myStr2Int(sym, id);
         id++;
         for (unsigned int i = 0; i < _gatelist.size(); ++i)
         {
            if (_gatelist[i]->getGateID() == id && _gatelist[i]->getTypeStr() == "PI")
            {
               _gatelist[i]->writeSym(nick);
            }
         }
      }
      else if ((sym[0] == 'o'))
      {
         sym = sym.substr(1);
         myStr2Int(sym, id);
         id++;
         for (unsigned int i = 0; i < _gatelist.size(); ++i)
         {
            int idx = id + _M;
            if (_gatelist[i]->getGateID() == idx && _gatelist[i]->getTypeStr() == "PO")
            {
               _gatelist[i]->writeSym(nick);
            }
         }
      }
   }
   return true;
}

bool CirMgr::read_comment(fstream &fin)
{
   return true;
}

bool CirMgr::checkdif(int j)
{
   for (unsigned int i = 0; i < _diflist.size(); ++i)
   {
      if (j == _diflist[i]){return true;}
   }
   for (unsigned int i = 0; i < _undeflist.size(); ++i)
   {
      if (j == _undeflist[i]){return false;}
   }
   _undeflist.push_back(j);
   return false;
}

bool
CirMgr::readCircuit(const string& fileName)
{
   fstream fin(fileName);
   if (!fin.is_open()){return false; }
   string aag;
   int count_line = 1;
   fin >> aag;
   fin >> _M; fin >> _I; fin >> _L; fin >> _O; fin >> _A;
   int id;
   for (unsigned int i = 0; i < _I; ++i)
   {
      fin >> id;
      count_line++;
      CirGate* PI = new CirPiGate(id/2, count_line);
      _gatelist.push_back(PI);
      _diflist.push_back(id/2);
   }
   bool inv;
   for (unsigned int i = 0; i < _O; ++i)
   {
      fin >> id;
      count_line++;
      if (id % 2 == 0){inv = false; }
      else {inv = true; }
      CirGate* PO = new CirPoGate(_M + i + 1, id/2, inv, count_line);
      _gatelist.push_back(PO);
      checkdif(id/2);     
   }
   int in1;
   int in2;
   bool inv1;
   bool inv2;
   for (unsigned int i = 0; i < _A; ++i)
   {
      fin >> id; fin >> in1; fin >> in2;
      count_line++;
      if (in1 % 2 == 0){inv1 = false; }
      else {inv1 = true; }
      if (in2 % 2 == 0){inv2 = false; }
      else {inv2 = true; }
      CirGate* AIG = new CirAigGate(id/2, in1/2, inv1, in2/2, inv2, count_line);
      _gatelist.push_back(AIG);
      _diflist.push_back(id/2);
      for (unsigned int i = 0; i < _undeflist.size(); ++i)
      {
         if (id/2 == _undeflist[i]){_undeflist.erase(_undeflist.begin()+i); break;}
      }
      checkdif(in1/2);
      checkdif(in2/2);
   }
   read_symbol(fin);
   read_comment(fin);
   fin.close();
   for (unsigned int i = 0; i < _undeflist.size(); ++i)
   {
      CirGate* UND = new CirundefGate(_undeflist[i], 0);
      _gatelist.push_back(UND);
   }
   
   connect();
   return true;
}
void
CirMgr::connect()
{
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PI"){continue; }
      else if (_gatelist[i]->getTypeStr() == "PO")
      {
         for (unsigned int j = 0; j < _gatelist.size(); ++j)
         {
            if (_gatelist[i]->getFaninID() == _gatelist[j]->getGateID())
            {
               _gatelist[i]->setFanin(_gatelist[j]);
               _gatelist[j]->pushFanout(_gatelist[i]);
            }
         }
      }
      else if (_gatelist[i]->getTypeStr() == "AIG")
      {
         for (unsigned int j = 0; j < _gatelist.size(); ++j)
         {
            if (_gatelist[i]->getFaninID() == _gatelist[j]->getGateID())
            {
               _gatelist[i]->setFanin(_gatelist[j]);
               _gatelist[j]->pushFanout(_gatelist[i]);
            }
            if (_gatelist[i]->getFaninID2() == _gatelist[j]->getGateID())
            {
               _gatelist[i]->setFanin2(_gatelist[j]);
               _gatelist[j]->pushFanout(_gatelist[i]);
            }
         }
      }
   }
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout << "\n";
   cout << "Circuit Statistics" << "\n";
   cout << "==================" << "\n";
   cout << setw(7) << left << "  PI" << setw(9) << right << _I << endl;
   cout << setw(7) << left << "  PO" << setw(9) << right << _O << endl;
   cout << setw(7) << left << "  AIG" << setw(9) << right << _A << endl;
   cout << "------------------" << "\n";
   cout << setw(7) << left << "  Total" << setw(9) << right << _I + _O + _A << endl;
}


void
CirMgr::printNetlist() const
{

   cout << endl;
   resetAllFlag();
   size_t num = 0;
   for (unsigned i = 0, n = _gatelist.size(); i < n; ++i) 
   {
      if (_gatelist[i]->getTypeStr() == "PO"){_gatelist[i]->dfs(num);}
   }

}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PI"){cout << ' ' << _gatelist[i]->getGateID();}
   }
   cout << "\n";
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PO"){cout << ' ' << _gatelist[i]->getGateID();}
   }
   cout << "\n";
}

void
CirMgr::printFloatGates() const
{
   if (_undeflist.size() != 0){cout << "Gates with floating fanin(s):";}
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getFanin() != 0)
      {
         if (_gatelist[i]->getFanin()->getTypeStr() == "UNDEF") {cout << ' ' << _gatelist[i]->getGateID();}
      }
      else if (_gatelist[i]->getFanin2() != 0)
      {
         if (_gatelist[i]->getFanin2()->getTypeStr() == "UNDEF") {cout << ' ' << _gatelist[i]->getGateID();}
      }
   }
   if (_undeflist.size() != 0){cout << "\n";}
   vector<int> thelist;
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getFanoutSize() == 0 && _gatelist[i]->getTypeStr() != "PO")
      {
         thelist.push_back(_gatelist[i]->getGateID());
      }
   }
   if (thelist.size() == 0){return ; }
   cout << "Gates defined but not used  :";
   std::sort(thelist.begin(),thelist.end());
   for (unsigned int i = 0; i < thelist.size(); ++i)
   {
      cout << ' ' << thelist[i];
   }
   cout << "\n";
}

void
CirMgr::printFECPairs() const
{
}

void
CirMgr::writeAag(ostream& outfile) const
{
   int a = _A;
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "AIG" && _gatelist[i]->getFanoutSize() == 0)
      {
         a--;
      }
   }
   outfile << "aag ";
   outfile << _M << ' ' << _I << ' ' << _L << ' ' << _O << ' ' << a << "\n";
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PI")
      {
         outfile << _gatelist[i]->getGateID() * 2 << "\n";
      }
   }
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PO")
      {
         if (!(_gatelist[i]->getFanininv())){outfile << _gatelist[i]->getFanin()->getGateID() * 2 << "\n";}
         else if (_gatelist[i]->getFanininv()){outfile << _gatelist[i]->getFanin()->getGateID() * 2 + 1 << "\n";}  
      }
   }
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "AIG" && _gatelist[i]->getFanoutSize() != 0)
      {
         outfile << _gatelist[i]->getGateID() * 2 << ' ';
         if (!(_gatelist[i]->getFanininv())){outfile << _gatelist[i]->getFanin()->getGateID() * 2 << ' ';}
         else if (_gatelist[i]->getFanininv()){outfile << _gatelist[i]->getFanin()->getGateID() * 2 + 1 << ' ';}
         if (!(_gatelist[i]->getFanininv2())){outfile << _gatelist[i]->getFanin2()->getGateID() * 2 << "\n";}
         else if (_gatelist[i]->getFanininv2()){outfile << _gatelist[i]->getFanin2()->getGateID() * 2 + 1 << "\n";}  
      }      
   }
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PI" && _gatelist[i]->getSym() != "")
      {
         outfile << 'i' << _gatelist[i]->getGateID() - 1 << ' ' << _gatelist[i]->getSym() << "\n";
      }
   }
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PO" && _gatelist[i]->getSym() != "")
      {
         outfile << 'o' << _gatelist[i]->getGateID() - 1 << ' ' << _gatelist[i]->getSym() << "\n";
      }
   }
   outfile << 'c' << "\n";
   outfile << "no comment. :)" << "\n";
}



void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}



void
CirMgr::delGate(int i)
{
   _gatelist.erase(_gatelist.begin() + i);
}

void
CirMgr::delGate(CirGate* gate)
{
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i] == gate){delGate(i); return ;}
   }
}


void
CirMgr::count_PI()
{
   size_t count = 0;
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "PI"){count++;}
   }
   _I = count;
}

void
CirMgr::count_AIG()
{
   size_t count = 0;
   for (unsigned int i = 0; i < _gatelist.size(); ++i)
   {
      if (_gatelist[i]->getTypeStr() == "AIG"){count++;}
   }
   _A = count;
}


