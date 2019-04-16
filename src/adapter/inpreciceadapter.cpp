#include "inpreciceadapter.h"

#include <precice/SolverInterface.hpp>

#include <cassert>
#include <iostream>
#include <thread>

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

void InpreciceAdapter::setVisualizationMesh( const gridDimension_t& gridDimension )
{
  gridDimension_ = gridDimension;
  const size_t numPoints = gridDimension[0] * gridDimension[1];
  vertexIDs_.resize(numPoints);

  boost::multi_array<double, 2> gridPoints(
        boost::extents[numPoints][3]);

  std::array<double, 2> cellDim =
  {   100.0 / static_cast<double>(gridDimension[0]),
      100.0 / static_cast<double>(gridDimension[1])};

  for (size_t y = 0; y < gridDimension[1]; ++y)
    for (size_t x = 0; x < gridDimension[0]; ++x)
    {
      size_t idx = y * gridDimension[0] + x;
      gridPoints[idx][0] = x * cellDim[0] + 0.5 * cellDim[0];
      gridPoints[idx][1] = y * cellDim[1] + 0.5 * cellDim[1];
      gridPoints[idx][2] = -0.6 * gridPoints[idx][0] + 80.0;
    }

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
  preciceThread_ = std::thread( &InpreciceAdapter::runCoupling, this, std::ref(pressure_), std::ref(concentration_) );
}

void InpreciceAdapter::runCoupling( boost::multi_array<double, 2>& pressure,
                                    boost::multi_array<double, 2>& concentration )
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


