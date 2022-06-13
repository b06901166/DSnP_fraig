/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   CirGate() 
	{
		_flag = 0;
		_fi = 0;
		_fi2 = 0;
		_symbol = ""; 
		_fanoutList.clear();
	}
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const = 0;
   virtual unsigned getLineNo() const = 0;
   virtual int getGateID() const = 0;
	void setFanin(CirGate* cig) {_fi = cig; }
	void setFanin2(CirGate* cig) {_fi2 = cig; }
	void pushFanout(CirGate* cig) {_fanoutList.push_back(cig); }
	int getFanoutSize() {return _fanoutList.size(); }
	CirGate* getFanout(int i) {return _fanoutList[i];}
	virtual int getFaninID() const = 0;
	virtual int getFaninID2() const = 0;
	virtual bool getFanininv() const = 0;
	virtual bool getFanininv2() const = 0;
	virtual void setFanininv(bool inv) = 0;
	virtual void setFanininv2(bool inv) = 0;
	CirGate* getFanin() const {return _fi;}
	CirGate* getFanin2() const {return _fi2;}
	void writeSym(string sym) {_symbol = sym; }
	string getSym() const {return _symbol; }
	void setflag1() const {_flag = 1; }
	void setflag2() const {_flag = 2; }
	void resetflag() const {_flag = 0; }
	int flag() const {return _flag; }
	void printTab(int k) const ;
	void printFanin(int level, int max, bool inv) const ;
	void printFanout(int level, int max, bool inv) const;
	void dfs(bool in, int level, int search_level) const ;
	void dfs(size_t& num) const ;
	void dfsNoPrint(bool in) const ;
	void dfs_search() const; 
	void delFanout(int j) ;
	void delFanout(CirGate* gate) ;
	virtual bool isAig() const = 0;

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

private:
	string                   _symbol;
	CirGate*            	 	  	  _fi;
	CirGate*					       _fi2;
	vector<CirGate*>     _fanoutList;
protected:
	mutable int 					_flag;

};

class CirPiGate : public CirGate
{
public:
	CirPiGate(int g, int l)
	{
		_gateID = g;
		_lineID = l;
	}
	~CirPiGate() {}
	string getTypeStr() const {return "PI";}
	void printGate() const {};
	int getGateID() const {return _gateID; }
	int getFaninID() const {return 0; }
	int getFaninID2() const {return 0; }
	bool getFanininv() const {return false; }
	bool getFanininv2() const {return false; }
	unsigned getLineNo() const {return _lineID; }
	bool isAig() const {return false; }
	void setFanininv(bool inv) {};
	void setFanininv2(bool inv) {};
private:
	int _gateID;
	int _lineID;
};

class CirPoGate : public CirGate
{
public:
	CirPoGate(int g, int in, bool inv, int l)
	{
		_gateID = g;
		_fanin = in;
		_isinv = inv;
		_lineID = l;
	}
	~CirPoGate() {}
	string getTypeStr() const {return "PO";}
	void printGate() const {};
	int getGateID() const {return _gateID; }
	int getFaninID() const {return _fanin; }
	int getFaninID2() const {return 0; }
	bool getFanininv() const {return _isinv; }
	bool getFanininv2() const {return false; }
	unsigned getLineNo() const {return _lineID; }
	bool isAig() const {return false; }
	void setFanininv(bool inv) {_isinv = inv;};
	void setFanininv2(bool inv) {};
private:
	int _gateID;
	int _fanin;
	bool _isinv;
	int _lineID;
};

class CirAigGate : public CirGate
{
public:
	CirAigGate(int g, int in1, bool inv1, int in2, bool inv2, int l)
	{
		_gateID = g;
		_fanin1 = in1;
		_ininv1 = inv1;
		_fanin2 = in2;
		_ininv2 = inv2;
		_lineID = l;
	}
	~CirAigGate() {}
	string getTypeStr() const {return "AIG";}
	void printGate() const {};
	int getGateID() const {return _gateID; }
	int getFaninID() const {return _fanin1; }
	int getFaninID2() const {return _fanin2; }
	bool getFanininv() const {return _ininv1; }
	bool getFanininv2() const {return _ininv2; }
	unsigned getLineNo() const {return _lineID; }
	bool isAig() const {return true; }
	void setFanininv(bool inv) {_ininv1 = inv;};
	void setFanininv2(bool inv) {_ininv2 = inv;};
private:
	int _gateID;
	int _fanin1;
	bool _ininv1;
	int _fanin2;
	bool _ininv2;
	int _lineID;
};

class CirundefGate : public CirGate
{
public:
	CirundefGate(int g, int l)
	{
		_gateID = g;
		_lineID = l;
	}
	~CirundefGate(){}
	string getTypeStr() const {return "UNDEF";}
	void printGate() const {};
	int getGateID() const {return _gateID; }
	int getFaninID() const {return 0; }
	int getFaninID2() const {return 0; }
	bool getFanininv() const {return false; }
	bool getFanininv2() const {return false; }
	unsigned getLineNo() const {return _lineID; }
	bool isAig() const {return false; }
	void setFanininv(bool inv) {};
	void setFanininv2(bool inv) {};
private:
	int _gateID;
	int _lineID;
};

class CirConstGate : public CirGate
{
public:
	CirConstGate()
	{
		_gateID = 0;
		
	}
	~CirConstGate(){}
	string getTypeStr() const {return "CONST";}
	void printGate() const {};
	int getGateID() const {return _gateID; }
	int getFaninID() const {return 0; }
	int getFaninID2() const {return 0; }
	bool getFanininv() const {return false; }
	bool getFanininv2() const {return false; }
	unsigned getLineNo() const {return 0; }
	bool isAig() const {return false; }
	void setFanininv(bool inv) {};
	void setFanininv2(bool inv) {};
private:
	int _gateID;
};
#endif // CIR_GATE_H

