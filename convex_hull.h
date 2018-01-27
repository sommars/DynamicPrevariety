// Main function in this unit is NewHull, which takes a set of points and
// returns a vector of cones. These cones are what are used to initialize
// the prevariety algorithm.

#include "prevariety_util.h"

//------------------------------------------------------------------------------
vector<Cone> GetDisjointHalfOpenConesFromPolytope(
   Support &S,
   vector<double> &VectorForOrientation,
   bool Verbose,
   bool FindLowerHullOnly,
   bool FindUpperHullOnly);

//------------------------------------------------------------------------------
vector<Cone> GetDisjointHalfOpenConesFromSignedPolytope(
   Support &S,
   bool Verbose);

//------------------------------------------------------------------------------
C_Polyhedron FindCPolyhedron(Support &S);
