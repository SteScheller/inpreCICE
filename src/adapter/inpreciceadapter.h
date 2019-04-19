#ifndef INPRECICEADAPTER_H
#define INPRECICEADAPTER_H

#include <array>
#include <mutex>
#include <string>
#include <thread>
#include <boost/multi_array.hpp>

#include <precice/SolverInterface.hpp>

class InpreciceAdapter
{
  /// Alias for grid dimension
  using gridDimension_t = std::array< size_t, 2 >;
  private:
    /// Unique pointer to preCICe instance
    std::unique_ptr<precice::SolverInterface> interface_;
    /// Mutex for data access when threaded execution is run
    std::mutex dataMutex_;

    /// Dimension of grid for visualization
    gridDimension_t gridDimension_;

    /// True if the visualization mesh has been created
    bool meshIsCreated_;
    /// True if initialize() routine of preCICE has been called
    bool preciceIsInitialized_;

    int meshID_;
    int pressureID_;
    int concentrationID_;

    double timeStepSize_;

    std::vector<int> vertexIDs_;
    boost::multi_array<double, 2> pressure_;
    boost::multi_array<double, 2> concentration_;

    std::thread preciceThread_;

    void runCoupling( boost::multi_array<double, 2>& pressure,
                      boost::multi_array<double, 2>& concentration );
  public:

    // No standard constructor as we want to create preCICE instance on creation of adapter
    InpreciceAdapter() = delete;
    // No copy constructor. We only want one instance of preCICE
    InpreciceAdapter( const InpreciceAdapter& ) = delete;


    InpreciceAdapter( const size_t solverProcessRank,
                      const size_t solverNumberOfProcesses );
    InpreciceAdapter( const std::string& solverName,
                      const size_t solverProcessRank,
                      const size_t solverNumberOfProcesses );

    void configure( const std::string& configurationFileName );
    void setMeshName( const std::string& meshName );
    void setVisualizationMesh( const std::string& meshFilePath );
    void initialize();

    void runCouplingThreaded();

    const boost::multi_array<double, 2>& getPressureVector();
    const boost::multi_array<double, 2>& getConcentrationVector();

    void finalize();

    virtual ~InpreciceAdapter();
};

#endif // INPRECICEADAPTER_H
