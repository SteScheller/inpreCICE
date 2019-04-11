#include <iostream>
#include <cstdlib>
#include <array>
#include <cstdio>
#include <thread>

#include <boost/multi_array.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <precice/SolverInterface.hpp>
#include "draw/draw.hpp"

//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------
int applyProgramOptions(int argc, char *argv[]);


void doPreciceCoupling( precice::SolverInterface& interface, 
    const double timestepSize,
    std::vector<int>& vertexIDs, 
    const int pressureId,
    boost::multi_array<double, 2>& pressure, 
    const int concentrationId,
    boost::multi_array<double, 2>& concentration )
{
  while(interface.isCouplingOngoing())
  {
    interface.readBlockScalarData(
        pressureId,
        vertexIDs.size(),
        vertexIDs.data(),
        pressure.data());

    interface.readBlockScalarData(
        concentrationId,
        vertexIDs.size(),
        vertexIDs.data(),
        concentration.data());
    
    for (size_t y=0; y < concentration.shape()[1]; ++y)
    {
      for (size_t x=0; x < concentration.shape()[0]; ++x)
        std::printf("%.3f ", concentration[y][x]);
      std::cout << std::endl;
    }
 
    interface.advance(timestepSize);
  }


}

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

    // setup coupling with precice
    precice::SolverInterface interface("Visus", 0, 1);
    interface.configure("precice-config.xml");

    //const int dim = interface.getDimensions();
    const int meshId = interface.getMeshID("VisusMesh");

    const std::array<size_t, 2> gridDim = {10, 10};
    const size_t numPoints = gridDim[0] * gridDim[1];
    std::vector<int> vertexIDs(numPoints, 0);
    boost::multi_array<double, 2> gridPoints(
            boost::extents[numPoints][3]);

    std::array<double, 2> cellDim =
        {   100.0 / static_cast<double>(gridDim[0]),
            100.0 / static_cast<double>(gridDim[1])};

    for (size_t y = 0; y < gridDim[1]; ++y)
    for (size_t x = 0; x < gridDim[0]; ++x)
    {
        size_t idx = y * gridDim[0] + x;
        gridPoints[idx][0] = x * cellDim[0] + 0.5 * cellDim[0];
        gridPoints[idx][1] = y * cellDim[1] + 0.5 * cellDim[1];
        gridPoints[idx][2] = -0.6 * gridPoints[idx][0] + 80.0;
    }
    interface.setMeshVertices(
            meshId, numPoints, gridPoints.data(), vertexIDs.data());

    const int pressureId = interface.getDataID("Pressure", meshId);
    const int concentrationId = interface.getDataID("Concentration", meshId);

    boost::multi_array<double, 2> pressure(
            boost::extents[gridDim[0]][gridDim[1]]);
    boost::multi_array<double, 2> concentration(
            boost::extents[gridDim[0]][gridDim[1]]);

    double timestepSize = interface.initialize();
    interface.initializeData();

    // Spawn preCICE thread
    std::thread t_precice( doPreciceCoupling, 
        std::ref(interface),
        timestepSize,
        std::ref(vertexIDs),
        pressureId,
        std::ref(pressure),
        concentrationId,
        std::ref(concentration) );

    // get data from coupling and draw it
    bool run = true;
    while(run)
    {
/*
                for (size_t y=0; y < concentration.shape()[1]; ++y)
                {
                    for (size_t x=0; x < concentration.shape()[0]; ++x)
                        std::printf("%.3f ", concentration[y][x]);
                    std::cout << std::endl;
                }
*/
                if (EXIT_FAILURE == renderer.draw(concentration))
                {
                    std::cout << "Error: Renderer draw call reported a failure!"
                        << std::endl;
                }
            run = renderer.processEvents();
    }

    t_precice.join();

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
