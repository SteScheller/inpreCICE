#include <iostream>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <string>

#include <boost/multi_array.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "draw/draw.hpp"
#include "adapter/inpreciceadapter.h"

//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------
int applyProgramOptions(int argc, char *argv[], std::string &meshFile);

//-----------------------------------------------------------------------------
// function implementations
//-----------------------------------------------------------------------------
/**
 * \brief Main entry point of the program
 */
int main(int argc, char *argv[])
{
    std::string meshFile("./visus-mesh.json");

    // initialize the renderer
    if (EXIT_FAILURE == applyProgramOptions(argc, argv, meshFile))
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

    inpreciceadapter::InpreciceAdapter interface(
        "Visus", "precice-config.xml", 0, 1 );

    interface.initialize(meshFile);

    // Run precice (runs a thread)
    interface.runCouplingThreaded();


    // get data from coupling and draw it
    bool run = true;
    while(run)
    {
      {
        const inpreciceadapter::VisualizationDataInfoVec_t visData =
            interface.getVisualisationData();
        const draw::Renderer::fractureDataArray_t dataArray = {
            visData[0].buffers[0],
            visData[1].buffers[0],
            visData[2].buffers[0],
            visData[3].buffers[0],
            visData[4].buffers[0],
            visData[5].buffers[0],
            visData[6].buffers[0],
            visData[7].buffers[0],
            visData[8].buffers[0]};

        if (EXIT_FAILURE == renderer.drawFractureNetwork(dataArray))
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
 * \param argc  number of input arguments
 * \param argv  array of char pointers to the input arguments
 * \param mesh  ref to variable containing the path of the visualization mesh
 *
 * \return  EXIT_SUCCESS or EXIT_FAILURE depending on success of parsing the
 *          program arguments
 */
int applyProgramOptions(int argc, char *argv[], std::string& meshFile)
{
    int ret = EXIT_SUCCESS;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("mesh,m", po::value<std::string>(), "json file containing the visualization mesh")
    ;

    try
    {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (vm.count("mesh") > 0)
            meshFile = vm["mesh"].as<std::string>();
    }
    catch(std::exception &e)
    {
        std::cout << "Invalid program options!" << std::endl;
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return ret;
}

