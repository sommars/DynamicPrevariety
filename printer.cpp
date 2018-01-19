#include "printer.h"

//------------------------------------------------------------------------------
void PrintPoint(vector<int> &Point)
{
   cout << "{";
   for(size_t i = 0; i != Point.size(); i++)
   {
      cout << Point[i];
      if (i != Point.size() - 1)
      	cout << ",";
   };
   cout << "}" << endl;
}

//------------------------------------------------------------------------------
void PrintPoints(vector<vector<int> > &Points)
{
   for (vector<vector<int> >::iterator itr=Points.begin();
        itr != Points.end();
        itr++)
      PrintPoint(*itr);
}

//------------------------------------------------------------------------------
void PrintSupport(Support &S)
{
   for (size_t i = 0; i != S.Pts.size(); i++) {
      if (S.Pts[i].Sign == PLUS)
         cout << "+ ";
      else if (S.Pts[i].Sign == MINUS)
         cout << "- ";
      PrintPoint(S.Pts[i].Pt);
   };
}

//------------------------------------------------------------------------------
void PrintPoint(set<int> &Point)
{
   set<int>::iterator it;
   cout << "{ ";
   for (it=Point.begin(); it != Point.end(); it++)
      cout << (*it) << " ";
   cout << "}" << endl;
}

//------------------------------------------------------------------------------
void PrintPoint(vector<bool> &Point)
{
   vector<bool>::iterator it;
   cout << "{ ";
   for (it=Point.begin(); it != Point.end(); it++)
      cout << (*it) << " ";
   cout << "}" << endl;
};

//------------------------------------------------------------------------------
void PrintPointForPython(vector<int> &Point)
{
   // Print a point such that it can be immediately read in by Python
   cout << "[ ";
   for (vector<int>::iterator it=Point.begin(); it != Point.end(); it++)
      cout << (*it) << ",";
   cout << "]";
}

//------------------------------------------------------------------------------
void PrintPointsForPython(vector<vector<int> > &Points)
{
   cout << "[";
   for (vector<vector<int> >::iterator itr=Points.begin(); 
        itr != Points.end(); 
        itr++)
   {
      PrintPointForPython(*itr);
      cout << ",";
   };
   cout << "]";
}

//------------------------------------------------------------------------------
int toint(float r)
{
   // Helper function for printLP
   return *((int*)&r);
};

//------------------------------------------------------------------------------
void PrintMaximalCones(TropicalPrevariety &TP, stringstream &s)
{
   if (TP.ConeTree.size() == 0)
      return;
   vector<int> MaximalConeCounts(TP.ConeTree.size());
   // Prints maximal cones from prevariety object.
   for (size_t i = 0; i != TP.ConeTree.size() - 1; i++)
   {
      if (TP.ConeTree[i].size() > 0)
         s << "------ Cones of dimension " << i + 1 << " ------"<< endl;
      for (size_t j = 0; j != TP.ConeTree[i].size(); j++)
      {
         if (TP.ConeTree[i][j].Status != 1)
            continue;
         MaximalConeCounts[i]++;
         set<int>::iterator it;
         s << "{";
         for (it=TP.ConeTree[i][j].RayIndices.begin();
              it != TP.ConeTree[i][j].RayIndices.end();
              it++)
            s << (*it) << ",";
         s << "}" << endl;
      };
   };
   int Dim = TP.ConeTree.size() - 1;
   
   if (Dim < 0)
      return;
      
   s << "------ Cones of dimension " << Dim + 1 << " ------"<< endl;
   
   for (size_t j = 0; j != TP.ConeTree[Dim].size(); j++)
   {
      set<int>::iterator it;
      s << "{";
      for (it=TP.ConeTree[Dim][j].RayIndices.begin();
           it != TP.ConeTree[Dim][j].RayIndices.end();
           it++)
         s << (*it) << ",";
      s << "}" << endl;
      MaximalConeCounts[Dim]++;
   };
   
   s << "------ F-vector  ------"<< endl;
   s << "{ ";
   for (size_t i = 0; i != MaximalConeCounts.size(); i++)
   {
      s << MaximalConeCounts[i];
      if (i != MaximalConeCounts.size() - 1)
         s << ", ";
   };
   s << " }" << endl;
}

//------------------------------------------------------------------------------
void StreamPoint(set<int> &Point, stringstream &s)
{
   set<int>::iterator it;
   s << "{ ";
   for (it = Point.begin(); it != Point.end(); it++)
      s << (*it) << " ";
   s << "}" << endl;
};

//------------------------------------------------------------------------------
void PrintRT(vector<BitsetWithCount> &RT)
{

cout << endl;
   for (size_t i = 0; i != RT.size(); i++)
   {
      cout << RT[i].Indices << endl;
   };
cout << endl;
};

//------------------------------------------------------------------------------
void StreamRayToIndexMap(TropicalPrevariety &TP, stringstream &s)
{
   // Streams ray to index map. Necessary for interpreting output.
   if (TP.RayToIndexMap.size() != 0)
      s << "------ Rays  ------" << endl;
   for(map<vector<int>, int>::iterator itr = TP.RayToIndexMap.begin();
       itr != TP.RayToIndexMap.end();
       ++itr)
   {
      s << itr->second << ": {";
      for (size_t i = 0; i != itr->first.size(); i++)
         s << itr->first[i] << ",";
      s << "}" << endl;
   }
   return;
}

//------------------------------------------------------------------------------
void StreamRayToIndexMapGfan(TropicalPrevariety &TP, stringstream &s)
{
   for(map<vector<int>, int>::iterator itr = TP.RayToIndexMap.begin();
       itr != TP.RayToIndexMap.end();
       ++itr)
   {
      for (size_t i = 0; i != itr->first.size(); i++)
      {
         s << itr->first[i];
         if (i != itr->first.size() - 1)
            s << " ";
      };
      s << "\t# " << itr->second << endl;
   }
   return;
}

//------------------------------------------------------------------------------
void GetConesAndFvectorForGfan(TropicalPrevariety &TP, stringstream &ConeStream, stringstream &MultiplicitiesStream, stringstream &FvectorStream)
{
   if (TP.ConeTree.size() == 0)
   {
      ConeStream << "{}\t# Dimension 0" << endl;
      MultiplicitiesStream << "1\t# Dimension 0" << endl;
   };
   vector<int> MaximalConeCounts(TP.ConeTree.size());
   for (size_t i = 0; i != TP.ConeTree.size(); i++)
   {
      for (size_t j = 0; j != TP.ConeTree[i].size(); j++)
      {
         if ((TP.ConeTree[i][j].Status != 1) && (i + 1 != TP.ConeTree.size()))
            continue;
         set<int>::iterator it;
         ConeStream << "{";
         for (it=TP.ConeTree[i][j].RayIndices.begin(); it != TP.ConeTree[i][j].RayIndices.end(); )
         {
            ConeStream << (*it);
            it++;
            if (it != TP.ConeTree[i][j].RayIndices.end())
               ConeStream << " ";
         };
         ConeStream << "}";
         MultiplicitiesStream << "1";
         if (MaximalConeCounts[i] == 0)
         {
            ConeStream << "\t# Dimension " << i + 1;
            MultiplicitiesStream << "\t# Dimension " << i + 1;
         }
         MultiplicitiesStream << endl;
         ConeStream << endl;
         MaximalConeCounts[i]++;
      };
   };      

   FvectorStream << "1";
   for (size_t i = 0; i != MaximalConeCounts.size(); i++)
   {
      FvectorStream << " " << MaximalConeCounts[i];
   };
}
