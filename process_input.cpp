#include "process_input.h"
#include <string>
#include <fstream>
#include <streambuf>

//------------------------------------------------------------------------------
vector<vector<vector<int> > > ParseSupportFile(string FileName)
{
   ifstream t(FileName);
   string str((std::istreambuf_iterator<char>(t)),std::istreambuf_iterator<char>());
   str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
   str = "[" + str + "]";
   return ParseToSupport(str);
};

