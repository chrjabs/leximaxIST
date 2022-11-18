#include <leximaxIST_types.h>
#include <leximaxIST_Solver.h>
#include <leximaxIST_Options.h>
#include <leximaxIST_printing.h>
#include <FormulaPB.h>
#include <MaxSATFormula.h>
#include <ParserPB.h>
#include <Encoder.h>
#include <Mcnf.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <preprocessorinterface.hpp>

leximaxIST::Solver solver;

void print_header()
{
    std::cout << "c -------------------------------------------------------------------------\n";
    std::cout << "c leximaxIST - Boolean Leximax Optimisation Solver\n";
    std::cout << "c Authors: Miguel Cabral, Mikolas Janota, Vasco Manquinho\n";
    std::cout << "c Contributors:\n";
    std::cout << "c     * From Open-WBO: João Cortes, Ruben Martins, Inês Lynce,\n";
    std::cout << "c       Miguel Neves, Norbert Manthey, Saurabh Joshi, Andreia P. Guerreiro.\n";
    std::cout << "c -------------------------------------------------------------------------\n";
}

void signal_handler(int signum) {
  std::cout << "c Received external signal " << signum << '\n'; 
  std::cout << "c Terminating...\n";
  solver.print_solution();
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    // signals
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);
    signal(SIGINT, signal_handler);    
    
    /* parse options */
    leximaxIST::Options options;
    if (!options.parse(argc, argv)) {
        leximaxIST::print_error_msg("Error parsing options. Exiting.");
        options.print_usage(std::cout);
        return 1;
    }
    if (options.get_help() == 1) {
        options.print_usage(std::cout);
        return 0;
    }

    solver.set_verbosity(options.get_verbosity());
    solver.set_leave_tmp_files(options.get_leave_tmp_files());
    
    if (options.get_verbosity() > 0 && options.get_verbosity() <= 2) {
        print_header();
        std::cout << "c Parsing instance file " << options.get_input_file_name() << "...\n";
    }

    maxPreprocessor::PreprocessorInterface * prepro = nullptr;
    
    if (options.get_input_file_type() == leximaxIST::Options::FileType::OPB) {
        // read pbmo file
        leximaxIST::MaxSATFormula maxsat_formula;
        leximaxIST::ParserPB parser_pb (&maxsat_formula);
        parser_pb.parse(options.get_input_file_name().c_str());
        
        // add hard clauses
        for (size_t pos (0); pos < maxsat_formula.nHard(); ++pos) {
            std::vector<int> hc (maxsat_formula.getHardClause(pos).clause);
            solver.add_hard_clause(hc);
        }
        
        // use an encoder to encode the pseudo-boolean constraints to cnf and add the clauses to solver
        leximaxIST::Encoder enc (leximaxIST::_INCREMENTAL_NONE_,
                                options.get_card_enc(),
                                leximaxIST::_AMO_LADDER_,
                                options.get_pb_enc());
        // pb constraints
        for (int i = 0; i < maxsat_formula.nPB(); i++) {
            // Make sure the PB is on the form <=
            if (!maxsat_formula.getPBConstraint(i)._sign)
                maxsat_formula.getPBConstraint(i).changeSign();
            enc.encodePB(solver, maxsat_formula.getPBConstraint(i)._lits,
                            maxsat_formula.getPBConstraint(i)._coeffs,
                            maxsat_formula.getPBConstraint(i)._rhs);
        }
        // cardinality and at most one
        for (int i = 0; i < maxsat_formula.nCard(); i++) {
            if (maxsat_formula.getCardinalityConstraint(i)._rhs == 1) {
                enc.encodeAMO(solver, maxsat_formula.getCardinalityConstraint(i)._lits);
            } else {
                enc.encodeCardinality(solver,
                                        maxsat_formula.getCardinalityConstraint(i)._lits,
                                        maxsat_formula.getCardinalityConstraint(i)._rhs);
            }
        }
        
        // add objective functions
        for (int i (0); i < maxsat_formula.nObjFunctions(); ++i) {
            std::vector<std::pair<uint64_t, leximaxIST::Clause>> soft_clauses;
            const leximaxIST::PBObjFunction &obj (maxsat_formula.getObjFunction(i));
            for (size_t j (0); j < obj._lits.size(); ++j) {
              leximaxIST::Clause sc;
              sc.push_back(-(obj._lits.at(j)));
              soft_clauses.push_back(std::make_pair(obj._coeffs.at(j), sc));
            }
            solver.add_soft_clauses(soft_clauses);
        }
    } else {
        // read dimacs mcnf file
        leximaxIST::Mcnf mcnf(options.get_input_file_name());

        if (options.get_preprocessing()) {
            std::vector<std::vector<int>> clauses{};
            std::vector<std::vector<uint64_t>> weights{};
            uint64_t top_weight = 1;

            clauses.reserve(mcnf.hards.size());
            weights.reserve(mcnf.hards.size());
            for (leximaxIST::Clause cl : mcnf.hards) {
                clauses.push_back(cl);
                weights.push_back({});
            }
            for (size_t idx = 0; idx < mcnf.softs.size(); ++idx) {
                auto obj = mcnf.softs[idx];
                clauses.reserve(clauses.size() + obj.size());
                weights.reserve(weights.size() + obj.size());
                for (auto cl : obj) {
                    top_weight += cl.first;
                    clauses.push_back(cl.second);
                    std::vector<uint64_t> w(idx+1, 0);
                    w.back() = cl.first;
                    weights.push_back(w);
                }
            }

            prepro = new maxPreprocessor::PreprocessorInterface(clauses, weights, top_weight);
            prepro->preprocess(options.get_maxpre_techiques());
            clauses.clear();
            weights.clear();
            std::vector<int> labels{};
            prepro->getInstance(clauses, weights, labels, true);

            std::vector<std::vector<std::pair<uint64_t, leximaxIST::Clause>>> softs(mcnf.softs.size());
            for (size_t i = 0; i < clauses.size(); ++i) {
                leximaxIST::Clause cl = clauses[i];
                std::vector<uint64_t> ws = weights[i];
                bool is_hard = true;
                for (size_t idx = 0; idx < ws.size(); ++idx) {
                    if (ws[idx] == top_weight) continue;
                    softs[idx].push_back(std::make_pair(ws[idx], cl));
                    is_hard = false;
                }
                if (is_hard) solver.add_hard_clause(cl);
            }
            for (auto obj : softs) {
                solver.add_soft_clauses(obj);
            }
        } else {
            for (leximaxIST::Clause cl : mcnf.hards) {
                solver.add_hard_clause(cl);
            }
            for (auto obj : mcnf.softs) {
                solver.add_soft_clauses(obj);
            }
        }

    }
    
    // approximation
    if (!options.get_approx().empty()) {
        solver.set_approx(options.get_approx());
        solver.set_mss_incr(options.get_mss_incr());
        solver.set_mss_add_cls(options.get_mss_add_cls());
        solver.set_mss_tol(options.get_mss_tol());
        solver.set_gia_incr(options.get_gia_incr());
        solver.set_gia_pareto(options.get_gia_pareto());
        solver.set_approx_tout(options.get_timeout());
        solver.approximate();
    }
    // optimisation
    if (!options.get_optimise().empty() && solver.get_status() != 'u') {
        solver.set_disjoint_cores(options.get_disjoint_cores());
        solver.set_ilp_solver(options.get_ilp_solver());
        solver.set_opt_mode(options.get_optimise());
        solver.optimise();
    }
    

    if (prepro) {
        std::vector<int> sol = solver.get_solution();
        std::vector<int> rec_sol = prepro->reconstruct(sol);
        solver.print_solution(rec_sol);
        delete prepro;
    } else {
        solver.print_solution();
    }

    return 0;
}
