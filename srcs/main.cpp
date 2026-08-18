#include "program.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main() {

    try {
        Program program;
        program.run();
    } catch (std::exception const & e) {
        std::cerr << e.what() << std::endl;
        return (EXIT_FAILURE);
    }

    return (EXIT_SUCCESS);
}