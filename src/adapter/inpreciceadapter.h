#ifndef INPRECICEADAPTER_H
#define INPRECICEADAPTER_H

#include <array>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>
#include <boost/multi_array.hpp>

#include <precice/SolverInterface.hpp>

namespace inpreciceadapter {

//struct VisualizationDataInfo {
////  int meshId = -1;
////  int dataId = -1;
//  std::string dataName = "";
//  std::string meshName = "";

////  std::array< size_t, 2 > gridDimension;
//};

struct VisualizationDataInfo  {

  int meshID{-1};
  std::vector<int> dataIDs{};
  std::vector<std::string> dataNames{};
  std::string meshName{""};

  std::array< size_t, 2 > gridDimension{};
  std::vector<int> vertexIDs{};
  std::vector<boost::multi_array<double, 2> > buffers{};
};

std::ostream& operator<<( std::ostream& os, const VisualizationDataInfo& info );
//std::ostream& operator<<( std::ostream& os, const VisualizationDataInfoFull& info );

using VisualizationDataInfoVec_t = std::vector<VisualizationDataInfo>;
//using VisualizationDataInfoFullVec_t = std::vector<VisualizationDataInfoFull>;

namespace detail {

//void setName( const json& conf )
//{

//}

} //namespace detail

class InpreciceAdapter
{
  /// Alias for grid dimension
  using gridDimension_t = std::array< size_t, 2 >;
  private:
    /// Unique pointer to preCICe instance
    std::unique_ptr<precice::SolverInterface> interface_;
    /// Mutex for data access when threaded execution is run
    std::mutex dataMutex_;

    /// Dimension of grid for visualvoid InpreciceAdapter::initialize()
 //    gridDimension_t gridDimension_;

    /// True if the visualization mesh has been created
    bool meshIsCreated_;
    /// True if initialize() routine of preCICE has been called
    bool preciceIsInitialized_;

//    int meshID_;
//    int pressureID_;
//    int concentrationID_;
//    std::vector<int> dataIDs_;

    double timeStepSize_;

//    std::vector<int> vertexIDs_;
//    boost::multi_array<double, 2> pressure_;
//    boost::multi_array<double, 2> concentration_;

    VisualizationDataInfoVec_t visInfoData_;

    std::thread preciceThread_;

//    void runCoupling( boost::multi_array<double, 2>& pressure,
//                      boost::multi_array<double, 2>& concentration,
//                      bool printData = false);

    void runCoupling(VisualizationDataInfoVec_t& visInfoDataVec_,
                     bool printData = false);
//    VisualizationDataInfoFullVec_t
  public:

    // No standard constructor as we want to create preCICE instance on creation of adapter
    InpreciceAdapter() = delete;
    // No copy constructor. We only want one instance of preCICE
    InpreciceAdapter( const InpreciceAdapter& ) = delete;


//    InpreciceAdapter( const size_t solverProcessRank,
//                      const size_t solverNumberOfProcesses );
//    InpreciceAdapter( const std::string& solverName,
//                      const size_t solverProcessRank,
//                      const size_t solverNumberOfProcesses );

    InpreciceAdapter(const std::string& solverName,
                     const std::string& configurationFileName,
                     const size_t solverProcessRank,
                     const size_t solverNumberOfProcesses );


//    void configure( const std::string& configurationFileName );
    void setMeshName( const std::string& meshName );
    void setVisualizationMesh( const std::string& meshFilePath );


//    void setVisualizationMeshes( const std::string& meshFilePath, VisualizationDataInfoVec_t& visInfoVec );


//    void initialize();

//    void initialize(const std::string& meshFilePath,
//                    const VisualizationDataInfoVec_t& visInfoVec );

    void initialize(const std::string& meshFilePath);

    void runCouplingThreaded();

    const boost::multi_array<double, 2>& getPressureVector();
    const boost::multi_array<double, 2>& getConcentrationVector();

    VisualizationDataInfoVec_t getVisualisationData();

    void finalize();

    virtual ~InpreciceAdapter();
};

} //namespace inpreciceadapter

#endif // INPRECICEADAPTER_H
