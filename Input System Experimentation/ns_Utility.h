#pragma once
#include <functional>
#include <vector>
#include <cassert>

#define HR(a) { HRESULT hr = a; assert(SUCCEEDED(hr)); }

///<summary>Utility functions and classes</summary>
namespace Utility {
  ///<summary>Executes a function on destruction</summary>
  ///<remarks>This class can be used to easily implement exception-safe design patterns without explicitly coding unusual RAII classes</remarks>
  class OnScopeExit {
  public:
    ///<summary>Generate object that will call its associated function when it leaves scope</summary>
    ///<param name="func">The function to be executed when this object destructs</param>
    OnScopeExit(std::function<void()> func);

    ~OnScopeExit();

    ///<summary>Set a new associated function, replaceing the previous one</summary>
    ///<param name="func">The function to be executed when this object destructs</param>
    void reset(std::function<void()> func);

    ///<summary>Set the associated function to an empty one</summary>
    void release();

  private:
    std::function<void()> func;

  };

  ///<summary>Read a file into a vector of char (binary read)</summary>
  ///<param name="filename">Path to the file to be read</param>
  std::vector<char> readFile(const std::string& filename);

}

