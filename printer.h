#include "prevariety_types.h"
#include "prevariety_inlines.h"

//------------------------------------------------------------------------------
void PrintPoint(vector<int> &Point);

//------------------------------------------------------------------------------
void PrintPoints(vector<vector<int> > &Points);

//------------------------------------------------------------------------------
void PrintSupport(Support &S);

//------------------------------------------------------------------------------
void PrintPoint(set<int> &Point);

//------------------------------------------------------------------------------
void PrintPoint(vector<bool> &Point);

//------------------------------------------------------------------------------
void PrintPointForPython(vector<int> &Point);

//------------------------------------------------------------------------------
void PrintPointsForPython(vector<vector<int> > &Points);

//------------------------------------------------------------------------------
void PrintMaximalCones(TropicalPrevariety &TP, stringstream &s);

//------------------------------------------------------------------------------
void StreamPoint(set<int> &Point, stringstream &s);

//------------------------------------------------------------------------------
void PrintRT(vector<BitsetWithCount> &RT);

//------------------------------------------------------------------------------
void StreamRayToIndexMap(TropicalPrevariety &TP, stringstream &s);

//------------------------------------------------------------------------------
void StreamRayToIndexMapGfan(TropicalPrevariety &TP, stringstream &s);

//------------------------------------------------------------------------------
void StreamRayToIndexMapPolymake(TropicalPrevariety &TP, stringstream &s);

//------------------------------------------------------------------------------
void GetConesAndFvectorForGfan(TropicalPrevariety &TP, stringstream &ConeStream, stringstream &MultiplicitiesStream, stringstream &FvectorStream);

//------------------------------------------------------------------------------
void StreamMaximalConesPolymake(TropicalPrevariety &TP, stringstream &s);

