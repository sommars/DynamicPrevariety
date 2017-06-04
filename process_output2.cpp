#include "process_output2.h"

int MaxVectorSize;
int EndOfVector = -1;
int NullVectorValue = -2;

//------------------------------------------------------------------------------
struct NormalizedPrevariety
{
	vector<vector<set<int> > > NewStructure;
};

//------------------------------------------------------------------------------
void InitializeVectorFromSet(vector<int> &V, set<int> &S)
{
   set<int>::iterator Itr = S.begin();
   int i = 0;
   while (Itr != S.end())
   {
      V[i] = *Itr;
      i++;
      Itr++;
   };
   V[i] = EndOfVector;
   i++;
   while (i < V.size())
   {
      V[i] = NullVectorValue;
      i++;
   };
};

//------------------------------------------------------------------------------
void IntersectVectorWithSet(vector<int> &V, set<int> &S)
{
   set<int>::iterator Itr = S.begin();
   int i = 0;
   while ((Itr != S.end()) && (V[i] != EndOfVector))
   {
      if (V[i] == NullVectorValue)
         i++;
      else if (*Itr < V[i])
         Itr++;
      else if (V[i]<*Itr)
      {
         V[i] = NullVectorValue;
         i++;
      }
      else 
      {
         Itr++;
         i++;
      };
   };
   
   while (i < V.size())
   {
      if (V[i] == EndOfVector)
         break;
      V[i] = NullVectorValue;
      i++;
   };
};

//------------------------------------------------------------------------------
bool SizeOfVectorIsZero(vector<int> &V)
{
   for(size_t i = 0; i != V.size(); i++)
   {
      if (V[i] == EndOfVector)
         return true;
      if (V[i] != NullVectorValue)
         return false;
   };
   return true;
};

//------------------------------------------------------------------------------
void ParallelMarking(NormalizedPrevariety &NP, TropicalPrevariety &TP, mutex &ConeMtx)
{
   vector<int> MyVec(MaxVectorSize+1);
   
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
            
            InitializeVectorFromSet(MyVec,NP.NewStructure[k][*Itr]);
            Itr++;
            if (Itr == TP.ConeTree[i][j].RayIndices.end())
            {
               if (!SizeOfVectorIsZero(MyVec))
                  WeKnowConeIsNotMaximal = true;
            
            } else while (Itr != TP.ConeTree[i][j].RayIndices.end())
            {
               {
                  IntersectVectorWithSet(MyVec,NP.NewStructure[k][*Itr]);
                  if (SizeOfVectorIsZero(MyVec))
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
	          if (NP.NewStructure[i][*Itr].size() > MaxVectorSize)
	             MaxVectorSize = NP.NewStructure[i][*Itr].size();
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
   
   for (size_t i = 0; i != TP.ConeTree.size(); i++)
   {
      for (size_t j = 0; j != TP.ConeTree[i].size(); j++)
      {
	       PrintPoint(TP.ConeTree[i][j].RayIndices);
      };
      cout << endl;
   };
   
   MaxVectorSize = 0;
   NormalizedPrevariety NP = MakeNormalizedPrevariety(TP);
   cout << MaxVectorSize << endl;
   
   cout << "Made structure" << endl;
   for (size_t i = 0; i != NP.NewStructure.size(); i++)
   {
      for (size_t j = 0; j != NP.NewStructure[i].size(); j++)
      {
         PrintPoint(NP.NewStructure[i][j]);
      };
      cout << endl;
   };
   
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

