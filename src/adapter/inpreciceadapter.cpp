#include "inpreciceadapter.h"

#include <precice/SolverInterface.hpp>

#include <cassert>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>

#include <json.hpp>
using json = nlohmann::json;



//std::ostream& inpreciceadapter::operator<<( std::ostream& os, const VisualizationDataInfo& info )
//{
//  os << "  Mesh name: "  << info.meshName << "\n"
//     << "  Data Name: " << info.dataName << std::endl;
//  return os;
//}


std::ostream& inpreciceadapter::operator<<( std::ostream& os, const VisualizationDataInfo& info )
{
  os << "  Mesh id: "   << info.meshID << "\n"
     << "  Mesh name: "  << info.meshName << "\n"
     << "  Mesh dim: " << info.gridDimension[0] << " x " << info.gridDimension[1] << "\n"
     << "  #Vertices: " << info.vertexIDs.size() << "\n"
//     << "  Buffer size: " << info.buffers.size() << "\n"
     << std::endl;

  os << "Data names:" << std::endl;
  assert( info.dataIDs.size() == info.dataNames.size() );
  for (size_t i = 0; i < info.dataIDs.size(); ++i) {
    os << info.dataNames[i] << "(ID: " << info.dataIDs[i] << ")" << "\n";
  }
  os << std::endl;

  return os;
}

using namespace inpreciceadapter;

//InpreciceAdapter::InpreciceAdapter(const size_t solverProcessRank,
//                                        const size_t solverNumberOfProcesses) :
//  InpreciceAdapter( "Visus", solverProcessRank, solverNumberOfProcesses )
//{

//}

InpreciceAdapter::InpreciceAdapter(const std::string& solverName,
                                   const std::string& configurationFileName,
                                   const size_t solverProcessRank,
                                   const size_t solverNumberOfProcesses ) :
                                                                           interface_( std::make_unique<precice::SolverInterface>(solverName, solverProcessRank, solverNumberOfProcesses) ),
                                                                           //  meshIsCreated_(false),
                                                                           preciceIsInitialized_(false),
                                                                           //  meshID_(0),
                                                                           //  pressureID_(0),
                                                                           //  concentrationID_(0),
                                                                           timeStepSize_(0.)
{
  interface_->configure( configurationFileName );
}

//void InpreciceAdapter::configure(const std::string &configurationFileName)
//{
//  interface_->configure( configurationFileName );
//}

//void InpreciceAdapter::setMeshName(const std::string &meshName)
//{
//  meshID_ = interface_->getMeshID( meshName );
//}

//void InpreciceAdapter::setVisualizationMesh( const std::string& meshFilePath )
//{
//  std::array<size_t, 2> gridDimension({0, 0});

//  std::ifstream fs;
//  fs.open(meshFilePath.c_str(), std::ofstream::in);
//  json conf;
//  fs >> conf;

//  gridDimension_ = conf["gridDimensions"].get<gridDimension_t>();
//  const size_t numPoints = gridDimension_[0] * gridDimension_[1];
//  vertexIDs_.resize(numPoints);
//  boost::multi_array<double, 2> gridPoints(boost::extents[numPoints][3]);
//  for (size_t idx = 0; idx < numPoints; ++idx)
//  {
//    gridPoints[idx][0] =
//        conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[0];
//    gridPoints[idx][1] =
//        conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[1];
//    gridPoints[idx][2] =
//        conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[2];
//  }
//  fs.close();

//  interface_->setMeshVertices(
//        meshID_, numPoints, gridPoints.data(), vertexIDs_.data());

//  meshIsCreated_ = true;
//}


//void InpreciceAdapter::setVisualizationMeshes( const std::string& meshFilePath,
//                                              VisualizationDataInfoVec_t& visInfoVec )
//{
//  std::ifstream fs;
//  fs.open(meshFilePath.c_str(), std::ofstream::in);
//  json conf;
//  fs >> conf;

//  for (auto& visInfo: visInfoVec )
//  {
//    const std::string& meshName = visInfo.meshName;
//    visInfo.meshId = interface_->getMeshID( meshName );
//    visInfo.gridDimension = conf[meshName]["gridDimensions"].get<gridDimension_t>();

//    const std::size_t numPoints = gridDimension_[0] * gridDimension_[1];
//    vertexIDs_.resize(numPoints);

//    boost::multi_array<double, 2> gridPoints(boost::extents[numPoints][3]);
//    for (size_t idx = 0; idx < numPoints; ++idx)
//    {
//      gridPoints[idx][0] =
//          conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[0];
//      gridPoints[idx][1] =
//          conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[1];
//      gridPoints[idx][2] =
//          conf["vertices"][idx]["pos"].get<std::array<float, 3>>()[2];
//    }

//    interface_->setMeshVertices(
//        visInfo.meshId, numPoints, gridPoints.data(), vertexIDs_.data());

//  }

//  fs.close();

//  meshIsCreated_ = true;
//}

//void InpreciceAdapter::initialize()
//{
//  assert( meshIsCreated_ );
//  pressureID_ = interface_->getDataID( "Pressure", meshID_ );
////  concentrationID_ = interface_->getDataID( "Concentration", meshID_ );

//  pressure_.resize( boost::extents[gridDimension_[0]][gridDimension_[1]] );
//  concentration_.resize( boost::extents[gridDimension_[0]][gridDimension_[1]] );

//  timeStepSize_ = interface_->initialize();
//  assert( timeStepSize_ > 0. );

//  interface_->initializeData();
//  preciceIsInitialized_ = true;
//}

//void InpreciceAdapter::initialize(const std::string& meshFilePath,
//                                  const VisualizationDataInfoVec_t& visInfoVec )
//{
//  std::ifstream fs;
//  fs.open(meshFilePath.c_str(), std::ofstream::in);
//  json conf;
//  fs >> conf;

//  {
//    visInfoData_.resize( visInfoVec.size() );
//    size_t i = 0;
//    for (const auto& visInfo: visInfoVec )
//    {
//      visInfoData_[i].meshName = visInfo.meshName;
//      visInfoData_[i].dataName = visInfo.dataName;
//    }
//  }

//  for (auto& visInfo: visInfoData_ )
//  {
//    const std::string& meshName = visInfo.meshName;
//    visInfo.meshId = interface_->getMeshID( meshName );
//    auto& gridDimension = visInfo.gridDimension;
//    gridDimension = conf[meshName]["gridDimensions"].get<gridDimension_t>();

//    const std::size_t numPoints = visInfo.gridDimension[0] * visInfo.gridDimension[1];
//    visInfo.vertexIDs.resize(numPoints);
//    visInfo.buffer.resize( boost::extents[gridDimension[0]][gridDimension[1]] );

//    boost::multi_array<double, 2> gridPoints(boost::extents[numPoints][3]);
//    for (size_t idx = 0; idx < numPoints; ++idx)
//    {
//      gridPoints[idx][0] =
//          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[0];
//      gridPoints[idx][1] =
//          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[1];
//      gridPoints[idx][2] =
//          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[2];
//    }

//    interface_->setMeshVertices(
//        visInfo.meshId, (int)numPoints, gridPoints.data(), visInfo.vertexIDs.data());

//    assert( visInfo.buffer.size() == visInfo.vertexIDs.size() );
//    assert( visInfo.buffer.size() == numPoints );
//  }

//  fs.close();



//  timeStepSize_ = interface_->initialize();
//  assert( timeStepSize_ > 0. );

//  interface_->initializeData();
//  preciceIsInitialized_ = true;
//}


void InpreciceAdapter::initialize(const std::string& meshFilePath )
{
  std::ifstream fs;
  fs.open(meshFilePath.c_str(), std::ofstream::in);
  json conf;
  fs >> conf;

  std::cout << conf.size() << std::endl;

//  std::for_each( conf.begin(), conf.end(), [](const std::string& name) { std::cout << name << "\n"; }  );
//  for (std::size_t i = 0; i < conf.size(); ++i) {
//    std::cout << conf[i] << std::endl;
//  }
//  for (json::iterator it = conf.begin(); it != conf.end(); ++it) {
////    std::cout << *it << '\n';
//    std::cout << it->get<std::string>() << '\n';
//  }

//  std::cout << conf.object() << std::endl;
//  std::cout << conf.get<json::object_t>() << std::endl;

  {
    visInfoData_.resize( conf.get<json::object_t>().size() );
    size_t i = 0;
    for (const auto& obj : conf.get<json::object_t>()) {
      const std::string& meshName = obj.first;
      std::cout << meshName << std::endl;
      visInfoData_[i].meshName = meshName;
      visInfoData_[i].meshID = interface_->getMeshID( meshName );


      const size_t nDataFields = conf[meshName]["mappingData"].size();
      visInfoData_[i].buffers.resize( nDataFields );

      auto& dataNames = visInfoData_[i].dataNames;
      dataNames.reserve( nDataFields );

      auto& dataIDs = visInfoData_[i].dataIDs;
      dataIDs.reserve( nDataFields );

      for (const auto& dataName: conf[meshName]["mappingData"] ) {
        const std::string name = dataName;
        std::cout << name << std::endl;
        dataNames.push_back( name );
        dataIDs.push_back( interface_->getDataID( name, visInfoData_[i].meshID ) );
      }
      ++i;
    }
  }


  for (auto& visInfo: visInfoData_ )
  {
    const std::string& meshName = visInfo.meshName;
    auto& gridDimension = visInfo.gridDimension;
    gridDimension = conf[meshName]["gridDimensions"].get<gridDimension_t>();

    const std::size_t numPoints = visInfo.gridDimension[0] * visInfo.gridDimension[1];
    visInfo.vertexIDs.resize(numPoints);

//    visInfo.buffer.resize( boost::extents[gridDimension[0]][gridDimension[1]] );
    std::for_each( visInfo.buffers.begin(), visInfo.buffers.end(), [gridDimension](auto& buffer)
                  {
                    buffer.resize( boost::extents[gridDimension[0]][gridDimension[1]] );
                  }
                  );

    boost::multi_array<double, 2> gridPoints(boost::extents[numPoints][3]);
    for (size_t idx = 0; idx < numPoints; ++idx)
    {
      gridPoints[idx][0] =
          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[0];
      gridPoints[idx][1] =
          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[1];
      gridPoints[idx][2] =
          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[2];
    }

    interface_->setMeshVertices(
        visInfo.meshID, (int)numPoints, gridPoints.data(), visInfo.vertexIDs.data());

    assert( visInfo.vertexIDs.size() == numPoints );
  }


//  std::cout << conf.begin()->get<std::string>() << std::endl;

//  std::for_each( conf.get<json::object_t>().begin(), conf.get<json::object_t>().end(), [](const auto& it) { std::cout << it.first << "\n"; }  );
//  std::for_each( conf.get<json::object_t>().begin(), conf.get<json::object_t>().end(), [](const auto& it) { std::cout << it << "\n"; }  );
//  std::for_each( conf.get<json::object_t>().begin(), conf.get<json::object_t>().end(), [](const auto& it) { std::cout << it.first << "\n"; }  );


//  {
//    visInfoData_.resize( visInfoVec.size() );
//    size_t i = 0;
//    for (const auto& visInfo: visInfoVec )
//    {
//      visInfoData_[i].meshName = visInfo.meshName;
//      visInfoData_[i].dataName = visInfo.dataName;
//    }
//  }

//  for (auto& visInfo: visInfoData_ )
//  {
//    const std::string& meshName = visInfo.meshName;
//    visInfo.meshId = interface_->getMeshID( meshName );
//    auto& gridDimension = visInfo.gridDimension;
//    gridDimension = conf[meshName]["gridDimensions"].get<gridDimension_t>();

//    const std::size_t numPoints = visInfo.gridDimension[0] * visInfo.gridDimension[1];
//    visInfo.vertexIDs.resize(numPoints);
//    visInfo.buffer.resize( boost::extents[gridDimension[0]][gridDimension[1]] );

//    boost::multi_array<double, 2> gridPoints(boost::extents[numPoints][3]);
//    for (size_t idx = 0; idx < numPoints; ++idx)
//    {
//      gridPoints[idx][0] =
//          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[0];
//      gridPoints[idx][1] =
//          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[1];
//      gridPoints[idx][2] =
//          conf[meshName]["vertices"][idx]["pos"].get<std::array<float, 3>>()[2];
//    }

//    interface_->setMeshVertices(
//        visInfo.meshId, (int)numPoints, gridPoints.data(), visInfo.vertexIDs.data());

//    assert( visInfo.buffer.size() == visInfo.vertexIDs.size() );
//    assert( visInfo.buffer.size() == numPoints );
//  }

  fs.close();

  timeStepSize_ = interface_->initialize();
  assert( timeStepSize_ > 0. );

  interface_->initializeData();
  preciceIsInitialized_ = true;
}

//void InpreciceAdapter::setConcentrationDataIDs(std::size_t n)
//{
////  data
//}

void InpreciceAdapter::runCouplingThreaded()
{
  assert( preciceIsInitialized_ );
//  preciceThread_ = std::thread( &InpreciceAdapter::runCoupling, this, std::ref(pressure_), std::ref(concentration_), false);
  preciceThread_ = std::thread( &InpreciceAdapter::runCoupling, this, std::ref(visInfoData_), false);
}

void InpreciceAdapter::runCoupling(VisualizationDataInfoVec_t& visInfoDataVec_,
                                   bool printData)
{
  //  if ( interface_->isReadDataAvailable() )
  //  {
  //    std::lock_guard<std::mutex> guard( dataMutex_ );
  //    interface_->readBlockScalarData( pressureID_,
  //                                     vertexIDs_.size(),
  //                                     vertexIDs_.data(),
  //                                     pressure.data());
  //  }

  //  // Pressure is only read once
  //  {
  //    std::lock_guard<std::mutex> guard(dataMutex_);
  //    interface_->readBlockScalarData( pressureID_,
  //                                    vertexIDs_.size(),
  //                                    vertexIDs_.data(),
  //                                    pressure.data());
  //  }
//  if ( interface_->isReadDataAvailable() )
//  {
//    for (auto& visInfo: visInfoData_ )
//    {
//      std::lock_guard<std::mutex> guard( dataMutex_ );
//      interface_->readBlockScalarData(visInfo.dataId,
//                                      (int)visInfo.vertexIDs.size(),
//                                      visInfo.vertexIDs.data(),
//                                      visInfo.buffer.data());
//    }
//  }


  while(interface_->isCouplingOngoing())
  {
    for (auto& visInfo: visInfoDataVec_ )
    {
      for (size_t i = 0; i < visInfo.buffers.size(); ++i)
      {
        std::lock_guard<std::mutex> guard( dataMutex_ );
        interface_->readBlockScalarData(visInfo.dataIDs[i],
                                        static_cast<int>(visInfo.vertexIDs.size()),
                                        visInfo.vertexIDs.data(),
                                        visInfo.buffers[i].data());
      }
    }

//    if (printData)
//      if ( interface_->isReadDataAvailable() )
//      {
//        for (auto& visInfo: visInfoData_ )
//        {
//          std::lock_guard<std::mutex> guard( dataMutex_ );
//          interface_->readBlockScalarData(visInfo.dataId,
//                                          (int)visInfo.vertexIDs.size(),
//                                          visInfo.vertexIDs.data(),
//                                          visInfo.buffer.data());
//          for (size_t y=0; y < visInfo.buffer.shape()[1]; ++y)
//          {
//            for (size_t x=0; x < visInfo.buffer.shape()[0]; ++x)
//              std::printf("%.3f ", visInfo.buffer[y][x]);
//            std::cout << std::endl;
//          }
//        }
//      }
//      for (size_t y=0; y < concentration_.shape()[1]; ++y)
//      {
//        for (size_t x=0; x < concentration_.shape()[0]; ++x)
//          std::printf("%.3f ", concentration_[y][x]);
//        std::cout << std::endl;
//      }

    const double preciceDt = interface_->advance(timeStepSize_);
    timeStepSize_ = std::max( timeStepSize_, preciceDt );
  }
}


//const boost::multi_array<double, 2>& InpreciceAdapter::getPressureVector()
//{
//  //  std::lock_guard<std::mutex> guard( dataMutex_ );
//  return pressure_;
//}

//const boost::multi_array<double, 2>& InpreciceAdapter::getConcentrationVector()
//{
//  //  std::lock_guard<std::mutex> guard( dataMutex_ );
//  return concentration_;
//}

const VisualizationDataInfoVec_t& InpreciceAdapter::getVisualisationData()
{
  std::lock_guard<std::mutex> guard( dataMutex_ );
  return visInfoData_;
}

void InpreciceAdapter::finalize()
{
  preciceThread_.join();
  interface_->finalize();
}

InpreciceAdapter::~InpreciceAdapter()
{

}


