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

#ifndef _EncodingExtractor_hpp_INCLUDED
#define _EncodingExtractor_hpp_INCLUDED

#include "SolverWrapperBase.hh"
#include "basic_clset.hh"
#include "common_types.hh"
#include "id_manager.hh"

// Class for extracting the encoding from PackUP for using it with another main
// process
class EncodingExtractor : public SolverWrapperBase<BasicClause *> {
public:
  EncodingExtractor(IDManager &id_manager) : _id_manager(id_manager) {}

  // Implementation of methods for SolverWrapper
  void init() { clause_set.set_def_cl_weight(0); }
  XLINT get_top() { return clause_set.get_top(); }
  void set_top(XLINT top) { clause_set.set_top(top); }
  bool solve() { return true; } // Solving not implemented
  IntVector &get_model() { return model; }
  bool has_solution() { return false; } // Never has a solution
  void dump(ostream &out) {}            // Dump not implemented
  bool register_weight(XLINT weight);
  XLINT get_min_unsat_cost() { return get_top(); }

  BasicClauseSet &get_clause_set() { return clause_set; }
  WeightSet &get_obj_multipliers() { return obj_multipliers; }

protected:
  // Implementation of methods for SolverWrapperBase
  void _output_clause(/*const*/ LiteralVector &literals);
  void _output_unary_clause(LINT l);
  void _output_binary_clause(LINT l1, LINT l2);
  void _output_weighted_clause(/*const*/ LiteralVector &literals, XLINT weight);
  void _output_unary_weighted_clause(LINT l, XLINT weight);
  void _output_binary_weighted_clause(LINT l1, LINT l2, XLINT weight);
  BasicClause *_record_clause(LiteralVector &literals);
  void _increase_weight(BasicClause *clause, XLINT weight);

  bool has_weight(XLINT weight) const {
    FOR_EACH(WeightSet::const_iterator, weight_index, obj_multipliers)
    if (*weight_index == weight)
      return true;
    return false;
  }

private:
  IntVector model{}; // Will never be assigned but needs to exist
  IDManager &_id_manager;
  BasicClauseSet clause_set; // The encoding
  WeightSet obj_multipliers;
};

#endif