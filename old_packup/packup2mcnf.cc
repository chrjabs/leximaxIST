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

// Tool for dumping PackUP instances as a multi-objective MCNF (DIMACS
// WCNF-like) instance.
// The first parameter should be an objective specification, the second a .cudf
// instance file and the third the output path.

#define ENCODING_EXTRACTOR

#include "ConverterMem.hh"
#include "EncodingExtractor.hh"
#include "Lexer.hh"
#include "basic_clause.hh"
#include "common_types.hh"
#include "id_manager.hh"

using std::string;
using std::vector;

extern int yyparse(void); // PackUP parser generated by bison
extern Lexer *lexer;      // Lexer to use in the main parsing function

IDManager packupIdManager;
EncodingExtractor packupExtractor(packupIdManager);
ConverterMem parser(packupExtractor, packupIdManager);

bool parseObjSpec(char *objSpec, vector<Objective> &objs) {
  std::stringstream ss(objSpec);
  std::string item;
  while (std::getline(ss, item, ',')) {
    const bool maximize(item[0] == '+');
    const string sfunction(item.substr(1));
    OBJECTIVE_FUNCTION function;
    if (sfunction.compare("new") == 0) {
      function = COUNT_NEW;
    } else if (sfunction.compare("unmet_recommends") == 0) {
      function = COUNT_UNMET_RECOMMENDS;
    } else if (sfunction.compare("removed") == 0) {
      function = COUNT_REMOVED;
    } else if (sfunction.compare("notuptodate") == 0) {
      function = COUNT_NOT_UP_TO_DATE;
    } else if (sfunction.compare("changed") == 0) {
      function = COUNT_CHANGED;
    } else {
      cerr << "unknown criterion " << sfunction << endl;
      return false;
    }
    objs.push_back(Objective(function, maximize));
  }
  return true;
}

void dumpInstance(const char *outPath, const char *instance) {
  std::ofstream file(outPath);
  file << "c Multi-Objective Dimacs file generated by packup2mcnf\n";
  file << "c Original instance file: " << instance << "\n";

  BasicClauseSet packupClauses = packupExtractor.get_clause_set();
  WeightSet packupObjMultUnsorted = packupExtractor.get_obj_multipliers();
  vector<long long> packupObjMult(packupObjMultUnsorted.begin(),
                                  packupObjMultUnsorted.end());
  std::sort(packupObjMult.begin(), packupObjMult.end(), greater<>());

  for (auto clausePtr : packupClauses) {
    vector<int> clause{};
    if (packupClauses.is_cl_hard(clausePtr)) {
      file << "h ";
      for (auto it = clausePtr->begin(); it != clausePtr->end(); ++it)
        file << *it << " ";
      file << "0\n";
    } else {
      auto clWeight = packupClauses.get_cl_weight(clausePtr);
      for (uint objIdx = 0; objIdx < packupObjMult.size(); objIdx++) {
        // Split objective
        const auto objMult = packupObjMult[objIdx];
        const auto clObjWeight = clWeight / objMult;
        clWeight %= objMult;
        if (clObjWeight > 0) {
          file << "o" << objIdx + 1 << " " << clObjWeight << " ";
          for (auto it = clausePtr->begin(); it != clausePtr->end(); ++it)
            file << *it << " ";
          file << "0\n";
        }
      }
    }
  }

  file.close();
}

int main(int argc, char **argv) {
  parser.get_encoder().set_opt_edges(1);
  parser.get_encoder().set_iv(3);
  parser.get_encoder().set_opt_not_removed(true);
  if (argc != 4) {
    std::cerr << "Useage: packup2mcnf <obj spec> <inst> <output file>"
              << "\n";
    exit(1);
  }
  vector<Objective> objs{};
  if (!parseObjSpec(argv[1], objs)) {
    std::cerr << "Invalid objective specification: " << argv[1] << "\n";
    exit(1);
  }
  parser.get_encoder().set_lexicographic(objs);
  ifstream inFile;
  inFile.open(argv[2]);
  if (inFile.fail()) {
    std::cerr << "Failed to read instance: " << argv[2] << "\n";
    exit(1);
  }
  lexer = new Lexer(inFile);
  std::cout << "Running PackUP\n";
  try {
    yyparse();
  } catch (const ReadException &e) {
    std::cerr << "Error in parsing the PackUP input file: " << e.what() << "\n";
    exit(1);
  }
  std::cout << "=> Finished running PackUP, starting dump to " << argv[3]
            << "\n";

  dumpInstance(argv[3], argv[2]);
}
