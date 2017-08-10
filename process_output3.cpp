#include "process_output3.h"

//------------------------------------------------------------------------------
C_Polyhedron GetConeFromTree(TropicalPrevariety &TP, int i, int j)
{
   vector<vector<int> > Rays;
   for (set<int>::iterator Itr = TP.ConeTree[i][j].RayIndices.begin(); 
      Itr != TP.ConeTree[i][j].RayIndices.end(); Itr++)
      Rays.push_back(TP.IndexToRayMap[*Itr]);
   return RaysToCone(Rays);
}

//------------------------------------------------------------------------------
void ParallelMarkingVersion3(TropicalPrevariety TP, mutex &ConeMtx)
{
   cout << TP.ConeTree[0][0].Status << endl;
   cout << TP.ConeTree[0][0].HOPolyhedron.affine_dimension() << endl;
   cout << TP.ConeTree[0][0].HOPolyhedron.minimized_generators() << endl;
   cout << "AAAAAAAAAAAA" << endl;

   for(size_t i = TP.ConeTree.size() - 2; i != -1; i--)
   {
      for(size_t j = 0; j != TP.ConeTree[i].size(); j++)
      {
         ConeMtx.lock();
         if (TP.ConeTree[i][j].Status != 2)
         {
            ConeMtx.unlock();
            continue;
         };
         ConeMtx.unlock();
         TP.ConeTree[i][j].Status = 3;
         
         bool WeKnowConeIsNotMaximal = false;
         for(size_t k = TP.ConeTree.size() - 1; k != i; k--)
         {
            for(size_t l = 0; l != TP.ConeTree[k].size(); l++)
            {
               if (TP.ConeTree[k][l].Status == 0)
                  continue;
               cout << "------------" << endl;
               cout << TP.ConeTree[i][j].HOPolyhedron.minimized_generators() << endl << endl;
               cout << TP.ConeTree[k][l].HOPolyhedron.minimized_generators() << endl<< endl;
               if (TP.ConeTree[k][l].HOPolyhedron.contains(TP.ConeTree[i][j].HOPolyhedron))
               {
                  WeKnowConeIsNotMaximal = true;
                  TP.ConeTree[i][j].Status = 0;
               }
               if (WeKnowConeIsNotMaximal)
                  break;
            }
            if (WeKnowConeIsNotMaximal)
               break;
         }
         
         if (!WeKnowConeIsNotMaximal)
            TP.ConeTree[i][j].Status = 1;
      };
   };
   return;
}


//------------------------------------------------------------------------------
void MarkMaximalCones3(TropicalPrevariety &TP, int ProcessCount)
{
   if (TP.ConeTree.size() < 2)
      return;

   // Need to reset the cone tree. Anything that we think is maximal only *may* be maximal.
   for (size_t i = 0; i != TP.ConeTree.size() - 1; i++)
   {
      for (size_t j = 0; j != TP.ConeTree[i].size(); j++)
      {
         if (TP.ConeTree[i][j].Status == 1) {
            TP.ConeTree[i][j].Status = 2;
         };
      };
   };
   cout << "A" << endl;
   TP.ConeTree[0][0].Status = 100;
   cout << TP.ConeTree[0][0].HOPolyhedron.minimized_generators() << endl;

   {
      Thread_Pool<function<void()>> thread_pool(ProcessCount);
      mutex ConeMtx;
      for (size_t i = 0; i != ProcessCount; i++)
      {
         thread_pool.submit(bind(
            ParallelMarkingVersion3,
            ref(TP),
            ref(ConeMtx)));
      }
      
      // Wait for all workers to complete.
      thread_pool.finalize();
   }
   return;
}

