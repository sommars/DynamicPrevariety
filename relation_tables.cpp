#include "relation_tables.h"
int RTIntersectionCount;
int ZeroCount;
int NonZeroCount;

//------------------------------------------------------------------------------
void DoMarkRelationTables(vector<vector<Cone> > &HullCones, vector<vector<vector<BitsetWithCount> > > &RTs, mutex &Mtx, int &DimForIntersectTrue, int ID)
{
   for(int i = 0; i != HullCones.size(); i++)
   {
      for(size_t j = i+1; j != HullCones.size(); j++)
      {
         for(size_t k = 0; k != HullCones[i].size(); k++)
         {
            for(size_t l = 0; l != HullCones[j].size(); l++)
            {
               if (RTs[i][k][j].Indices[l] == 0)
               {
                  Mtx.lock();
                  if (RTs[i][k][j].Indices[l] != 0)
                  {
                     Mtx.unlock();
                     continue;
                  };
                  // Mark off BOTH of them
                  RTs[i][k][j].Indices[l] = 1;
                  RTs[j][l][i].Indices[k] = 1;
                  RTIntersectionCount++;
                  Mtx.unlock();
                  if (DimForIntersectTrue <= 0)
                  { 
                     if (!HullCones[i][k].HOPolyhedron.is_disjoint_from(HullCones[j][l].HOPolyhedron))
                     {
                        Mtx.lock();
                        HullCones[i][k].RelationTables[j].Indices[l] = 1;
                        HullCones[j][l].RelationTables[i].Indices[k] = 1;
                        HullCones[i][k].RelationTables[j].Count++;
                        HullCones[j][l].RelationTables[i].Count++;
                        NonZeroCount++;
                        Mtx.unlock();
                     } else ZeroCount++;
                  }
                  else
                  {
                     Cone TestCone = HullCones[i][k];
                     TestCone.HOPolyhedron.add_constraints(
                        HullCones[j][l].HOPolyhedron.constraints());

                     if (TestCone.HOPolyhedron.affine_dimension() > DimForIntersectTrue)
                     {
                        Mtx.lock();
                        HullCones[i][k].RelationTables[j].Indices[l] = 1;
                        HullCones[j][l].RelationTables[i].Indices[k] = 1;
                        HullCones[i][k].RelationTables[j].Count++;
                        HullCones[j][l].RelationTables[i].Count++;
                        NonZeroCount++;
                        Mtx.unlock();
                     } else ZeroCount++;
                  }
               };
            };
         };
      };
   };
   return;
};

//------------------------------------------------------------------------------
int MarkRelationTables(vector<vector<Cone> > &HullCones, vector<vector<vector<BitsetWithCount> > > &RTs1, int ProcessCount, int DimForIntersectTrue, bool Verbose)
{
   {
      Thread_Pool<function<void()>> thread_pool(ProcessCount);
      mutex Mtx;
      for (size_t i = 0; i != ProcessCount; i++)
      {
         thread_pool.submit(make_threadable(bind(
            DoMarkRelationTables,
            ref(HullCones),
            ref(RTs1),
            ref(Mtx),
            ref(DimForIntersectTrue),
            i)));
      }
      thread_pool.finalize();
   }
   
   if (Verbose)
   {
      cout << "Zero count " << ZeroCount << endl;
      cout << "NonZero count " << NonZeroCount << endl;
   };
   
   return RTIntersectionCount;
};
