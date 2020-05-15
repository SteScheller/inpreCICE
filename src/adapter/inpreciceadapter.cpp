#include "inpreciceadapter.h"

#include <precice/SolverInterface.hpp>

#include <cassert>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>

#include <json.hpp>
using json = nlohmann::json;

InpreciceAdapter::InpreciceAdapter(const size_t solverProcessRank,
                                        const size_t solverNumberOfProcesses) :
  InpreciceAdapter( "Visus", solverProcessRank, solverNumberOfProcesses )
{

}

InpreciceAdapter::InpreciceAdapter(const std::string& solverName,
                                        const size_t solverProcessRank,
                                        const size_t solverNumberOfProcesses ) :
  interface_( std::make_unique<precice::SolverInterface>(solverName, solverProcessRank, solverNumberOfProcesses) ),
  meshIsCreated_(false),
  preciceIsInitialized_(false),
  meshID_(0),
  pressureID_(0),
  concentrationID_(0),
  timeStepSize_(0.)
{

}

void InpreciceAdapter::configure(const std::string &configurationFileName)
{
  interface_->configure( configurationFileName );
}

void InpreciceAdapter::setMeshName(const std::string &meshName)
{
  meshID_ = interface_->getMeshID( meshName );
}

void InpreciceAdapter::setVisualizationMesh( const std::string& meshFilePath )
{
  std::array<size_t, 2> gridDimension({0, 0});

  std::ifstream fs;
  fs.open(meshFilePath.c_str(), std::ofstream::in);
  json conf;
  fs >> conf;

  gridDimension_ = conf["gridDimensions"].get<gridDimension_t>();
  const size_t numPoints = gridDimension_[0] * gridDimension_[1];
  vertexIDs_.resize(numPoints);
  boost::multi_array<double, 2> gridPoints(boost::extents[numPoints][3]);
  for (size_t idx = 0; idx < numPoints; ++idx)
  {
    gridPoints[idx][0] =
        conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[0];
    gridPoints[idx][1] =
        conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[1];
    gridPoints[idx][2] =
        conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[2];
  }
  fs.close();

  interface_->setMeshVertices(
        meshID_, numPoints, gridPoints.data(), vertexIDs_.data());

  meshIsCreated_ = true;
}

void InpreciceAdapter::initialize()
{
  assert( meshIsCreated_ );
  pressureID_ = interface_->getDataID( "Pressure", meshID_ );
  concentrationID_ = interface_->getDataID( "Concentration", meshID_ );

  pressure_.resize( boost::extents[gridDimension_[0]][gridDimension_[1]] );
  concentration_.resize( boost::extents[gridDimension_[0]][gridDimension_[1]] );

  timeStepSize_ = interface_->initialize();
  assert( timeStepSize_ > 0. );

  interface_->initializeData();
  preciceIsInitialized_ = true;
}

void InpreciceAdapter::runCouplingThreaded()
{
  assert( preciceIsInitialized_ );
  preciceThread_ = std::thread( &InpreciceAdapter::runCoupling, this, std::ref(pressure_), std::ref(concentration_), false);
}

void InpreciceAdapter::runCoupling( boost::multi_array<double, 2>& pressure,
                                    boost::multi_array<double, 2>& concentration,
                                    bool printData)
{
  if ( interface_->isReadDataAvailable() )
  {
    std::lock_guard<std::mutex> guard( dataMutex_ );
    interface_->readBlockScalarData( pressureID_,
                                     vertexIDs_.size(),
                                     vertexIDs_.data(),
                                     pressure.data());
  }

  // Pressure is only read once
  {
    std::lock_guard<std::mutex> guard(dataMutex_);
    interface_->readBlockScalarData( pressureID_,
                                    vertexIDs_.size(),
                                    vertexIDs_.data(),
                                    pressure.data());
  }

  do
  {
    {
      std::lock_guard<std::mutex> guard(dataMutex_);

      interface_->readBlockScalarData( concentrationID_,
                                       vertexIDs_.size(),
                                       vertexIDs_.data(),
                                       concentration.data());
    }

    if (printData)
      for (size_t y=0; y < concentration_.shape()[1]; ++y)
      {
        for (size_t x=0; x < concentration_.shape()[0]; ++x)
          std::printf("%.3f ", concentration_[y][x]);
        std::cout << std::endl;
      }

    const double preciceDt = interface_->advance(timeStepSize_);
    timeStepSize_ = std::max( timeStepSize_, preciceDt );
  } while(interface_->isCouplingOngoing());
}

const boost::multi_array<double, 2>& InpreciceAdapter::getPressureVector()
{
//  std::lock_guard<std::mutex> guard( dataMutex_ );
  return pressure_;
}

const boost::multi_array<double, 2>& InpreciceAdapter::getConcentrationVector()
{
//  std::lock_guard<std::mutex> guard( dataMutex_ );
  return concentration_;
}

void InpreciceAdapter::finalize()
{
  preciceThread_.join();
  interface_->finalize();
}

InpreciceAdapter::~InpreciceAdapter()
{

}


