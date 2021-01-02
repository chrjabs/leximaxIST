#include <Leximax_encoder.h>

void Leximax_encoder::print_error_msg(const std::string &msg)
{
    std::cerr << "Error leximaxIST: " << msg << std::endl;
}

void Leximax_encoder::terminate(int signum)
{
    // TODO
    // if external solver is not running (this is checked with m_child_pid) then return solution from last iteration if one exists
    // send signal to child process external solver (if one exists) with the kill function
    // wait a few seconds for the external solver to return the best solution so far
    // read solution
    // if satisfiable set m_sat to true, else m_sat to false and m_solution to empty
    // store solution in m_solution, if one exists
    // determine objective values and store them in optimum, if solution exists
}
