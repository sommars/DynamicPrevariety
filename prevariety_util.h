#include "printer.h"

//------------------------------------------------------------------------------
vector<int> GeneratorToPoint(Generator &g, bool KnockOffLastTerm);

//------------------------------------------------------------------------------
Constraint InequalityToStrictInequality(Constraint &c);

//------------------------------------------------------------------------------
Constraint InequalityToEquation(Constraint &c);

//------------------------------------------------------------------------------
vector<int> ConstraintToPoint(Constraint &c);

//------------------------------------------------------------------------------
double DoubleInnerProduct(vector<int> &V1, vector<double> &V2);

//------------------------------------------------------------------------------
vector<Support> ParseToSupport(string &Input, bool HasSigns);

//------------------------------------------------------------------------------
vector<Support> ParseSupportFile(string &FileName, bool HasSigns);

//------------------------------------------------------------------------------
C_Polyhedron RaysToCone(vector<vector<int> > &Rays);
