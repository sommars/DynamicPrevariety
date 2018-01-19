// Main function in this unit is NewHull, which takes a set of points and
// returns a vector of cones. These cones are what are used to initialize
// the prevariety algorithm.

#include "prevariety_util.h"

//------------------------------------------------------------------------------
vector<Cone> NewHull(
   Support &S,
   vector<double> &VectorForOrientation,
   bool Verbose,
   bool FindLowerHullOnly,
   bool FindUpperHullOnly);

//------------------------------------------------------------------------------
void FindFacets(Hull &H);

//------------------------------------------------------------------------------
void FindEdges(Hull &H);

//------------------------------------------------------------------------------
vector<vector<int> > FindCandidateEdges(Hull &H);

//------------------------------------------------------------------------------
C_Polyhedron FindCPolyhedron(Support &S);
