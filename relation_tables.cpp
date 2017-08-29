#include "relation_tables.h"
int RTIntersectionCount;

//------------------------------------------------------------------------------
void DoMarkRelationTables(vector<Hull> &Hulls, vector<vector<vector<BitsetWithCount> > > &RTs, mutex &Mtx, int ID)
{
   for(int i = 0; i != Hulls.size(); i++)
   {
      for(size_t j = i+1; j != Hulls.size(); j++)
      {
         for(size_t k = 0; k != Hulls[i].Cones.size(); k++)
         {
            for(size_t l = 0; l != Hulls[j].Cones.size(); l++)
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
                  if (!Hulls[i].Cones[k].HOPolyhedron.is_disjoint_from(
                      Hulls[j].Cones[l].HOPolyhedron))
                  {
                     Mtx.lock();
                     Hulls[i].Cones[k].RelationTables[j].Indices[l] = 1;
                     Hulls[j].Cones[l].RelationTables[i].Indices[k] = 1;
                     Hulls[i].Cones[k].RelationTables[j].Count++;
                     Hulls[j].Cones[l].RelationTables[i].Count++;
                     Mtx.unlock();
                  };
               };
            };
         };
      };
   };
   return;
};

//------------------------------------------------------------------------------
int MarkRelationTables(vector<Hull> &Hulls, vector<vector<vector<BitsetWithCount> > > &RTs1, int ProcessCount)
{
   {
      Thread_Pool<function<void()>> thread_pool(ProcessCount);
      mutex Mtx;
      for (size_t i = 0; i != ProcessCount; i++)
      {
         thread_pool.submit(make_threadable(bind(
            DoMarkRelationTables,
            ref(Hulls),
            ref(RTs1),
            ref(Mtx),
            i)));
      }
      thread_pool.finalize();
   }
   return RTIntersectionCount;
};
