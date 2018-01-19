#include "ppl.hh"
#include <boost/dynamic_bitset.hpp>
#include "Thread_Pool_defs.hh"
#include <fstream>
#include <iostream>

using namespace std;
namespace Parma_Polyhedra_Library {using IO_Operators::operator<<;}

enum SupportPointSign {PLUS, MINUS, NONE};

//------------------------------------------------------------------------------
struct SupportPoint
{
   vector<int> Pt;
   double IP;
   SupportPointSign Sign;
   bool operator<( const SupportPoint& other ) const
   {
      for (size_t i = 0; i != Pt.size(); i++)
      {
         if (Pt[i] != other.Pt[i])
            return Pt[i] < other.Pt[i];
      };
      return false;
   };
};

//------------------------------------------------------------------------------
struct Support
{
   vector<SupportPoint> Pts;
};

//------------------------------------------------------------------------------
struct BitsetWithCount
{
   // Used for relation tables and for keeping track which
   // polytopes have been visited by a polytope.
   // As expected, 1 represents true, 0 represents false.
   boost::dynamic_bitset<> Indices;
   int Count;
};

//------------------------------------------------------------------------------
struct Cone
{
   // This is the base object used in computation. They hold relation tables
   // and PPL C_Polyhedron.
   vector<BitsetWithCount> RelationTables;
   BitsetWithCount PolytopesVisited;
   NNC_Polyhedron HOPolyhedron;
   //LPRowSetReal Rows;
};

//------------------------------------------------------------------------------
struct Edge
{
   // Represents an edge, used primarily for convex hull computations.
   set<int> PointIndices;
   set<int> NeighborIndices;
};

//------------------------------------------------------------------------------
struct Facet
{
   // Represents a facet, used primarily for convex hull computations.
   set<int> PointIndices;
   vector<int> Normal;
};

//------------------------------------------------------------------------------
struct Hull
{
   // Convex hull object, used to create the initial set of cones for each
   // polytope.
   Support S;
   map<SupportPoint,int> PointToIndexMap;
   map<int,SupportPoint> IndexToPointMap;
   vector<Edge> Edges;
   vector<Cone> Cones;
   vector<Facet> Facets;
   C_Polyhedron CPolyhedron;
   int AffineDimension;
   int SpaceDimension;
};

//------------------------------------------------------------------------------
struct ConeWithIndicator
{
   // Helper object used to aide in parsing output of algorithm
   set<int> RayIndices;
   int Status;
   C_Polyhedron HOPolyhedron;
   // 0 means that this cone is NOT maximal.
   // 1 means that this cone is maximal.
   // 2 means that this cone's status is unknown, and it is not being followed.
   // 3 means that this cone is currently being followed.
};

//------------------------------------------------------------------------------
struct TropicalPrevariety
{
   // TODO: This could probably be improved with sorting beforehand.
   // Output object. Only outputs all the maximal cones.
   map<vector<int>, int> RayToIndexMap;
   map<int, vector<int>> IndexToRayMap;
   vector<vector<ConeWithIndicator > > ConeTree;
};

//------------------------------------------------------------------------------
struct ThreadQueue
{
   // Thread Queue for parallel computation. Each thread has its own independent
   // queue, which facilitates work stealing.
   mutable mutex M;
   vector<list<Cone> > SharedCones;
   bool ThreadShouldDie;
   ThreadQueue(vector<list<Cone> > ConeVector): SharedCones(ConeVector) {};
   ThreadQueue(const ThreadQueue& TQ) {
      lock_guard<mutex> lock(TQ.M);
      ThreadShouldDie = false;
      SharedCones = TQ.SharedCones;
   };
};

//------------------------------------------------------------------------------
struct OutputProcessor
{
   stringstream s;
   mutable mutex M;
   int level;
   OutputProcessor(int x): level(x) {};
   OutputProcessor(const OutputProcessor& OP) {
      lock_guard<mutex> lock(OP.M);
   }; 
};
