#include "convex_hull.h"

//------------------------------------------------------------------------------
bool SupportPtSort(SupportPoint i, SupportPoint j)
{
   return (i.IP < j.IP);
}

//------------------------------------------------------------------------------
void FindConesHelper(Hull &H,
   bool FindLowerHullOnly,
   bool FindUpperHullOnly,
   SupportPoint &CurrentPt)
{
   Linear_Expression LowerHullLE;
   LowerHullLE += Variable(0) * -1;
   Linear_Expression UpperHullLE;
   UpperHullLE += Variable(0) * 1;
   for (size_t i = 1; i != H.SpaceDimension; i++)
   {
      LowerHullLE += Variable(i) * 0;
      UpperHullLE += Variable(i) * 0;
   };
   Constraint LowerHullConstraint = LowerHullLE > 0;
   Constraint UpperHullConstraint = UpperHullLE > 0;
   
   vector<Constraint> Constraints;
   int PtIndex = H.PointToIndexMap[CurrentPt];
   // Go through all of the edges. If the edge is not on the facet.
   for (size_t j = 0; j != H.Edges.size(); j++)
   {
      if (H.Edges[j].PointIndices.find(PtIndex) 
          != H.Edges[j].PointIndices.end())
      {
         set<int>::iterator PtIter;
         SupportPoint OtherPt;
         int OtherPtIndex;
         for (PtIter=H.Edges[j].PointIndices.begin();
              PtIter != H.Edges[j].PointIndices.end();
              PtIter++)
         {
            OtherPtIndex = (*PtIter);
            if (OtherPtIndex != PtIndex)
               OtherPt = H.IndexToPointMap[(*PtIter)];
         };
         
         Linear_Expression LE;
         for (size_t k = 0; k != CurrentPt.Pt.size(); k++)
            LE += (OtherPt.Pt[k] - CurrentPt.Pt[k]) * Variable(k);
         
         // Manufacture the constraint from the edge.
         Constraint c;
         if (OtherPtIndex > PtIndex)
            c = (LE >= 0);
         else
            c = (LE > 0);
         Constraints.push_back(c);
      };
   };
   // If no non-strict inequalities exist, then you may simply throw the cone
   // away. This process produces just one half open cone for each edge.
   for (size_t j = 0; j != Constraints.size(); j++)
   {
      // Take one of your non-strict inequalities for the cone.
      Constraint TempConstraint = Constraints[j];
      if (!TempConstraint.is_strict_inequality())
      {
         // Introduce it as an equation to get a lower dimensional cone,
         // which you output.
         Constraints[j] = InequalityToEquation(TempConstraint);

         Constraint_System csOriginalDim;
         for (size_t k = 0; k != Constraints.size(); k++)
         {
            csOriginalDim.insert(Constraints[k]);
         };
         
         if (FindLowerHullOnly)
            csOriginalDim.insert(LowerHullConstraint);
         if (FindUpperHullOnly)
            csOriginalDim.insert(UpperHullConstraint);
            
         Cone NewCone;
         NewCone.HOPolyhedron = NNC_Polyhedron(csOriginalDim);
         NewCone.HOPolyhedron.minimized_constraints();
         NewCone.HOPolyhedron.minimized_generators();
         NewCone.HOPolyhedron.affine_dimension();
         
         // If we are only finding the lower hull, it's very likely that
         // some of the cones will be zero dimensional. Exclude them.
         if ((FindLowerHullOnly || FindUpperHullOnly) && (NewCone.HOPolyhedron.affine_dimension() == 0))
         { // Do nothing
         } else
            H.Cones.push_back(NewCone);

         //Introduce it as a strict inequality to describe the rest.
         // Call recursively.
         Constraints[j] = InequalityToStrictInequality(TempConstraint);
      };
   };
}

//------------------------------------------------------------------------------
void FindCones(Hull &H,
   bool FindLowerHullOnly,
   bool FindUpperHullOnly)
{
   for (size_t i = 0; i != H.S.Pts.size(); i++)
      FindConesHelper(H, FindLowerHullOnly, FindUpperHullOnly, H.S.Pts[i]);
};

//------------------------------------------------------------------------------
void FindFacets(Hull &H)
{
   // Find the Facets by shooting rays at the polytopes
   // spin through all of the constraints. shoot each constraint at the polytope
   Constraint_System cs = H.CPolyhedron.minimized_constraints();
   for (Constraint_System::const_iterator i = cs.begin(), cs_end = cs.end();
        i != cs_end; 
        ++i)
   {
      if (!i->is_inequality())
         continue;
      Constraint C = *i;
      vector<int> Pt = ConstraintToPoint(C);
      Support FacetPts = FindInitialForm(H.S, Pt);
      
      Facet F;
      for (size_t j = 0; j != FacetPts.Pts.size(); j++)
      {
         F.PointIndices.insert(H.PointToIndexMap[FacetPts.Pts[j]]);
      };
      F.Normal = Pt;
      H.Facets.push_back(F);
   }
}

//------------------------------------------------------------------------------
vector<vector<int> > FindCandidateEdges(Hull &H, int NecessaryVertexIndex)
{
   // Helper function for FindEdges. Again, there _has_ to be a better way to
   // do this.
   vector<vector<int> > CandidateEdges;
   int n = H.S.Pts.size();
   vector<int> d(n);
   for (size_t i = 0; i != d.size(); ++i)
      d[i] = i;
   do
   {
      if (d[0] < d[1])
      {
         vector<int> CandidateEdgeIndices;
         if (NecessaryVertexIndex < 0 || (d[0] == NecessaryVertexIndex || d[1] == NecessaryVertexIndex))
         {
            CandidateEdgeIndices.push_back(d[0]);
            CandidateEdgeIndices.push_back(d[1]);
            CandidateEdges.push_back(CandidateEdgeIndices);
         };
      };
      reverse(d.begin()+2, d.end());
   } while (next_permutation(d.begin(), d.end()));
   return CandidateEdges;
}

//------------------------------------------------------------------------------
void FindEdges(Hull &H, int NecessaryVertexIndex)
{
   // Find the set of edges. This could be done in a better way, but this works.
   vector<vector<int> > CandidateEdges = FindCandidateEdges(H, NecessaryVertexIndex);

   vector<vector<int> >::iterator itr;
   for (itr=CandidateEdges.begin(); itr != CandidateEdges.end(); itr++)
   {
      vector<int> CandidateEdge = (*itr);
      int Point1 = CandidateEdge[0];
      int Point2 = CandidateEdge[1];
   
      int FacetCount = 0;
      vector<int> VectorInCone(H.SpaceDimension);
      for (vector<Facet>::iterator FacetIt = H.Facets.begin();
           FacetIt != H.Facets.end();
           FacetIt++)
      {
         set<int> PtIndices = FacetIt->PointIndices;
         bool Point1IsInFacet = PtIndices.find(Point1) != PtIndices.end();
         bool Point2IsInFacet = PtIndices.find(Point2) != PtIndices.end();
         if (Point1IsInFacet and Point2IsInFacet)
         {
            FacetCount++;
            for (size_t i = 0; i != VectorInCone.size(); i++)
               VectorInCone[i] = VectorInCone[i] + FacetIt->Normal[i];
         }
      };
      if ((FacetCount >= (H.AffineDimension - 1))
      && (FindInitialForm(H.S, VectorInCone).Pts.size() == 2))
      {
         Edge NewEdge;
         NewEdge.PointIndices.insert(Point1);
         NewEdge.PointIndices.insert(Point2);
         H.Edges.push_back(NewEdge);
      };
   };
}

//------------------------------------------------------------------------------
C_Polyhedron FindCPolyhedron(Support &S)
{
   // Converts an input set of pts to a PPL C_Polyhedron
   Generator_System gs;
   for (size_t i = 0; i != S.Pts.size(); i++)
   {
      Linear_Expression LE;
      for (size_t j = 0; j != S.Pts[i].Pt.size(); j++)
         LE += Variable(j) * (S.Pts[i].Pt[j]);
      gs.insert(point(LE));
   };
   return C_Polyhedron(gs);
}

//------------------------------------------------------------------------------
vector<Cone> GetDisjointHalfOpenConesFromPolytope(
   Support &S,
   vector<double> &VectorForOrientation, 
   bool Verbose,
   bool FindLowerHullOnly,
   bool FindUpperHullOnly)
{
   // For a given set of points and orientation vector, this function computes
   // the set of half open edge cones for the polytope defined by these points.
   Hull H;
   H.CPolyhedron = FindCPolyhedron(S);
   for (size_t i = 0; i != S.Pts.size(); i++)
   {
      S.Pts[i].IP = DoubleInnerProduct(S.Pts[i].Pt, VectorForOrientation);
   };
   
   sort(S.Pts.begin(), S.Pts.end(), SupportPtSort);
   
   for (size_t i = 0; i != S.Pts.size(); i++)
      H.S.Pts.push_back(S.Pts[i]);

   H.AffineDimension = H.CPolyhedron.affine_dimension();
   H.SpaceDimension = H.CPolyhedron.space_dimension();
      
   // Create PointToIndexMap
   for (size_t i = 0; i != H.S.Pts.size(); i++)
   {
      SupportPoint Point = H.S.Pts[i];
      H.PointToIndexMap[Point]=i;
      H.IndexToPointMap[i]=Point;
   };

   FindFacets(H);
   FindEdges(H, -1);
   FindCones(H, FindLowerHullOnly, FindUpperHullOnly);

   if (Verbose)
   {
      cout << "Convex hull------------------------" << endl;
      PrintSupport(H.S);
      cout << "Affine dimension: " << H.AffineDimension << endl;
      cout << "Space dimension: " << H.SpaceDimension << endl;
      cout << "Number of cones: " << H.Cones.size() << endl;
      cout << "Number of edges: " << H.Edges.size() << endl;
      cout << "Number of facets: " << H.Facets.size() << endl << endl;
   };
   return H.Cones;
}

//------------------------------------------------------------------------------
vector<Cone> GetDisjointHalfOpenConesFromSignedPolytope(
   Support &S,
   bool Verbose)
{
   Hull H;
   H.CPolyhedron = FindCPolyhedron(S);
   H.AffineDimension = H.CPolyhedron.affine_dimension();
   H.SpaceDimension = H.CPolyhedron.space_dimension();

   for (size_t i = 0; i != S.Pts.size(); i++)
      H.S.Pts.push_back(S.Pts[i]);

   int PositiveVertexIndex = -1;
   for (size_t i = 0; i != H.S.Pts.size(); i++)
   {
      SupportPoint Point = H.S.Pts[i];
      H.PointToIndexMap[Point]=i;
      H.IndexToPointMap[i]=Point;
      if (Point.Sign == PLUS)
         PositiveVertexIndex = i;
   };
   
   FindFacets(H);
   
   vector<double> VectorForOrientation(H.CPolyhedron.space_dimension());
   bool PositiveVertexIsVertex = false;
   for (size_t i = 0; i != H.Facets.size(); i++)
   {
      if (H.Facets[i].PointIndices.find(PositiveVertexIndex) != H.Facets[i].PointIndices.end())
      {
         PositiveVertexIsVertex = true;
         double ScaleFactor = rand();
         for (size_t j = 0; j != VectorForOrientation.size(); j++)
            VectorForOrientation[j] += ScaleFactor * H.Facets[i].Normal[j];
      };
   };
   
   if (!PositiveVertexIsVertex)
      throw runtime_error("Positive monomial is not a vertex.");
   
   for (size_t i = 0; i != S.Pts.size(); i++)
   {
      H.S.Pts[i].IP = DoubleInnerProduct(H.S.Pts[i].Pt, VectorForOrientation);
   };
   
   sort(H.S.Pts.begin(), H.S.Pts.end(), SupportPtSort);


   H.PointToIndexMap.clear();
   H.IndexToPointMap.clear();
   for (size_t i = 0; i != H.S.Pts.size(); i++)
   {
      SupportPoint Point = H.S.Pts[i];
      H.PointToIndexMap[Point]=i;
      H.IndexToPointMap[i]=Point;
      if (Point.Sign == PLUS && i != 0)
         throw runtime_error("Internal error: the positive vertex should be the first afte sorting.");
   };

   FindEdges(H, 0);
   
   FindConesHelper(H, false, false, H.S.Pts[0]);

   if (Verbose)
   {
      cout << "Convex hull------------------------" << endl;
      PrintSupport(H.S);
      cout << "Affine dimension: " << H.AffineDimension << endl;
      cout << "Space dimension: " << H.SpaceDimension << endl;
      cout << "Number of cones: " << H.Cones.size() << endl;
      cout << "Number of edges: " << H.Edges.size() << endl;
      cout << "Number of facets: " << H.Facets.size() << endl << endl;
   };
   return H.Cones;
};
