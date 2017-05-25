#include "process_output2.h"

//------------------------------------------------------------------------------
struct NormalizedPrevariety
{
	vector<vector<set<int> > > NewStructure;
};

//------------------------------------------------------------------------------
void ParallelMarking(NormalizedPrevariety &NP, TropicalPrevariety &TP, mutex &ConeMtx)
{
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

         //Iterate over ray indices of TP.ConeTree[i][j]
         //For the first ray index, grab the set and copy it. Call it S.
         //For subsequent ray indices, grab the set, intersect with S, set equal to S.
         //If S is empty, then we continue. If S is not empty, then WeKnowConeIsNotMaximal.         
         bool WeKnowConeIsNotMaximal = false;
         //cout << "Point to test" << endl;
         //PrintPoint(TP.ConeTree[i][j].RayIndices);
         for(size_t k = TP.ConeTree.size() - 1; k != i; k--)
         {
            set<int>::iterator Itr = TP.ConeTree[i][j].RayIndices.begin();
            
            set<int> SetToTest = NP.NewStructure[k][*Itr];
            Itr++;
            if (Itr == TP.ConeTree[i][j].RayIndices.end())
            {
               //PrintPoint(SetToTest);
               if (SetToTest.size() != 0)
                  WeKnowConeIsNotMaximal = true;
            
            } else while (Itr != TP.ConeTree[i][j].RayIndices.end())
            {
               {
                  SetToTest = IntersectSets(SetToTest,NP.NewStructure[k][*Itr]);
                  if (SetToTest.size() == 0)
                     break;
                  Itr++;
                  if (Itr == TP.ConeTree[i][j].RayIndices.end())
                     WeKnowConeIsNotMaximal = true;
               };
            };
            if (WeKnowConeIsNotMaximal)
               break;
         }
         
         if (WeKnowConeIsNotMaximal)
            TP.ConeTree[i][j].Status = 0;
         else
            TP.ConeTree[i][j].Status = 1;
      };
   };
   return;
}

//------------------------------------------------------------------------------
NormalizedPrevariety MakeNormalizedPrevariety(TropicalPrevariety &TP)
{
   // Set up the structure so that all containers are appropriately sized
   NormalizedPrevariety NP;
   for (size_t i = 0; i != TP.ConeTree.size(); i++)
   {
      vector<set<int> > Temp(TP.RayToIndexMap.size());
      NP.NewStructure.push_back(Temp);
   };
   for (size_t i = 0; i != TP.ConeTree.size(); i++)
   {
      for (size_t j = 0; j != TP.ConeTree[i].size(); j++)
      {
	       for (set<int>::iterator Itr = TP.ConeTree[i][j].RayIndices.begin(); 
	       Itr != TP.ConeTree[i][j].RayIndices.end(); Itr++)
	       {
	          NP.NewStructure[i][*Itr].insert(j);
	       };
      };
   };
   
   return NP;
};

//------------------------------------------------------------------------------
void MarkMaximalCones2(TropicalPrevariety &TP, int ProcessCount)
{
   if (TP.ConeTree.size() < 2)
      return;
   /*
   for (size_t i = 0; i != TP.ConeTree.size(); i++)
   {
      for (size_t j = 0; j != TP.ConeTree[i].size(); j++)
      {
	       PrintPoint(TP.ConeTree[i][j].RayIndices);
      };
      cout << endl;
   };
   */
   
   NormalizedPrevariety NP = MakeNormalizedPrevariety(TP);
   /*
   cout << "Made structure" << endl;
   for (size_t i = 0; i != NP.NewStructure.size(); i++)
   {
      for (size_t j = 0; j != NP.NewStructure[i].size(); j++)
      {
         PrintPoint(NP.NewStructure[i][j]);
      };
      cout << endl;
   };
   */
   {
      Thread_Pool<function<void()>> thread_pool(ProcessCount);
      mutex ConeMtx;
      for (size_t i = 0; i != ProcessCount; i++)
      {
         thread_pool.submit(bind(
            ParallelMarking,
            ref(NP),
            ref(TP),
            ref(ConeMtx)));
      }
      
      // Wait for all workers to complete.
      thread_pool.finalize();
   }
   return;
}

