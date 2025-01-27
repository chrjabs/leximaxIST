/*
 * Author: Christoph Jabs - christoph.jabs@helsinki.fi
 *
 * Copyright © 2022 Christoph Jabs, University of Helsinki
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "Mcnf.h"

#include <cassert>
#include <fstream>

namespace leximaxIST {

inline int var(Lit l) { return l < 0 ? -l : l; }

Mcnf::Mcnf(std::string fileName) {
  // Parse file
  std::string line{};
  std::vector<std::string> toks{};
  std::fstream file;
  file.open(fileName, std::fstream::in);

  while (getline(file, line)) {
    if (line[0] == 'c')
      continue;
    size_t pos = 0;
    toks.clear();
    while ((pos = line.find(" ")) != std::string::npos) {
      toks.push_back(line.substr(0, pos));
      line.erase(0, pos + 1);
    }
    toks.push_back(line);
    assert(toks.size() >= 1);
    if (toks[0] == "h") {
      // Parse clause
      assert(toks.back() == "0");
      Clause cl{};
      Lit l{};
      for (uint32_t i = 1; i < toks.size() - 1; i++) {
        l = std::stoi(toks[i]);
        cl.push_back(l);
      }
      hards.push_back(cl);
      continue;
    }
    if (toks[0][0] == 'o') {
      // Soft clause
      assert(toks.size() >= 4);
      assert(toks.back() == "0");
      size_t idx = std::stoi(toks[0].substr(1)) - 1;
      uint64_t w = std::stoi(toks[1]);
      Clause cl{};
      for (size_t i = 2; i < toks.size() - 1; ++i) {
        Lit l = std::stoi(toks[i]);
        cl.push_back(l);
      }
      if (softs.size() <= idx) {
        softs.resize(idx + 1);
      }
      softs[idx].emplace_back(std::make_pair(w, cl));
      continue;
    }
    exit(1);
  }
}

} // namespace leximaxIST
