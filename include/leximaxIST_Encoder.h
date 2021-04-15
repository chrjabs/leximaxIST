#ifndef LEXIMAXIST_ENCODER
#define LEXIMAXIST_ENCODER
#include <leximaxIST_types.h>
#include <IpasirWrap.h>
#include <leximaxIST_parsing_utils.h>
#include <string> // std::string
#include <vector> // std::vector
#include <utility> // std::pair
#include <list> // std::list
#include <sys/types.h> // pid_t


namespace leximaxIST
{

    class Encoder {
        /* TODO: check if pointers can be removed without affecting performance
         * The problem is reallocation of vector while growing
         * It seems that it is not mandatory that the implementation of the STL
         * uses the move constructor while copying the elements of the vector
         * So it is not guaranteed for every compiler that the performance will
         * not be affected
         * TODO: signal handling: (1) external solver (2) internal ipasir solver
         */

    private:

        int m_verbosity; // 0: nothing, 1: solving phases, time + obj vector, 2: everything including encoding
        int m_id_count;
        int m_input_nb_vars; // number of vars of input problem - useful to return assignment of only these variables
        std::vector<Clause*> m_hard_clauses; // TODO: remove pointers?
        std::vector<int> m_soft_clauses; // unit clauses
        std::vector<std::vector<int>*> m_objectives; // TODO: remove pointers?
        int m_num_objectives;
        std::vector<std::vector<int>*> m_sorted_vecs; // TODO: remove pointers?
        std::vector<std::vector<std::vector<int>*>>  m_sorted_relax_collection; // TODO: remove pointers?
        std::vector<std::list<int>> m_all_relax_vars; // relax_vars of each iteration
        std::string m_ext_solver_cmd; // for external call to optimisation solver
        std::string m_formalism;
        std::string m_lp_solver;
        std::string m_valid_lp_solvers[6];
        std::string m_file_name;
        std::string m_opt_mode; // internal (binary, linear-su), or external (MaxSAT solver)
        pid_t m_child_pid;
        double m_timeout; // timeout for signal handling in milliseconds
        bool m_leave_tmp_files;
        bool m_simplify_last; // if true the algorithm does not use the sorting networks in the last iteration
        char m_status; // 's' for SATISFIABLE, 'u' for UNSATISFIABLE, and '?' for UNKNOWN
        int m_ub_presolve; // 0: sat call, 1: MSS - sequential choice, 2: MSS - maximum choice
        bool m_maxsat_presolve; // to get lower bound (and upper bound) of optimum
        std::string m_maxsat_psol_cmd;
        // the next one is usefull if computation is stoped and you get an intermediate solution
        // you want to know which values of the objective vector are in theory guaranteed to be optimal
        //int m_num_opts; // number of optimal values found: 0 = none; 1 = first maximum is optimal; 2 = first and second; ...
        std::string m_multiplication_string;
        std::vector<int> m_solution;
        size_t m_sorting_net_size; // size of largest sorting network
        //std::vector<double> m_times; // time of each step of solving (only external solver times)
        IpasirWrap *m_sat_solver;
        
    public:    

        Encoder(); 

        ~Encoder();
        
        void solve();
        
        char get_status() const;
        
        //int get_num_opts() const;
        
        //const std::vector<double>& get_times() const;
        
        /* if the problem is satisfiable, then m_solution is a satisfying assignment;
        * each entry i of m_solution is +i if variable i is true and -i otherwise;
        * if the problem is not satisfiable, m_solution is empty.*/
        std::vector<int> get_solution() const;
        
        // empty if unsat; not a const reference because it is not a member variable
        std::vector<int> get_objective_vector() const;
        
        void set_problem(const std::vector<std::vector<int>> &constraints, const std::vector<std::vector<std::vector<int>>> &objective_functions);
        
        void set_simplify_last(bool val);
        
        void set_ub_presolve(int val);
        
        void set_timeout(double val); // for terminate function
        
        void set_opt_mode(const std::string &mode);
        
        void set_ext_solver_cmd(const std::string &command);
        
        void set_formalism(const std::string &format);
        
        void set_verbosity(int v); // if value is invalid the program is terminated
        
        void set_lp_solver(const std::string &lp_solver);
        
        void set_leave_tmp_files(bool val);
        
        void set_multiplication_string(const std::string &str);
        
        void set_maxsat_presolve(bool v);
        
        void set_maxsat_psol_cmd(const std::string &cmd);
        
        int terminate(); // kill external solver and read approximate solution
        
        void clear(); // frees memory, and sets internal parameters to their initial value
        
    private:
        
        // setters.cpp
        
        int fresh();
        
        void reset_file_name();
        
        void update_id_count(const Clause &clause);
        
        void set_solution(std::vector<int> &model);
        
        // getters.cpp
        
        std::vector<int> get_objective_vector(const std::vector<int> &assignment) const;
        
        // constructors.cpp
        
        void add_hard_clause(const Clause &c);
        
        void add_hard_clause(int l);
        
        void add_hard_clause(int l1, int l2);
        
        void add_hard_clause(int l1, int l2, int l3);
        
        // destructor.cpp
        
        void clear_soft_clauses();
        
        void clear_sorted_relax();
        
        void clear_hard_clauses();
        
        void clear_ipasir();
        
        // sorting_net.cpp
        
        void encode_max(int var_out_max, int var_in1, int var_in2);
        
        void encode_min(int var_out_min, int var_in1, int var_in2);
        
        void insert_comparator(int el1, int el2, const std::vector<int> *objective, SNET &sorting_network);
        
        void odd_even_merge(std::pair<std::pair<int,int>,int> seq1, std::pair<std::pair<int,int>,int> seq2, const std::vector<int> *objective, SNET &sorting_network);
        
        void encode_network(std::pair<int,int> elems_to_sort, const std::vector<int> *objective, SNET &sorting_network);
        
        //void delete_snet(SNET &sorting_network);
        
        // encoding.cpp
        
        void encode_sorted();
        
        size_t largest_obj();
        
        void order_encoding(const std::vector<int>& vars);
        
        void generate_soft_clauses(int i);
        
        void all_subsets(std::list<int> set, int i, Clause &clause);
        
        void at_most(const std::list<int> &set, int i);
        
        void encode_relaxation(int i);
        
        void componentwise_OR(int i);
        
        int encode_upper_bound(int i);
        
        void encode_ub_sorted(int first_max);
        
        void encode_ub_soft(int max_i);
        
        void fix_soft_vars(int i);
        
        void fix_all(int i);
        
        void fix_only_some();
        
        // solver_call.cpp
        
        void internal_solve(int i, int lb, int ub);
        
        void search(int i, int lb, int ub);
        
        void get_sol_and_bound(int i, int &ub);
        
        void mss_add_falsified (IpasirWrap &solver, std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector) const;
        
        int mss_choose_var (std::vector<std::vector<int>> &todo_vec, std::vector<int> &obj_vector, int &obj_index) const;
        
        void mss_solve();
        
        void sat_solve();
        
        int get_lower_bound(const std::vector<int> &model);
        
        int maxsat_presolve();
        
        int presolve();
        
        void remove_tmp_files() const;

        void split_command(const std::string &command, std::vector<std::string> &command_split);
        
        void call_ext_solver();
        
        void read_solver_output(std::vector<int> &model);
        
        void external_solve(int i);
        
        void write_solver_input(int i);
        
        void write_cnf_file(int i);
        
        void write_lp_file(int i);
        
        void write_opb_file(int i);
        
        void write_wcnf_file(int i);
        
        void read_sat_output(std::vector<int> &model, bool &sat, StreamBuffer &r);
        
        void read_cplex_output(std::vector<int> &model, bool &sat, StreamBuffer &r);
        
        void read_gurobi_output(std::vector<int> &model, bool &sat, StreamBuffer &r);
        /*
        int read_glpk_output(std::vector<int> &model);
        
        int read_lpsolve_output(std::vector<int> &model);
        
        int read_scip_output(std::vector<int> &model);
        
        int read_cbc_output(std::vector<int> &model);
        */
        
        // printing.cpp
        
        void print_hard_clauses(std::ostream &output) const;
        
        void print_soft_clauses(std::ostream &output) const;
        
        void print_soft_clauses() const;
        
        void print_sorted_vec (int i) const;
        
        void print_sorted_true() const; // for debugging
        
        void print_obj_func(int i) const;
        
        void print_snet_size(int i) const;
        
        void print_mss_todo(const std::vector<std::vector<int>> &todo_vec) const;
        
        void print_obj_vector(const std::vector<int> &obj_vec) const;
        
        void print_obj_vector() const; // this one computes the obj_vector
        
        void print_waitpid_error(const std::string &errno_str) const;
        
        void print_clause(std::ostream &output, const Clause *cl, const std::string &leadingStr = "") const;
        
//         void print_wcnf_clauses(std::ostream &output, const std::vector<Clause*> &clauses, size_t weight) const;
        
//         void print_atmost_lp(int i, std::ostream &output) const;
        
        void print_lp_constraint(const Clause *cl, std::ostream &output) const;
        
//         void print_sum_equals_lp(int i, std::ostream &output) const;
        
//         void print_atmost_pb(int i, std::ostream &output) const;
        
        void print_pb_constraint(const Clause *cl, std::ostream &output) const;
        
//         void print_sum_equals_pb(int i, std::ostream &output) const;

    };

}

#endif
