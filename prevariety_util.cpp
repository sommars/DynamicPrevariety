#include "prevariety_util.h"

//------------------------------------------------------------------------------
vector<int> GeneratorToPoint(Generator &g, bool KnockOffLastTerm)
{
   // Converts a PPL Generator object to a vector of int
   vector<int> Result;
   int Dim = g.space_dimension();
   if (KnockOffLastTerm)
      Dim = Dim - 1;
   for (size_t i = 0; i < Dim; i++)
   {
      stringstream s;
      s << (g).coefficient(Variable(i));
      int ToAppend;
      istringstream(s.str()) >> ToAppend;
      Result.push_back(ToAppend);
   }
   return Result;
}

//------------------------------------------------------------------------------
Constraint InequalityToStrictInequality(Constraint &c)
{
   // Converts a PPL inequality to a strict inequality
   Linear_Expression LE;
   for (size_t i = 0; i < c.space_dimension(); i++)
      LE += c.coefficient(Variable(i)) * Variable(i);
   Constraint c2(LE > c.inhomogeneous_term());
   return c2;
}

//------------------------------------------------------------------------------
Constraint InequalityToEquation(Constraint &c)
{
   // Converts a PPL inequality to an equation
   Linear_Expression LE;
   for (size_t i = 0; i < c.space_dimension(); i++)
      LE += c.coefficient(Variable(i)) * Variable(i);
   Constraint c2(LE == c.inhomogeneous_term());
   return c2;
}

//------------------------------------------------------------------------------
vector<int> ConstraintToPoint(Constraint &c)
{
   // Converts a PPL Constraint to a point
   vector<int> Result;
   for (size_t i = 0; i < c.space_dimension(); i++)
   {
      stringstream s;
      s << (c).coefficient(Variable(i));
      int ToAppend;
      istringstream(s.str()) >> ToAppend;
      Result.push_back(ToAppend);
   }
   return Result;
}

//------------------------------------------------------------------------------
double DoubleInnerProduct(vector<int> &V1, vector<double> &V2)
{
   // Computes the inner product of two vectors.
   if (V1.size() != V2.size())
      throw out_of_range("Internal error: DoubleInnerProduct for vectors"
                             "with different sizes");
   double Result = 0;
   for (size_t i = 0; i != V1.size(); i++)
      Result += V1[i] * V2[i];
   return Result;
}

//------------------------------------------------------------------------------
vector<Support> ParseToSupport(string &Input, bool HasSigns)
{
   // Takes in strings that look like this:
   //'[[[1,0,0,0][0,1,0,0][0,0,1,0][0,0,0,1]]' + 
   //'[[1,1,0,0][0,1,1,0][1,0,0,1][0,0,1,1]]' +
   //'[[1,1,1,0][1,1,0,1][1,0,1,1][0,1,1,1]]]'
   vector<Support> Result;
   int ParenCount = 0;
   string NewTerm;
   Support Polynomial;
   SupportPoint Monomial;
   for (string::iterator it=Input.begin(); it!=Input.end(); ++it)
   {
      if ((*it) == ' ')
         continue;
      else if ((*it) == '[')
         ParenCount++;
      else if ((*it) == ']')
      {
         ParenCount--;
         if (NewTerm.length() > 0)
         {
            if (NewTerm == "+")
               Monomial.Sign = PLUS;
            else if (NewTerm == "-")
               Monomial.Sign = MINUS;
            else
            {
               Monomial.Sign = NONE;
               Monomial.Pt.push_back(stoi(NewTerm));
            }
            NewTerm.clear();
         };
         if (ParenCount == 2)
         {
            Polynomial.Pts.push_back(Monomial);
            Monomial.Pt.clear();
         } else if (ParenCount == 1)
         {
            Result.push_back(Polynomial);
            Polynomial.Pts.clear();
         };
      } else if ((*it) == ',')
      {
         if (NewTerm.length() > 0)
         {
            Monomial.Pt.push_back(stoi(NewTerm));
            NewTerm.clear();
         };
      } else
         NewTerm += (*it);
   };

   for (size_t i = 0; i != Result.size(); i++)
   {
      for (size_t j = 0; j != Result[i].Pts.size(); j++)
      {
         if (HasSigns && Result[i].Pts[j].Sign == NONE)
            throw invalid_argument("All points must have signs when run with the sign option.");

         if (!HasSigns && Result[i].Pts[j].Sign != NONE)
            throw invalid_argument("No points may have signs. If you want to use signs, run with the appropriate option.");
      };
   };
   
   return Result;
};

//------------------------------------------------------------------------------
vector<Support> ParseSupportFile(string &FileName, bool HasSigns)
{
   ifstream t(FileName);
   string str((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
   t.close();
   str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
   str = "[" + str + "]";
   return ParseToSupport(str, HasSigns);
};

//------------------------------------------------------------------------------
C_Polyhedron RaysToCone(vector<vector<int> > &Rays)
{
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
};
