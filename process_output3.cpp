#include "process_output3.h"

//------------------------------------------------------------------------------
C_Polyhedron GetConeFromTree(TropicalPrevariety &TP, int i, int j)
{
   vector<vector<int> > Rays;
   for (set<int>::iterator Itr = TP.ConeTree[i][j].RayIndices.begin(); 
      Itr != TP.ConeTree[i][j].RayIndices.end(); Itr++)
      Rays.push_back(TP.IndexToRayMap[*Itr]);
   
   Generator_System gs;
   for (vector<vector<int> >::iterator itr=Rays.begin();
        itr != Rays.end();
        itr++)
   {
      Linear_Expression LE;
      for (size_t i = 0; i != itr->size(); i++)
         LE += Variable(i) * ((*itr)[i]);
      gs.insert(ray(LE));
   };
   Linear_Expression LE;
   for (size_t i = 0; i != Rays[0].size(); i++)
      LE += Variable(i) * 0;
   gs.insert(point(LE));
   
   return C_Polyhedron(gs);
}

//------------------------------------------------------------------------------
void LinearMarkingVersion3(TropicalPrevariety &TP)
{
   for(size_t i = TP.ConeTree.size() - 2; i != -1; i--)
   {
      for(size_t j = 0; j != TP.ConeTree[i].size(); j++)
      {
         if (TP.ConeTree[i][j].Status != 2)
         {
            continue;
         };
         TP.ConeTree[i][j].Status = 3;

         bool WeKnowConeIsNotMaximal = false;
         for(size_t k = TP.ConeTree.size() - 1; k != i; k--)
         {
            for(size_t l = 0; l != TP.ConeTree[k].size(); l++)
            {
               if (TP.ConeTree[k][l].Status == 0)
                  continue;
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
void CleanupOutput(TropicalPrevariety &TP)
{

};

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
            TP.ConeTree[i][j].HOPolyhedron = GetConeFromTree(TP,i,j);
            
         };
      };
   };
   LinearMarkingVersion3(TP);
   CleanupOutput(TP);
   return;
}
