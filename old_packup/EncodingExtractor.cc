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

#include "EncodingExtractor.hh"

void EncodingExtractor::_output_clause(/*const*/ LiteralVector &literals) {
  BasicClause *clause = clause_set.create_clause(literals);
  clause_set.set_cl_hard(clause);
}

void EncodingExtractor::_output_unary_clause(LINT l) {
  BasicClause *clause = clause_set.create_unit_clause(l);
  clause_set.set_cl_hard(clause);
}

void EncodingExtractor::_output_binary_clause(LINT l1, LINT l2) {
  BasicClause *clause = clause_set.create_binary_clause(l1, l2);
  clause_set.set_cl_hard(clause);
}

void EncodingExtractor::_output_weighted_clause(
    /*const*/ LiteralVector &literals, XLINT weight) {
  BasicClause *clause = clause_set.create_clause(literals);
  clause_set.incr_cl_weight(clause, weight);
}

void EncodingExtractor::_output_unary_weighted_clause(LINT l, XLINT weight) {
  BasicClause *clause = clause_set.create_unit_clause(l);
  clause_set.incr_cl_weight(clause, weight);
}

void EncodingExtractor::_output_binary_weighted_clause(LINT l1, LINT l2,
                                                       XLINT weight) {
  BasicClause *clause = clause_set.create_binary_clause(l1, l2);
  clause_set.incr_cl_weight(clause, weight);
}

BasicClause *EncodingExtractor::_record_clause(LiteralVector &literals) {
  BasicClause *clause = clause_set.create_clause(literals);
  return clause;
}

void EncodingExtractor::_increase_weight(BasicClause *clause, XLINT weight) {
  assert(has_weight(weight));
  clause_set.incr_cl_weight(clause, weight);
}

bool EncodingExtractor::register_weight(XLINT weight) {
  std::pair<WeightSet::const_iterator, bool> r = obj_multipliers.insert(weight);
  return r.second;
}