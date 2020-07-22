#include "inpreciceadapter.hpp"

#include <precice/SolverInterface.hpp>

#include <cassert>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>

#include <json.hpp>
using json = nlohmann::json;

std::ostream& inpreciceadapter::operator<<(
        std::ostream& os,
        const VisualizationDataInfo& info )
{
    os << "  Mesh id: "   << info.meshID << "\n"
        << "  Mesh name: "  << info.meshName << "\n"
        << "  Mesh dim: " << info.gridDimension[0] << " x "
            << info.gridDimension[1] << "\n"
        << "  #Vertices: " << info.vertexIDs.size() << "\n"
        << std::endl;

    os << "Data names:" << std::endl;

    assert(info.dataIDs.size() == info.dataNames.size());
    for (size_t i = 0; i < info.dataIDs.size(); ++i)
        os << info.dataNames[i] << "(ID: " << info.dataIDs[i] << ")" << "\n";

    os << std::endl;

    return os;
}

using namespace inpreciceadapter;

InpreciceAdapter::InpreciceAdapter(
    const std::string& solverName,
    const std::string& configurationFileName,
    const int bmCase,
    const size_t solverProcessRank,
    const size_t solverNumberOfProcesses ) :
        interface_(
            std::make_unique<precice::SolverInterface>(
                solverName, solverProcessRank, solverNumberOfProcesses) ),
        bmCase_(bmCase),
        preciceIsInitialized_(false),
        timeStepSize_(0.)
{
    interface_->configure(configurationFileName);
}

void InpreciceAdapter::initialize(const std::string& meshFilePath )
{
    json conf;
    {
        std::ifstream fs;
        fs.open(meshFilePath.c_str(), std::ofstream::in);
        fs >> conf;
        fs.close();
    }

    if (1 == bmCase_)
    {
        conf = conf["case1"];
        visInfoData_.resize(1);
        VisualizationDataInfo &visInfo = visInfoData_[0];

        const std::string& meshName = "VisualizationMesh";
        visInfo.meshName = meshName;
        visInfo.meshID = interface_->getMeshID( meshName );

        const size_t nDataFields = conf["mappingData"].size();
        visInfo.buffers.resize(nDataFields);

        auto& dataNames = visInfo.dataNames;
        dataNames.reserve(nDataFields);

        auto& dataIDs = visInfo.dataIDs;
        dataIDs.reserve(nDataFields);

        for (const auto& dataName: conf["mappingData"] )
        {
            const std::string name = dataName;
            dataNames.push_back(name);
            dataIDs.push_back(
                interface_->getDataID(name, visInfo.meshID));
        }

        auto& gridDimension = visInfo.gridDimension;
        gridDimension = conf["gridDimensions"].get<gridDimension_t>();

        const std::size_t numPoints =
                visInfo.gridDimension[0] * visInfo.gridDimension[1];
        visInfo.vertexIDs.resize(numPoints);

        std::for_each(
            visInfo.buffers.begin(),
            visInfo.buffers.end(),
            [gridDimension](auto& buffer){
                buffer.resize(
                    boost::extents[gridDimension[0]][gridDimension[1]] );
                      }
            );

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

        interface_->setMeshVertices(
            visInfo.meshID,
            static_cast<int>(numPoints),
            gridPoints.data(),
            visInfo.vertexIDs.data());

        assert(visInfo.vertexIDs.size() == numPoints);
    }
    else if (2 == bmCase_)
    {
        conf = conf["case2"];
        {
            visInfoData_.resize( conf.get<json::object_t>().size() );
            size_t i = 0;
            for (const auto& obj : conf.get<json::object_t>())
            {
                const std::string& meshName = obj.first;
                visInfoData_[i].meshName = meshName;
                visInfoData_[i].meshID = interface_->getMeshID( meshName );


                const size_t nDataFields = conf[meshName]["mappingData"].size();
                visInfoData_[i].buffers.resize( nDataFields );

                auto& dataNames = visInfoData_[i].dataNames;
                dataNames.reserve( nDataFields );

                auto& dataIDs = visInfoData_[i].dataIDs;
                dataIDs.reserve( nDataFields );

                for (const auto& dataName: conf[meshName]["mappingData"] )
                {
                    const std::string name = dataName;
                    dataNames.push_back(name);
                    dataIDs.push_back(
                        interface_->getDataID(name, visInfoData_[i].meshID ));
                }
                ++i;
            }
        }

        for (auto& visInfo: visInfoData_ )
        {
            const std::string& meshName = visInfo.meshName;
            auto& gridDimension = visInfo.gridDimension;
            gridDimension = conf[meshName]["gridDimensions"].get<
                    gridDimension_t>();

            const std::size_t numPoints = visInfo.gridDimension[0] *
                    visInfo.gridDimension[1];
            visInfo.vertexIDs.resize(numPoints);

            std::for_each(
                visInfo.buffers.begin(),
                visInfo.buffers.end(),
                [gridDimension](auto& buffer){
                    buffer.resize(
                        boost::extents[gridDimension[0]][gridDimension[1]] );
                          }
                );

            boost::multi_array<double, 2> gridPoints(boost::extents[numPoints][3]);
            for (size_t idx = 0; idx < numPoints; ++idx)
            {
                gridPoints[idx][0] =
                    conf[meshName]["vertices"][idx]["pos"].get<
                        std::array<float, 3>>()[0];
                gridPoints[idx][1] =
                    conf[meshName]["vertices"][idx]["pos"].get<
                        std::array<float, 3>>()[1];
                gridPoints[idx][2] =
                    conf[meshName]["vertices"][idx]["pos"].get<
                        std::array<float, 3>>()[2];
            }

            interface_->setMeshVertices(
                visInfo.meshID,
                static_cast<int>(numPoints),
                gridPoints.data(),
                visInfo.vertexIDs.data());

            assert( visInfo.vertexIDs.size() == numPoints );
        }
    }
    else
    {
        // unimplemented benchmark case
        return;
    }

    timeStepSize_ = interface_->initialize();
    assert( timeStepSize_ > 0. );

    interface_->initializeData();
    preciceIsInitialized_ = true;
}

void InpreciceAdapter::runCouplingThreaded()
{
    assert( preciceIsInitialized_ );
    preciceThread_ = std::thread(
            &InpreciceAdapter::runCoupling,
            this,
            std::ref(visInfoData_));
}

void InpreciceAdapter::runCoupling(
    VisualizationDataInfoVec_t& visInfoDataVec_)
{
    while(interface_->isCouplingOngoing())
    {
        for (auto& visInfo: visInfoDataVec_ )
        {
            for (size_t i = 0; i < visInfo.buffers.size(); ++i)
            {
                std::lock_guard<std::mutex> guard( dataMutex_ );
                interface_->readBlockScalarData(
                    visInfo.dataIDs[i],
                    static_cast<int>(visInfo.vertexIDs.size()),
                    visInfo.vertexIDs.data(),
                    visInfo.buffers[i].data());
            }
        }

        const double preciceDt = interface_->advance(timeStepSize_);
        timeStepSize_ = std::max( timeStepSize_, preciceDt );
    }
}

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


