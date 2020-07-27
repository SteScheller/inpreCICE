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
#include "adapter/inpreciceadapter.hpp"

//-----------------------------------------------------------------------------
// types
//-----------------------------------------------------------------------------
struct ProgramSettings
{
    std::string meshFile;
    std::string preciceConfig;
    int bmCase;

    ProgramSettings() : meshFile(), preciceConfig(), bmCase(1) {}
    ProgramSettings(
            const std::string &mesh,
            const std::string &preciceConf) :
        ProgramSettings()
    {
        meshFile = mesh;
        preciceConfig = preciceConf;
    }
};
//-----------------------------------------------------------------------------
// function prototypes
//-----------------------------------------------------------------------------
int applyProgramOptions(int argc, char *argv[], ProgramSettings &settings);

//-----------------------------------------------------------------------------
// function implementations
//-----------------------------------------------------------------------------
/**
 * \brief Main entry point of the program
 */
int main(int argc, char *argv[])
{
    ProgramSettings settings("./vis-mesh.json", "./precice-config.xml");

    // initialize the renderer
    if (EXIT_FAILURE == applyProgramOptions(argc, argv, settings))
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
        "Visualization", settings.preciceConfig, settings.bmCase, 0, 1);
    interface.initialize(settings.meshFile);

    // Run precice (runs a thread)
    interface.runCouplingThreaded();

    // get data from coupling and draw it
    bool run = true;
    int ret = EXIT_SUCCESS;
    while(run)
    {
        const inpreciceadapter::VisualizationDataInfoVec_t visData =
                interface.getVisualisationData();
        if (settings.bmCase == 1)
            ret = renderer.drawSingleFracture(visData[0].buffers[0]);
        else if (settings.bmCase == 2)
        {
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

            ret = renderer.drawFractureNetwork(dataArray);
        }
        else
        {
            std::cout << "Error: Unsupported benchmark case!" << std::endl;
            return EXIT_FAILURE;
        }

        if (EXIT_FAILURE == ret)
            std::cout << "Error: Renderer draw call reported a failure!\n";

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
 * \param settings
 * reference to structure containing the settings of the program
 *
 * \return  EXIT_SUCCESS or EXIT_FAILURE depending on success of parsing the
 *          program arguments
 */
int applyProgramOptions(int argc, char *argv[], ProgramSettings &settings)
{
    int ret = EXIT_SUCCESS;

    // declare the supported options
    po::options_description generic("Generic options");
    generic.add_options()
        ("help,h", "produce help message")
        ("mesh,m",
         po::value<std::string>(),
         "json file containing the visualization meshes")
    ;

    // positional arguments are hidden options
    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("precice-config",
         po::value<std::string>(),
         "preCICE .xml-file with coupling configuration")
        ("case",
         po::value<int>(),
         "number of fracture benchmark case")
    ;
    po::positional_options_description p;
    p.add("precice-config", 1);
    p.add("case", 1);

    po::options_description all("All options");
    all.add(generic).add(hidden);

    try
    {
        po::variables_map vm;
        po::store(
                po::command_line_parser(
                    argc,
                    argv).options(all).positional(p).run(),
                vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            std::cout <<
                "Usage: inpreCICE [options] PRECICE-CONFIG-PATH CASE-NUMBER\n"
                << std::endl;
            std::cout << generic << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (    (vm.count("precice-config") != 1) ||
                (vm.count("case") != 1) )
        {
            std::cout <<
                "Usage: inpreCICE [options] PRECICE-CONFIG-PATH CASE-NUMBER\n"
                << std::endl;
            std::cout << generic << std::endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            settings.preciceConfig = vm["precice-config"].as<std::string>();
            settings.bmCase = vm["case"].as<int>();
        }


        if (vm.count("mesh") > 0)
            settings.meshFile = vm["mesh"].as<std::string>();
    }
    catch(std::exception &e)
    {
        std::cout << "Invalid program options!" << std::endl;
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return ret;
}

