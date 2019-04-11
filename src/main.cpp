#include <iostream>
#include <cstdlib>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "draw/draw.hpp"

//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------
int applyProgramOptions(int argc, char *argv[]);

//-----------------------------------------------------------------------------
// function implementations
//-----------------------------------------------------------------------------
/**
 * \brief Main entry point of the program
 */
int main(int argc, char *argv[])
{
    if (EXIT_FAILURE == applyProgramOptions(argc, argv))
    {
        std::cout << "Error: Parsing of program options failed!" << std::endl;
        return EXIT_FAILURE;
    }

    draw::Renderer renderer;

    if (EXIT_SUCCESS == renderer.initialize())
        std::cout << "Renderer was successfully initialized!" << std::endl;
    else
    {
        std::cout << "Error: Renderer initialization failed!" << std::endl;
        return EXIT_FAILURE;
    }

    while(renderer.processEvents())
    {
        if (EXIT_FAILURE == renderer.draw())
        {
            std::cout << "Error: Renderer draw call reported a failure!"
                << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/**
 * \brief Takes in input arguments, parses and loads the specified data
 *
 * \param   argc number of input arguments
 * \param   argv array of char pointers to the input arguments
 *
 * \return  EXIT_SUCCESS or EXIT_FAILURE depending on success of parsing the
 *          program arguments
 */
int applyProgramOptions(int argc, char *argv[])
{
    return EXIT_SUCCESS;
}
