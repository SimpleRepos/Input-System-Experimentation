#include "ns_Utility.h"
#include <fstream>

Utility::OnScopeExit::OnScopeExit(std::function<void()> func) {
  reset(func);
}

Utility::OnScopeExit::~OnScopeExit() {
  func();
}

void Utility::OnScopeExit::reset(std::function<void()> func) {
  this->func = func;
}

void Utility::OnScopeExit::release() {
  reset([]() {});
}

std::vector<char> Utility::readFile(const std::string& filename) {
  std::ifstream file(filename, std::ifstream::binary);
  assert(file);

  file.seekg(0, std::ifstream::end);
  size_t len = (size_t)file.tellg();
  file.seekg(0, std::ifstream::beg);

  std::vector<char> data(len);
  file.read(data.data(), data.size());

  return data;
}

