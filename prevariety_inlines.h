// Various functions that ought to be inlined

//------------------------------------------------------------------------------
inline C_Polyhedron IntersectCones(C_Polyhedron ph1, C_Polyhedron &ph2)
{
   // Intersect C_Polyhedron
   ph1.add_constraints(ph2.constraints());
   ph1.affine_dimension();
   return ph1;
};

//------------------------------------------------------------------------------
inline NNC_Polyhedron IntersectCones(NNC_Polyhedron ph1, NNC_Polyhedron &ph2)
{
   // Intersect C_Polyhedron
   ph1.add_constraints(ph2.constraints());
   ph1.affine_dimension();
   return ph1;
};

//------------------------------------------------------------------------------
inline Support FindInitialForm(
   Support &S, vector<int> &Vector)
{
   // Computes the initial form of a vector and a set of points.
   if (S.Pts.size() == 0)
      return S;
   Support InitialForm;

   SupportPoint *Point;
   Point = &(S.Pts[0]);
   InitialForm.Pts.push_back(*Point);
   int MinimalIP = 0;
   for (size_t i = 0; i != (*Point).Pt.size(); i++)
      MinimalIP += Vector[i] * (*Point).Pt[i];

   for (size_t i = 1; i != S.Pts.size(); i++)
   {
      Point = &(S.Pts[i]);
      int IP = 0;
      for (size_t j = 0; j != (*Point).Pt.size(); j++)
         IP += Vector[j] * (*Point).Pt[j];
      if (MinimalIP > IP)
      {
         MinimalIP = IP;
         InitialForm.Pts.clear();
         InitialForm.Pts.push_back(*Point);
      } else if (IP == MinimalIP)
         InitialForm.Pts.push_back(*Point);
   };
   
   return InitialForm;
};

//------------------------------------------------------------------------------
inline set<int> IntersectSets(set<int> &S1, set<int> &S2)
{
   // Computes and returns the intersection of two sets.
   set<int>::iterator S1Itr = S1.begin();
   set<int>::iterator S2Itr = S2.begin();
   set<int> Result;
   while ((S1Itr != S1.end()) && (S2Itr != S2.end()))
   {
      if (*S1Itr < *S2Itr)
         ++S1Itr;
      else if (*S2Itr<*S1Itr)
         ++S2Itr;
      else 
      {
         Result.insert(*S1Itr);
         S1Itr++;
         S2Itr++;
      };
   };
   return Result;
};

//------------------------------------------------------------------------------
inline bool SetsDoIntersect(set<int> &S1, set<int> &S2)
{
   // Tests if two sets have a non-trivial intersection
   set<int>::iterator S1Itr = S1.begin();
   set<int>::iterator S2Itr = S2.begin();
   while ((S1Itr != S1.end()) && (S2Itr != S2.end()))
   {
      if (*S1Itr < *S2Itr)
         ++S1Itr;
      else if (*S2Itr<*S1Itr)
         ++S2Itr;
      else
         return true;
   };
   return false;
};

//------------------------------------------------------------------------------
inline BitsetWithCount IntersectRTs(BitsetWithCount R1, BitsetWithCount &R2)
{
   // Intersects two relation tables and correctly establishes the 
   // count property
   R1.Indices = R1.Indices&=R2.Indices;
   R1.Count = R1.Indices.count();
   return R1;
};

//------------------------------------------------------------------------------
inline BitsetWithCount ORRTs(BitsetWithCount R1, BitsetWithCount &R2)
{
   // ORs two relation tables and correctly establishes the 
   // count property
   R1.Indices = R1.Indices|=R2.Indices;
   R1.Count = R1.Indices.count();
   return R1;
};

//------------------------------------------------------------------------------
inline vector<BitsetWithCount> ORFullRTS(vector<BitsetWithCount> R1, vector<BitsetWithCount> &R2, BitsetWithCount PolytopesVisited)
{
   for (size_t i = 0; i != R1.size(); i++)
   {
      R1[i] = ORRTs(R1[i],R2[i]);
   };
   return R1;
};

//------------------------------------------------------------------------------
inline bool Set1IsSubsetOfSet2(set<int> &S1, set<int> &S2)
{
   // Tests if S1 is a subset of S2;
   set<int>::iterator S1Itr = S1.begin();
   set<int>::iterator S2Itr = S2.begin();
   while ((S1Itr != S1.end()) && (S2Itr != S2.end()))
   {
      if (*S1Itr < *S2Itr)
         return false;
      else if (*S2Itr<*S1Itr)
         ++S2Itr;
      else 
      {
         S1Itr++;
         S2Itr++;
      };
   };
   
   // If S2 is at the end and there still are elements in S1, then false
   if ((S1Itr != S1.end()) && (S2Itr == S2.end()))
      return false;
   else
      return true;
}
