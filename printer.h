#include "prevariety_types.h"
#include "prevariety_inlines.h"

//------------------------------------------------------------------------------
void PrintPoint(vector<int> &Point);

//------------------------------------------------------------------------------
void PrintPoints(vector<vector<int> > &Points);

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
