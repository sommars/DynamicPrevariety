#include "relation_tables.h"


int ConeIntersectionCount;
bool OnlyFindLowerHull = false;
bool OnlyFindUpperHull = false;
bool ExitOnFindDimension = false;
bool ExitedComputationEarly = false;
bool OnlyFindHighestDimensionalCones = false;
int DimensionForExit = -1;

//------------------------------------------------------------------------------
list<Cone> DoCommonRefinement(
   int HullIndex,
   Cone &NewCone,
   vector<vector<Cone> > &HullCones)
{
   // Perform common refinement for the specified cone and set of cones.
   vector<Cone> *HIndex;
   HIndex = &HullCones[HullIndex];
   BitsetWithCount *RT = &NewCone.RelationTables[HullIndex];
   list<Cone> Result;
   Cone *ConeToTest;
   for (boost::dynamic_bitset<>::size_type i = 0; i != RT->Indices.size(); i++)
   {
      if (!RT->Indices[i])
         continue;
      ConeToTest = &(*HIndex)[i];

      bool SkipIntersection = false;
      vector<BitsetWithCount> RelationTables(HullCones.size());

      for (size_t j = 0; j != NewCone.RelationTables.size(); j++)
      {
         if (!NewCone.PolytopesVisited.Indices[j])
         {
            RelationTables[j] = IntersectRTs(
               ConeToTest->RelationTables[j], NewCone.RelationTables[j]);
            if (RelationTables[j].Count == 0)
            {
               SkipIntersection = true;
               break;
            };
         };
      };
      
      if (SkipIntersection)
         continue;
         
      ConeIntersectionCount++;
      Cone TestCone = NewCone;
      TestCone.HOPolyhedron.add_constraints(
         ConeToTest->HOPolyhedron.constraints());
      if (TestCone.HOPolyhedron.is_empty())
         continue;
         
      TestCone.RelationTables = RelationTables;

      Result.push_back(TestCone);
   };
   
   return Result;
}

//------------------------------------------------------------------------------
inline list<Cone> DynamicEnumerate(
   Cone &C, vector<vector<Cone> > &HullCones)
{
   // Figure out which polytope we want to visit next
   bool HaveCandidatePolytope = false;
   int SmallestInt;
   int SmallestIndex;
   for (size_t i = 0; i != C.RelationTables.size(); i++)
   {
      if ((!C.PolytopesVisited.Indices[i])
      && ((!HaveCandidatePolytope) || (C.RelationTables[i].Count<SmallestInt)))
      {
         SmallestInt = C.RelationTables[i].Count;
         SmallestIndex = i;
         HaveCandidatePolytope = true;
      };
   };
   if (!HaveCandidatePolytope)
      throw runtime_error("Internal error: DynamicEnumerate did not "
                          "find a next polytope to visit");
                              
   C.PolytopesVisited.Indices[SmallestIndex] = 1;
   C.PolytopesVisited.Count++;

   return DoCommonRefinement(SmallestIndex, C, HullCones);
};

//------------------------------------------------------------------------------
void ThreadEnum(
   vector<vector<Cone> > HullCones,
   int ProcessID,
   int ProcessCount,
   vector<ThreadQueue> &ThreadQueues,
   TropicalPrevariety &Output,
   mutex &OutputMtx,
   int &FinishedProcessCount)
{
   // Manages work stealing and dishing out jobs from thread queues.
   vector<vector<int> > Pretropisms;
   Cone C;
   bool HasCone = false;
   ThreadQueue *TQ;
   while (true)
   {
      int j = ProcessID;
      while (not HasCone)
      {
         int StartIndex;
         int EndIndex;
         int Incrementer;
         TQ = &ThreadQueues[j % ProcessCount];
         if (j == ProcessID)
         {
            StartIndex = TQ->SharedCones.size() - 1; // IS THIS WRONG?!?!?!
            EndIndex = -1;
            Incrementer = -1;
         } else {
            StartIndex = 0;
            EndIndex = TQ->SharedCones.size();
            Incrementer = 1;
         };
         TQ->M.lock();
         // If work stealing happens, we want to steal the less traveled cones,
         // not the more traveled cones.
         for (size_t i = StartIndex; i !=EndIndex; )
         {
            if (TQ->SharedCones[i].size() > 0)
            {
               C = TQ->SharedCones[i].back();
               TQ->SharedCones[i].pop_back();
               HasCone = true;
               // This case happens when the process tried to steal from every
               // process possible but did not succeed, making it bored.
               // It added itself to BoredProcesses, but then continued looking
               // and found a cone it could have. This made it no longer a
               // bored process.
               if (j > (ProcessID + ProcessCount))
               {
                  FinishedProcessCount -= 1;
               };
               break;
            };
            i+=Incrementer;
         };
         TQ->M.unlock();
         
         /*
         // This is a sketch of a progress counter
         if (ProcessID == 0)
         {
           int RemainingCones = 0;
           for(size_t ij = 0; ij != ThreadQueues.size(); ij++)
             for(size_t ijk = 0; ijk != ThreadQueues[ij].SharedCones.size(); ijk++)
               RemainingCones += ThreadQueues[ij].SharedCones[ijk].size();
           cout<< RemainingCones << " ";
           //cout << "RemainingCones: " << RemainingCones << endl;
         };
         */
         
         if (HasCone)
            break;
         // This case means that we spun through each of the other ThreadQueues
         // and they were all empty. Now we need to make the process bored.
         if (j == ProcessCount + ProcessID)
         {
            FinishedProcessCount += 1;
         };
         if ((j % ProcessCount) == ProcessID)
         {
            if (FinishedProcessCount == ProcessCount)
               return;
         };
         j++;
      };
      if (TQ->ThreadShouldDie)
         return;

      list<Cone> ResultCones = DynamicEnumerate(C, HullCones);

      // If there are remaining new cones, give them to the job queue.
      if (ResultCones.size() > 0)
      {
         int Index = ResultCones.front().PolytopesVisited.Count;
         
         // The cones have visited all of the polytopes.
         if (Index == HullCones.size())
         {
            list<Cone>::iterator i;
            for (i = ResultCones.begin(); i != ResultCones.end(); i++)
            {
               int ConeDim = i->HOPolyhedron.affine_dimension();
               if (ConeDim == 0)
                  continue;
                  
               if (ExitOnFindDimension && (ConeDim <= DimensionForExit))
                  continue;
                  
               if (TQ->ThreadShouldDie)
                  return;
               ConeWithIndicator CWI;
               CWI.HOPolyhedron = i->HOPolyhedron;
               CWI.Status = 2;
               
              //cout << i->HOPolyhedron.affine_dimension() << endl;
              //cout << i->HOPolyhedron.generators() << endl;
              //cout << i->HOPolyhedron.constraints() << endl;
               for (Generator_System::const_iterator gsi = 
                       i->HOPolyhedron.generators().begin(),
                       gs_end = i->HOPolyhedron.generators().end();
                   gsi != gs_end;
                   ++gsi)
               {
                  if (gsi->is_point() 
                  || gsi->is_closure_point() 
                  //|| gsi->coefficient(Variable(gsi->space_dimension() -1)) != 0
                  )
                     continue;
                     
                  Generator G = *gsi;
                  vector<int> Ray = GeneratorToPoint(G, false);
                  OutputMtx.lock();
                  clock_t FindStart = clock();
                  map<vector<int>, int>::iterator itr = Output.RayToIndexMap.find(Ray);
                  if (itr == Output.RayToIndexMap.end())
                  {
                     int Index = Output.RayToIndexMap.size();
                     Output.RayToIndexMap[Ray] = Index;
                     Output.IndexToRayMap[Index] = Ray;
                     CWI.RayIndices.insert(Output.RayToIndexMap.size() - 1);
                  } else
                     CWI.RayIndices.insert(itr->second);
                  OutputMtx.unlock();
                  
                  if (gsi->is_line()) {
                     // If it's a line, we have to add both rays that make it up
                     for(size_t j = 0; j != Ray.size(); j++)
                        Ray[j] = -1 * Ray[j];
                     
                     OutputMtx.lock();
                     map<vector<int>, int>::iterator itr2 = Output.RayToIndexMap.find(Ray);
                     if (itr2 == Output.RayToIndexMap.end())
                     {
                        int Index = Output.RayToIndexMap.size();
                        Output.RayToIndexMap[Ray] = Index;
                        Output.IndexToRayMap[Index] = Ray;
                        CWI.RayIndices.insert(Output.RayToIndexMap.size() - 1);
                     } else
                        CWI.RayIndices.insert(itr2->second);
                     OutputMtx.unlock();
                  }
               };
               OutputMtx.lock();
               while (Output.ConeTree.size() < (ConeDim))
               {
                  vector<ConeWithIndicator> Temp;
                  Output.ConeTree.push_back(Temp);
               };
               Output.ConeTree[ConeDim - 1].push_back(CWI);
               OutputMtx.unlock();
               
               if (ExitOnFindDimension && (ConeDim > DimensionForExit))
               {
                  // spin through all of the threads and set ThreadShouldDie on each of them.
                  for (size_t TQIndex = 0; TQIndex != ThreadQueues.size(); TQIndex++)
                     ThreadQueues[TQIndex].ThreadShouldDie = true;
                  ExitedComputationEarly = true;
                  return;
               };

            };
            HasCone = false;
         } else {
            // If there is at least one cone, hold onto it for the next round.
            C = ResultCones.back();
            ResultCones.pop_back();
            if (ResultCones.size() > 0)
            {
               TQ = &ThreadQueues[ProcessID];
               TQ->M.lock();
               list<Cone>::iterator i;
               for (i = ResultCones.begin(); i != ResultCones.end(); i++)
                  TQ->SharedCones[Index - 1].push_back(*i);
               TQ->M.unlock();
            };
         };
      } else
         HasCone = false;
   };
};

//------------------------------------------------------------------------------
bool StrIsInt(char* arg)
{
   string s = string(arg);
   string::const_iterator it = s.begin();
   while (it != s.end() && std::isdigit(*it))
      ++it;
   return !s.empty() && it == s.end();
};

//------------------------------------------------------------------------------
void PrintHelp()
{
   cout << "This program computes the tropical prevariety of a set of set of points." << endl << endl
   << "Valid options include:" << endl
   << "-t for setting the number of threads" << endl
   << "-l for finding only the open lower hull" << endl
   << "-u for finding only the open upper hull" << endl
   << "-d for returning the first cone of dimension > d of the tropical prevariety that is found" << endl
   << "-h will return only the highest dimensional cones, which may lead to a speedup" << endl
   << "-v for verbose" << endl
   << "An example call to the program is:" << endl
   << "./prevariety examples/cyclic/cyclic8 -t 4 -l -d 0" << endl
   << "which will use four threads to compute the lower tropical prevariety, and will exit as soon as it finds a cone of dimension > 0." << endl;

};

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
   struct timeval AlgStartTime, AlgEndTime;
   gettimeofday(&AlgStartTime, NULL);
   bool Verbose = false;

   double RandomSeed = time(NULL);
   
   int ProcessCount = 1;
   vector<vector<vector<int> > > PolynomialSystemSupport;
   
   if ((argc == 2) && (string(argv[1]) == "-help"))
   {
      PrintHelp();
      return 0;
   };
   
   if (argc >= 2)
   {
      string FileName;
      FileName = string(argv[1]);
      ifstream f(FileName.c_str());
      if (!f.good())
         throw invalid_argument("Please input a valid filename.");
      PolynomialSystemSupport = ParseSupportFile(FileName);

      for (size_t i = 2; i < argc; )
      {
         string Option = string(argv[i]);
         if (Option == "-d")
         {
            ExitOnFindDimension = true;
            if (((i + 1) < argc) && StrIsInt(argv[i+1]))
            {
               DimensionForExit = atoi(argv[i+1]);
            } else
               throw invalid_argument("-d must be followed by a dimension.");

            i++;
            i++;
         }
         else if (Option == "-t")
         {
            if (((i + 1) < argc) && StrIsInt(argv[i+1]))
            {
               ProcessCount = atoi(argv[i+1]);
            } else
               throw invalid_argument("-t must be followed by a number of threads.");
            i++;
            i++;
         }
         else if (Option == "-s")
         {
            if (((i + 1) < argc) && StrIsInt(argv[i+1]))
            {
               RandomSeed = atoi(argv[i+1]);
            } else
               throw invalid_argument("-s must be followed by an integer random seed.");
            i++;
            i++;
         }
         else if (Option == "-l")
         {
            OnlyFindLowerHull = true;
            i++;
         }
         else if (Option == "-u")
         {
            OnlyFindUpperHull = true;
            i++;
         }
         else if (Option == "-v")
         {
            Verbose = true;
            i++;
         }
         else if (Option == "-h")
         {
            OnlyFindHighestDimensionalCones = true;
            i++;
         }
         else if (Option == "-help")
         {
            PrintHelp();
            return 0;
         }
         else 
            throw invalid_argument("Invalid input. Please run -help for instructions");
   
      };
   } else
     throw invalid_argument("Invalid input. Please run -help for instructions");

   if (OnlyFindLowerHull && OnlyFindUpperHull)
     throw invalid_argument("Cannot find only upper hull and only lower hull.");
   
   if (Verbose)
   {
      cout << "----Options----" << endl
      << "ProcessCount: " << ProcessCount << endl
      << "Find lower hull: " << OnlyFindLowerHull << endl
      << "Find upper hull: " << OnlyFindUpperHull << endl
      << "Exit on find dimension: " << ExitOnFindDimension << endl
      << "Only find highest dimensional cones: " << OnlyFindHighestDimensionalCones << endl
      << "Seed: " << RandomSeed << endl;
   };

   srand(RandomSeed);
      
   bool DoMixedVol = false;
   if (DoMixedVol)
   {
      for (size_t i = 0; i != PolynomialSystemSupport.size(); i++)
         for (size_t j = 0; j != PolynomialSystemSupport[i].size(); j++)
            PolynomialSystemSupport[i][j].push_back((rand() % 100000) + 1);
   };
   if (ProcessCount > thread::hardware_concurrency())
   {
      string ThreadErrorMsg = "Internal error: hardware_concurrency = "
                              + to_string(thread::hardware_concurrency())
                              + " but ProcessCount = " 
                              + to_string(ProcessCount);
      throw invalid_argument(ThreadErrorMsg);
   };
   vector<vector<Cone> > HullCones;
   vector<double> VectorForOrientation;
   for (size_t i = 0; i != PolynomialSystemSupport[0][0].size(); i++)
      VectorForOrientation.push_back(rand());
   for (size_t i = 0; i != PolynomialSystemSupport.size(); i++)
      HullCones.push_back(
         NewHull(PolynomialSystemSupport[i], VectorForOrientation, Verbose, OnlyFindLowerHull, OnlyFindUpperHull));

   // Initialize each cone's PolytopesVisited object
   for(int i = 0; i != HullCones.size(); i++)
   {
      for (size_t j = 0; j != HullCones[i].size(); j++)
      {
         HullCones[i][j].PolytopesVisited.Indices.resize(HullCones.size());
         HullCones[i][j].PolytopesVisited.Indices[i] = 1;
         HullCones[i][j].PolytopesVisited.Count = 1;
      };
   };

   // Correctly size relation tables
   vector<vector<vector<BitsetWithCount> > > RTs;
   for(size_t i = 0; i != HullCones.size(); i++)
   {
      vector<vector<BitsetWithCount> > RTs1;
      for(size_t j = 0; j != HullCones[i].size(); j++)
      {
         vector<BitsetWithCount> RTs2;
         for(size_t k = 0; k != HullCones.size(); k++)
         {
            BitsetWithCount RT;
            RT.Indices.resize(HullCones[k].size());
            RT.Count = 0;
            HullCones[i][j].RelationTables.push_back(RT);
            BitsetWithCount RT2;
            RT2.Indices.resize(HullCones[k].size());
            RT2.Count = 0;
            RTs2.push_back(RT2);
         };
         RTs1.push_back(RTs2);
      };
      RTs.push_back(RTs1);
   };
   
   
   struct timeval PreIntStartTime, PreIntEndTime;
   gettimeofday(&PreIntStartTime, NULL);
   int TotalInt = MarkRelationTables(HullCones, RTs, ProcessCount, DimensionForExit, Verbose);
   gettimeofday(&PreIntEndTime, NULL);
   double PreintersectTime = ((PreIntEndTime.tv_sec  - PreIntStartTime.tv_sec) * 1000000u + 
         PreIntEndTime.tv_usec - PreIntStartTime.tv_usec) / 1.e6;

   // Pick which polytope to start with. Initialize to the first polytope
   int SmallestInt = 0;
   int SmallestIndex = 0;
   for (size_t i = 0; i != HullCones[0].size(); i++)
   {
      for (size_t j = 0; j != HullCones[0][i].RelationTables.size(); j++)
         SmallestInt += HullCones[0][i].RelationTables[j].Count;
   };
   // Then try all possible polytopes
   for (size_t i = 1; i != HullCones.size(); i++)
   {
      int TestValue = 0;
      for (size_t j = 0; j != HullCones[i].size(); j++)
      {
         for (size_t k = 0; k != HullCones[i][j].RelationTables.size(); k++)
            TestValue += HullCones[i][j].RelationTables[k].Count;
      };
      if (TestValue < SmallestInt)
      {
         SmallestInt = TestValue;
         SmallestIndex = i;
      };
   };
   
   vector<ThreadQueue> ThreadQueues;
   for (size_t i = 0; i != ProcessCount; i++)
   {
      vector<list<Cone> > SharedCones;
      for (size_t j = 0; j != HullCones.size() - 1; j++)
      {
         list<Cone> Temp;
         SharedCones.push_back(Temp);
      };
      ThreadQueue TQ(SharedCones);
      ThreadQueues.push_back(TQ);
   };
   for (size_t i = 0; i != HullCones[SmallestIndex].size(); i++)
      ThreadQueues[i % ProcessCount].SharedCones[0].push_back(
         HullCones[SmallestIndex][i]);
   
   mutex BPMtx;
   mutex OutputMtx;
   TropicalPrevariety Output;
   clock_t AlgorithmStartTime = clock();
   {
      Thread_Pool<function<void()>> thread_pool(ProcessCount);
      int FinishedProcessCount = 0;
      for (size_t i = 0; i != ProcessCount; i++)
      {
         thread_pool.submit(make_threadable(bind(
            ThreadEnum,
            HullCones,
            i,
            ProcessCount,
            ref(ThreadQueues),
            ref(Output),
            ref(OutputMtx),
            ref(FinishedProcessCount))));
      }
      
      // Wait for all workers to complete.
      thread_pool.finalize();
   }
   
   clock_t MarkingTimeStart = clock();

   if (OnlyFindHighestDimensionalCones)
   {
      // This is a poor way to do this. Ideally, this process should be integrated into the algorithm.
      if (Output.ConeTree.size() > 1)
      {
         int HighestDimension = Output.ConeTree.size();
         Output.ConeTree.erase(Output.ConeTree.begin(), Output.ConeTree.begin() + HighestDimension - 1);
         vector<ConeWithIndicator> EmptyVec;
         while (HighestDimension > Output.ConeTree.size())
         {
            Output.ConeTree.insert(Output.ConeTree.begin(), EmptyVec);
         }
      };
   };
   
   MarkMaximalCones2(Output, ProcessCount);
   MarkMaximalCones3(Output, ProcessCount);
   double MarkingTime = double(clock() - MarkingTimeStart) / CLOCKS_PER_SEC;
   
   stringstream s;
   clock_t PrintingTimeStart = clock();
   StreamRayToIndexMap(Output, s);
   PrintMaximalCones(Output, s);
   
   gettimeofday(&AlgEndTime, NULL);
   double TotalAlgTime = ((AlgEndTime.tv_sec  - AlgStartTime.tv_sec) * 1000000u + 
         AlgEndTime.tv_usec - AlgStartTime.tv_usec) / 1.e6;
   s << "------ Run data ------" << endl
   << "Intersections for building RT: " << TotalInt << endl
   << "Alg intersections: " << ConeIntersectionCount << endl
   << "Total intersections: " << TotalInt + ConeIntersectionCount << endl
   << "Preintersection time: " << PreintersectTime << endl
   << "Marking time: " << MarkingTime << endl
   << "Pretropisms: " << Output.RayToIndexMap.size() << endl
   << "Total Alg time: " << TotalAlgTime << endl
   << "Exited computation early: " << boolalpha << ExitedComputationEarly << endl
   << fixed << "Random Seed: " << RandomSeed << endl;
   
   ofstream OutFile ("output.txt");
   OutFile << s.str();
   OutFile.close();
   double PrintingTime = double(clock() - PrintingTimeStart) / CLOCKS_PER_SEC;
   int PositiveCount = 0; int ZeroCount = 0; int NegativeCount = 0;
   if (Verbose)
   {
		  for(map<vector<int>, int>::iterator itr = Output.RayToIndexMap.begin();
		  itr != Output.RayToIndexMap.end();
		  ++itr)
		  {
		     int testval = itr->first[itr->first.size() - 1];
		     if (testval > 0)
		     {
		        PositiveCount++;
		     };
		     if (testval == 0)
		     {
		        ZeroCount++;
		     };
		     if (testval < 0)
		     {
		        NegativeCount++;
		     };    
      }
		  cout << "PositiveCount:" << PositiveCount << endl;
		  cout << "ZeroCount: " << ZeroCount << endl;
		  cout << "NegativeCount: " << NegativeCount << endl;
   }
}
