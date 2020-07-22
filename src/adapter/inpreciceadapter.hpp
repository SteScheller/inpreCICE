#pragma once

#include <array>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <boost/multi_array.hpp>

#include <precice/SolverInterface.hpp>

namespace inpreciceadapter {

struct VisualizationDataInfo
{
    int meshID{-1};
    std::vector<int> dataIDs{};
    std::vector<std::string> dataNames{};
    std::string meshName{""};

    std::array< size_t, 2 > gridDimension{};
    std::vector<int> vertexIDs{};
    std::vector<boost::multi_array<double, 2> > buffers{};
};

std::ostream& operator<<(std::ostream& os, const VisualizationDataInfo& info);

using VisualizationDataInfoVec_t = std::vector<VisualizationDataInfo>;

class InpreciceAdapter
{
    // Alias for grid dimension
    using gridDimension_t = std::array< size_t, 2 >;

    private:
    // Unique pointer to preCICe instance
    std::unique_ptr<precice::SolverInterface> interface_;
    // Mutex for data access when threaded execution is run
    std::mutex dataMutex_;

    // Number of the fracture benchmark case
    int bmCase_;

    // True if the visualization mesh has been created
    bool meshIsCreated_;
    // True if initialize() routine of preCICE has been called
    bool preciceIsInitialized_;

    double timeStepSize_;

    VisualizationDataInfoVec_t visInfoData_;

    std::thread preciceThread_;

    void runCoupling(VisualizationDataInfoVec_t& visInfoDataVec_);

    public:
    // No standard constructor as we want to create preCICE instance on creation of adapter
    InpreciceAdapter() = delete;
    // No copy constructor. We only want one instance of preCICE
    InpreciceAdapter( const InpreciceAdapter& ) = delete;

    InpreciceAdapter(const std::string& solverName,
                     const std::string& configurationFileName,
                     const int bmCase,
                     const size_t solverProcessRank,
                     const size_t solverNumberOfProcesses );



    void setMeshName( const std::string& meshName );
    void setVisualizationMesh( const std::string& meshFilePath );

    void initialize(const std::string& meshFilePath);

    void runCouplingThreaded();

    const VisualizationDataInfoVec_t& getVisualisationData();

    void finalize();

    virtual ~InpreciceAdapter();
};

} //namespace inpreciceadapter

