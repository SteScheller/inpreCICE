#include <iostream>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <cmath>
#include <iostream>

#include <boost/multi_array.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "draw/draw.hpp"
#include "adapter/inpreciceadapter.h"

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
    // initialize the renderer
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

    InpreciceAdapter interface( "Visus", 0, 1 );
    interface.configure("precice-config.xml");
    interface.setMeshName( "VisusMesh" );

    const std::array<size_t, 2> gridDim = {10, 10};
    interface.setVisualizationMesh( gridDim );

    interface.initialize();

    //Run precice (runs a thread)
    interface.runCouplingThreaded();


    // get data from coupling and draw it
    bool run = true;
    while(run)
    {
      {
        if (EXIT_FAILURE == renderer.draw(interface.getConcentrationVector()))
        {
          std::cout << "Error: Renderer draw call reported a failure!"
            << std::endl;
        }
      }
      run = renderer.processEvents();
    }

    interface.finalize();

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
    std::cout << "argc: " << argc << " argv[0]: " << argv[0] << std::endl;
    return EXIT_SUCCESS;
}
