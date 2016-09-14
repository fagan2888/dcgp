#include <iostream>

#include "../src/expression.hpp"
#include "../src/function_set.hpp"

// Here we solve the differential equation d^2y dy  = - 4 / x^3 (NLODE3) from Tsoulos paper
// Tsoulos and Lagaris: "Solving Differential equations with genetic programming"

double fitness(const dcgp::expression& ex, const std::vector<std::vector<double> >& in)
{
    double retval = 0;
    for (auto i = 0u; i < in.size(); ++i) {
        auto T = ex.taylor(in[i], 2);                   // We compute all the derivatives up to order two
        double dy = T[0].get_derivative({1});
        double ddy = T[0].get_derivative({2});
        double x = in[i][0];
        double ode1 = - 4 / x / x / x;          
        retval += (ode1 - ddy*dy) * (ode1 - ddy*dy);    // We compute the quadratic error 
    }
    return retval;
}

int main () {
    // Random seed
    std::random_device rd;

    // Function set
    dcgp::function_set basic_set({"sum", "diff", "mul", "div", "log"});

    // d-CGP expression
    dcgp::expression ex(1, 1, 1, 15, 16, 2, basic_set(), rd());

    // Symbols
    std::vector<std::string> in_sym({"x"});

    // We create the grid over x
    std::vector<double> dumb(1);
    std::vector<std::vector<double> > in(10, dumb);
    for (auto i = 0u; i < in.size(); ++i) {
        in[i][0] = 1 + 1. / (in.size() - 1) * i; // 1, .., 2
    }

    // We run the (1-4)-ES
    double best_fit = 1e32;
    std::vector<double> newfits(4, 0.);
    std::vector<std::vector<unsigned int> > newchromosomes(4);
    std::vector<unsigned int> best_chromosome(ex.get());
    unsigned int gen = 0;

    do
    {
        gen++;
        for (auto i = 0u; i < newfits.size(); ++i) {
            ex.set(best_chromosome);
            ex.mutate_active(2);
            double fitness_ic = ex(std::vector<double>({1}))[0];        // Penalty term to enforce the initial conditions
            newfits[i] = fitness(ex, in) + fitness_ic * fitness_ic;     // Total fitness
            newchromosomes[i] = ex.get();
        }

        for (auto i = 0u; i < newfits.size(); ++i) {
            if (newfits[i] <= best_fit) {
                if (newfits[i] != best_fit) {
                    std::cout << "New best found: gen: " << std::setw(7) << gen << "\t value: " << newfits[i] << std::endl;
                    //std::cout << "Expression: " << ex(in_sym) << std::endl;
                }
                best_fit = newfits[i];
                best_chromosome = newchromosomes[i];
                ex.set(best_chromosome);
            }
        }
    } while (best_fit > 1e-3 && gen < 10000);
    std::cout << "Number of generations: " << gen << std::endl;
    std::cout << "Expression: " << ex(in_sym) << std::endl;
}