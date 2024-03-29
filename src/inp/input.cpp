////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2019 Prashant K. Jha
//  Copyright (c) 2019 Patrick Diehl
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include "input.h"

#include <yaml-cpp/yaml.h>

#include <cmath>
#include <hpx/config.hpp>
#include <iostream>

#include "decks/fractureDeck.h"
#include "decks/initialConditionDeck.h"
#include "decks/interiorFlagsDeck.h"
#include "decks/loadingDeck.h"
#include "decks/massMatrixDeck.h"
#include "decks/materialDeck.h"
#include "decks/modelDeck.h"
#include "decks/neighborDeck.h"
#include "decks/outputDeck.h"
#include "decks/policyDeck.h"
#include "decks/quadratureDeck.h"
#include "decks/restartDeck.h"
#include "decks/solverDeck.h"
#include "inp/decks/absborbingCondDeck.h"
#include "inp/decks/meshDeck.h"
#include "util/utilIO.h"

static inline bool definitelyGreaterThan(const double &a, const double &b) {
  return (a - b) >
         ((std::abs(a) < std::abs(b) ? std::abs(b) : std::abs(a)) * 1.0E-5);
}

inp::Input::Input(const std::string &filename)
    : d_fractureDeck_p(nullptr),
      d_meshDeck_p(nullptr),
      d_initialConditionDeck_p(nullptr),
      d_interiorFlagsDeck_p(nullptr),
      d_loadingDeck_p(nullptr),
      d_materialDeck_p(nullptr),
      d_neighborDeck_p(nullptr),
      d_outputDeck_p(nullptr),
      d_policyDeck_p(nullptr),
      d_modelDeck_p(nullptr),
      d_solverDeck_p(nullptr) {
  d_inputFilename = filename;

  // follow the order of reading
  setModelDeck();
  setRestartDeck();
  setMeshDeck();
  setMassMatrixDeck();
  setQuadratureDeck();
  setNeighborDeck();
  setFractureDeck();
  setInteriorFlagsDeck();
  setInitialConditionDeck();
  setLoadingDeck();
  setMaterialDeck();
  setOutputDeck();
  setPolicyDeck();
  setSolverDeck();
  setAbsorbingCondDeck();
}

//
// accessor methods
//
inp::FractureDeck *inp::Input::getFractureDeck() { return d_fractureDeck_p; }

inp::InitialConditionDeck *inp::Input::getInitialConditionDeck() {
  return d_initialConditionDeck_p;
}

inp::InteriorFlagsDeck *inp::Input::getInteriorFlagsDeck() {
  return d_interiorFlagsDeck_p;
}

inp::LoadingDeck *inp::Input::getLoadingDeck() { return d_loadingDeck_p; }

inp::MassMatrixDeck *inp::Input::getMassMatrixDeck() {
  return d_massMatrixDeck_p;
}

inp::MaterialDeck *inp::Input::getMaterialDeck() { return d_materialDeck_p; }

inp::MeshDeck *inp::Input::getMeshDeck() { return d_meshDeck_p; }

inp::ModelDeck *inp::Input::getModelDeck() { return d_modelDeck_p; }

inp::NeighborDeck *inp::Input::getNeighborDeck() { return d_neighborDeck_p; }

inp::OutputDeck *inp::Input::getOutputDeck() { return d_outputDeck_p; }

inp::PolicyDeck *inp::Input::getPolicyDeck() { return d_policyDeck_p; }

inp::QuadratureDeck *inp::Input::getQuadratureDeck() {
  return d_quadratureDeck_p;
}

inp::RestartDeck *inp::Input::getRestartDeck() { return d_restartDeck_p; }

inp::SolverDeck *inp::Input::getSolverDeck() { return d_solverDeck_p; }

inp::AbsorbingCondDeck *inp::Input::getAbsorbingCondDeck() {
  return d_absorbingCondDeck_p;
}

const std::string inp::Input::getSpatialDiscretization() {
  return d_modelDeck_p->d_spatialDiscretization;
}

//
// setter methods
//
void inp::Input::setModelDeck() {
  d_modelDeck_p = new inp::ModelDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  // read dimension
  if (config["Model"]["Dimension"])
    d_modelDeck_p->d_dim = config["Model"]["Dimension"].as<size_t>();
  else {
    std::cerr << "Error: Please specify the dimension.\n";
    exit(1);
  }

  if (config["Model"]["Relaxation_Steps"])
    d_modelDeck_p->d_RelaxN = config["Model"]["Relaxation_Steps"].as<size_t>();

  // read discretization info
  if (config["Model"]["Discretization_Type"]["Time"])
    d_modelDeck_p->d_timeDiscretization =
        config["Model"]["Discretization_Type"]["Time"].as<std::string>();

  if (config["Model"]["Discretization_Type"]["Spatial"])
    d_modelDeck_p->d_spatialDiscretization =
        config["Model"]["Discretization_Type"]["Spatial"].as<std::string>();

  if (d_modelDeck_p->d_timeDiscretization == "central_difference" or
      d_modelDeck_p->d_timeDiscretization == "velocity_verlet")
    d_modelDeck_p->d_simType = "explicit";

  // read horizon and horizon to mesh ratio (if available)
  if (config["Model"]["Horizon"])
    d_modelDeck_p->d_horizon = config["Model"]["Horizon"].as<double>();
  if (config["Model"]["Horizon_h_Ratio"])
    d_modelDeck_p->d_rh = config["Model"]["Horizon_h_Ratio"].as<int>();
  if (config["Model"]["Mesh_Size"])
    d_modelDeck_p->d_h = config["Model"]["Mesh_Size"].as<double>();

  if (!config["Model"]["Horizon"]) {
    if (!config["Model"]["Horizon_h_Ratio"] or !config["Model"]["Mesh_Size"]) {
      std::cerr << "Error: Horizon is not provided. In this case "
                   "Horizon_h_Ratio and Mesh_Size are necessary to compute "
                   "horizon.\n";
      exit(1);
    }

    d_modelDeck_p->d_horizon = d_modelDeck_p->d_h * double(d_modelDeck_p->d_rh);
  }

  if (!config["Model"]["Mesh_Size"])
    if (config["Model"]["Horizon_h_Ratio"])
      d_modelDeck_p->d_h =
          d_modelDeck_p->d_horizon / double(d_modelDeck_p->d_rh);

  // read final time and time step
  if (config["Model"]["Final_Time"])
    d_modelDeck_p->d_tFinal = config["Model"]["Final_Time"].as<double>();
  if (config["Model"]["Time_Steps"])
    d_modelDeck_p->d_Nt = config["Model"]["Time_Steps"].as<int>();

  if (std::abs(d_modelDeck_p->d_tFinal) < 1.0E-10 or d_modelDeck_p->d_Nt <= 0) {
    std::cerr << "Error: Check Final_Time and Time_Steps data.\n";
    exit(1);
  }

  d_modelDeck_p->d_dt = d_modelDeck_p->d_tFinal / d_modelDeck_p->d_Nt;

  // check if this is restart problem
  if (config["Restart"]) d_modelDeck_p->d_isRestartActive = true;
}  // setModelDeck

void inp::Input::setRestartDeck() {
  d_restartDeck_p = new inp::RestartDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  // read restart file
  if (config["Restart"]) {
    if (config["Restart"]["File"])
      d_restartDeck_p->d_file = config["Restart"]["File"].as<std::string>();
    else {
      std::cerr << "Error: Please specify the file for restart.\n";
      exit(1);
    }

    // read time step from which to begin
    if (config["Restart"]["Step"])
      d_restartDeck_p->d_step = config["Restart"]["Step"].as<size_t>();
    else {
      std::cerr << "Error: Please specify the time step from which to restart "
                   "the simulation.\n";
      exit(1);
    }
  }
}  // setRestartDeck

void inp::Input::setMeshDeck() {
  d_meshDeck_p = new inp::MeshDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  // read dimension
  if (config["Model"]["Dimension"])
    d_meshDeck_p->d_dim = config["Model"]["Dimension"].as<size_t>();

  // read spatial discretization type
  if (config["Model"]["Discretization_Type"]["Spatial"])
    d_meshDeck_p->d_spatialDiscretization =
        config["Model"]["Discretization_Type"]["Spatial"].as<std::string>();

  // read mesh filename
  if (config["Mesh"]["File"])
    d_meshDeck_p->d_filename = config["Mesh"]["File"].as<std::string>();
  else {
    std::cerr << "Error: Please specify mesh filename.\n";
    exit(1);
  }
  if (config["Mesh"]["Gmsh_File_Version"]) {
    d_meshDeck_p->d_gmsh_msh_version =
        config["Mesh"]["Gmsh_File_Version"].as<double>();
  }
  if (config["Mesh"]["Load_PUM_data"])
    d_meshDeck_p->d_loadPUMData =
        config["Mesh"]["Load_PUM_data"].as<std::string>();
  else
    d_meshDeck_p->d_loadPUMData = "None";

  if (d_modelDeck_p->d_h < 1.0E-12)
    d_meshDeck_p->d_computeMeshSize = true;
  else
    d_meshDeck_p->d_h = d_modelDeck_p->d_h;

  // check if we should consider centroid based discretization
  if (config["Mesh"]["Is_Centroid_Based_Discretization"])
    d_meshDeck_p->d_isCentroidBasedDiscretization =
        config["Mesh"]["Is_Centroid_Based_Discretization"].as<bool>();

  if (config["Mesh"]["Keep_Element_Conn"])
    d_meshDeck_p->d_keepElementConn =
        config["Mesh"]["Keep_Element_Conn"].as<bool>();

}  // setMeshDeck

void inp::Input::setMassMatrixDeck() {
  d_massMatrixDeck_p = new inp::MassMatrixDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  // read fem related parameters
  if (config["Mesh"]["FEM_Info"]["M_Matrix_Approx"])
    d_massMatrixDeck_p->d_MApproxType =
        config["Mesh"]["FEM_Info"]["M_Matrix_Approx"].as<std::string>();
}

void inp::Input::setQuadratureDeck() {
  d_quadratureDeck_p = new inp::QuadratureDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  if (config["Mesh"]["FEM_Info"]["Order_Quad_Int"])
    d_quadratureDeck_p->d_quadOrder =
        config["Mesh"]["FEM_Info"]["Order_Quad_Int"].as<size_t>();
  if (config["Mesh"]["FEM_Info"]["Order_Quad_Int_M_Matrix"])
    d_quadratureDeck_p->d_quadOrderM =
        config["Mesh"]["FEM_Info"]["Order_Quad_Int_M_Matrix"].as<size_t>();
}

void inp::Input::setNeighborDeck() {
  d_neighborDeck_p = new inp::NeighborDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  if (config["Neighbor"]["Safety_Factor"])
    d_neighborDeck_p->d_safetyFactor =
        config["Neighbor"]["Safety_Factor"].as<double>();

  if (config["Neighbor"]["Add_Partial_Elems"])
    d_neighborDeck_p->d_addPartialElems =
        config["Neighbor"]["Add_Partial_Elems"].as<bool>();
}  // setNeighborDeck

void inp::Input::setFractureDeck() {
  d_fractureDeck_p = new inp::FractureDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  size_t ncracks = 0;
  if (config["Fracture"]["Cracks"]["Sets"])
    ncracks = config["Fracture"]["Cracks"]["Sets"].as<size_t>();

  if (ncracks == 0)
    return;
  else {
    if (d_modelDeck_p->d_dim != 2) {
      std::cerr << "Error: Pre-cracks can only be prescribed for material in "
                   "dimension 2.\n";
      exit(1);
    }
  }

  for (size_t s = 1; s <= ncracks; s++) {
    // prepare string Set_s to read file
    std::string read_set = "Set_";
    read_set.append(std::to_string(s));
    auto e = config["Fracture"]["Cracks"][read_set];

    inp::EdgeCrack crack;

    if (e["Orientation"]) crack.d_o = e["Orientation"].as<int>();

    if (e["Line"]) {
      std::vector<double> locs;
      for (auto j : e["Line"]) locs.push_back(j.as<double>());
      if (locs.size() != 4) {
        std::cerr << "Error: Input data in tag Line does not have enough "
                     "elements.\n";
        exit(1);
      }
      crack.d_pb = util::Point3(locs[0], locs[1], 0.);
      crack.d_pt = util::Point3(locs[2], locs[3], 0.);
      crack.d_l = crack.d_pb.dist(crack.d_pt);
      crack.d_oldPb = crack.d_pb;
      crack.d_initPb = crack.d_pb;
      crack.d_oldPt = crack.d_pt;
      crack.d_initPt = crack.d_pt;
    } else {
      std::cerr << "Error: Need Line data for complete specification of "
                   "pre-crack.\n";
      exit(1);
    }

    if (crack.d_o == 0) {
      if (e["Angle"])
        crack.d_theta = e["Angle"].as<double>();
      else {
        std::cerr << "Error: Need Angle data for crack with arbitrary "
                     "orientation.\n";
        exit(1);
      }
    }

    if (e["Activation_Time"])
      crack.d_activationTime = e["Activation_Time"].as<double>();

    // add crack data to list
    d_fractureDeck_p->d_cracks.push_back(crack);
  }
}  // setFractureDeck

void inp::Input::setInteriorFlagsDeck() {
  d_interiorFlagsDeck_p = new inp::InteriorFlagsDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  if (config["No_Fail_Region"]) {
    auto e = config["No_Fail_Region"];

    d_interiorFlagsDeck_p->d_noFailActive = true;
    d_interiorFlagsDeck_p->d_noFailTol =
        d_modelDeck_p->d_horizon *
        config["No_Fail_Region"]["Factor"].as<double>();

    if (config["No_Fail_Region"]["Compute_And_Not_Store"])
      d_interiorFlagsDeck_p->d_computeAndNotStoreFlag =
          config["No_Fail_Region"]["Compute_And_Not_Store"].as<bool>();

    size_t num_regions = 0;
    if (e["Num_Regions"]) num_regions = e["Num_Regions"].as<size_t>();

    d_interiorFlagsDeck_p->d_noFailRegions.resize(num_regions);
    for (size_t i = 1; i <= num_regions; i++) {
      // prepare string Set_s to read file
      std::string read_set = "Region_";
      read_set.append(std::to_string(i));
      auto ee = e[read_set];

      std::pair<std::string, std::vector<double>> data;

      // get region type
      if (ee["Type"])
        data.first = ee["Type"].as<std::string>();
      else {
        std::cerr << "Error: Region type must be provided for no-fail region\n";
        exit(1);
      }

      // get parameters
      for (auto f : ee["Parameters"]) data.second.push_back(f.as<double>());

      // add data
      d_interiorFlagsDeck_p->d_noFailRegions[i - 1] = data;
    }
    config["No_Fail_Region"]["Compute_And_Not_Store"].as<bool>();
  }
}  // setInteriorFlagsDeck

void inp::Input::setInitialConditionDeck() {
  d_initialConditionDeck_p = new inp::InitialConditionDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  if (config["Initial_Condition"]["Displacement"]) {
    auto e = config["Initial_Condition"]["Displacement"];

    if (e["File"]) {
      d_initialConditionDeck_p->d_uICData.d_file = e["File"].as<std::string>();
      d_initialConditionDeck_p->d_uICData.d_type = "file";
    }

    if (e["Type"])
      d_initialConditionDeck_p->d_uICData.d_type = e["Type"].as<std::string>();

    for (auto j : e["Parameters"])
      d_initialConditionDeck_p->d_uICData.d_params.push_back(j.as<double>());
  }

  if (config["Initial_Condition"]["Velocity"]) {
    auto e = config["Initial_Condition"]["Velocity"];

    if (e["File"]) {
      d_initialConditionDeck_p->d_vICData.d_file = e["File"].as<std::string>();
      d_initialConditionDeck_p->d_vICData.d_type = "file";
    }

    if (e["Type"])
      d_initialConditionDeck_p->d_vICData.d_type = e["Type"].as<std::string>();

    for (auto j : e["Parameters"])
      d_initialConditionDeck_p->d_vICData.d_params.push_back(j.as<double>());
  }
}  // setInitialConditionDeck

void inp::Input::setLoadingDeck() {
  d_loadingDeck_p = new inp::LoadingDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  // since displacement bc and force bc are of same nature, we combine the
  // reading of both
  std::vector<std::string> vtags = {"Displacement_BC", "Force_BC"};

  for (const auto &tag : vtags) {
    // read boundary condition data
    if (config[tag]) {
      int nsets = config[tag]["Sets"].as<int>();
      for (size_t s = 1; s <= nsets; s++) {
        // prepare string Set_s to read file
        std::string read_set = "Set_";
        read_set.append(std::to_string(s));
        auto e = config[tag][read_set];

        inp::BCData bc;

        // read region data
        if (e["Location"]["Line"]) {
          bc.d_regionType = "line";
          std::vector<double> locs;
          for (auto j : e["Location"]["Line"]) locs.push_back(j.as<double>());

          if (locs.size() != 2) {
            std::cerr << "Error: Check Line data in " << tag << ".\n";
            exit(1);
          }
          bc.d_x1 = locs[0];
          bc.d_x2 = locs[1];
        } else if (e["Location"]["Rectangle"]) {
          bc.d_regionType = "rectangle";
          std::vector<double> locs;
          for (auto j : e["Location"]["Rectangle"])
            locs.push_back(j.as<double>());

          if (locs.size() != 4) {
            std::cerr << "Error: Check Rectangle data in " << tag << ".\n";
            exit(1);
          }
          bc.d_x1 = locs[0];
          bc.d_x2 = locs[2];
          bc.d_y1 = locs[1];
          bc.d_y2 = locs[3];
        } else if (e["Location"]["Angled_Rectangle"]) {
          bc.d_regionType = "angled_rectangle";
          std::vector<double> locs;
          for (auto j : e["Location"]["Angled_Rectangle"])
            locs.push_back(j.as<double>());

          if (locs.size() != 4) {
            std::cerr << "Error: Check Rectangle data in " + tag + ".\n";
            exit(1);
          }
          bc.d_x1 = locs[0];
          bc.d_x2 = locs[2];
          bc.d_y1 = locs[1];
          bc.d_y2 = locs[3];

          bc.d_theta = e["Location"]["Angle"].as<double>();

          // perform check
          double alpha = std::atan((bc.d_y2 - bc.d_y1) / (bc.d_x2 - bc.d_x1));
          if (definitelyGreaterThan(0.0, alpha)) alpha = alpha + M_PI;

          if (definitelyGreaterThan(bc.d_theta, alpha) or
              std::abs(alpha - bc.d_theta) <= 1.0E-5) {
            std::cerr
                << "Error: Check angled rectangle. Either angle of diagonal is "
                   "smaller than theta or it is very close to theta\n";
            exit(1);
          }
        } else if (e["Location"]["Point"] and e["Location"]["Radius"]) {
          bc.d_regionType = "circle";

          std::vector<double> locs;
          for (auto j : e["Location"]["Point"]) locs.push_back(j.as<double>());

          if (locs.size() != 2) {
            std::cerr << "Error: Check Point data in " + tag + ".\n";
            exit(1);
          }

          bc.d_x1 = locs[0];
          bc.d_y1 = locs[1];

          std::vector<double> radii;
          for (auto j : e["Location"]["Radius"])
            radii.push_back(j.as<double>());

          if (radii.size() != 1) {
            std::cerr << "Error: Check Radii data in " + tag + ".\n";
            exit(1);
          }

          bc.d_r1 = radii[0];

        } else if (e["Location"]["Point"] and e["Location"]["Radii"]) {
          bc.d_regionType = "torus";

          std::vector<double> locs;
          for (auto j : e["Location"]["Point"]) locs.push_back(j.as<double>());

          if (locs.size() != 2) {
            std::cerr << "Error: Check Point data in " + tag + ".\n";
            exit(1);
          }

          bc.d_x1 = locs[0];
          bc.d_y1 = locs[1];

          std::vector<double> radii;
          for (auto j : e["Location"]["Radii"]) radii.push_back(j.as<double>());

          if (radii.size() != 2) {
            std::cerr << "Error: Check Radii data in " + tag + ".\n";
            exit(1);
          }

          bc.d_r1 = radii[0];
          bc.d_r2 = radii[1];
        }

        // read bc region
        else if (e["Location"]["Cuboid"]) {
          bc.d_regionType = "cuboid";
          std::vector<double> locs;
          for (auto j : e["Location"]["Cuboid"]) locs.push_back(j.as<double>());

          if (locs.size() != 6) {
            std::cerr << "Error: Check Cuboid data in " << tag << ".\n";
            exit(1);
          }
          bc.d_x1 = locs[0];
          bc.d_x2 = locs[3];
          bc.d_y1 = locs[1];
          bc.d_y2 = locs[4];
          bc.d_z1 = locs[2];
          bc.d_z2 = locs[5];
        }  // read bc region
        else if (e["Location"]["Displacement_from_pum"]) {
          bc.d_regionType = "displacement_from_pum";
        } else if (e["Location"]["Force_from_pum"]) {
          bc.d_regionType = "force_from_pum";
        }

        // read direction
        for (auto j : e["Direction"]) bc.d_direction.push_back(j.as<size_t>());

        // read time function type
        if (e["Time_Function"]) {
          bc.d_timeFnType = e["Time_Function"]["Type"].as<std::string>();
          if (e["Time_Function"]["Parameters"])
            for (auto j : e["Time_Function"]["Parameters"])
              bc.d_timeFnParams.push_back(j.as<double>());
        }

        if (e["Spatial_Function"]) {
          bc.d_spatialFnType = e["Spatial_Function"]["Type"].as<std::string>();
          if (e["Spatial_Function"]["Parameters"])
            for (auto j : e["Spatial_Function"]["Parameters"])
              bc.d_spatialFnParams.push_back(j.as<double>());
        }

        // push data
        if (tag == "Displacement_BC")
          d_loadingDeck_p->d_uBCData.push_back(bc);
        else
          d_loadingDeck_p->d_fBCData.push_back(bc);
      }  // loop sets
    }    // if tag exists
  }      // loop over tags
}  // setLoadingDeck

void inp::Input::setMaterialDeck() {
  d_materialDeck_p = new inp::MaterialDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  auto e = config["Material"];
  if (e["Is_Plane_Strain"])
    d_materialDeck_p->d_isPlaneStrain = e["Is_Plane_Strain"].as<bool>();
  if (e["Type"])
    d_materialDeck_p->d_materialType = e["Type"].as<std::string>();
  else {
    std::cerr << "Error: Please specify the material type.\n";
    exit(1);
  }

  if (d_materialDeck_p->d_materialType == "PDBond") {
    std::cout << "Warning: We now have new name for peridynamic material. "
                 "RNPBond for Robs nonlinear bond-based material. PMBBond for"
                 " pmb. PDElasticBond for linear elastic bond-based. PDState "
                 "for Silling state-based peridynamic model.\n";
    std::cout << "Warning: PDBond is changed to RNPBond.\n";

    d_materialDeck_p->d_materialType = "RNPBond";
  }

  if (e["Compute_From_Classical"])
    d_materialDeck_p->d_computeParamsFromElastic =
        e["Compute_From_Classical"].as<bool>();
  if (e["E"]) d_materialDeck_p->d_matData.d_E = e["E"].as<double>();
  if (e["G"]) d_materialDeck_p->d_matData.d_G = e["G"].as<double>();
  if (e["K"]) d_materialDeck_p->d_matData.d_K = e["K"].as<double>();
  if (e["Lambda"])
    d_materialDeck_p->d_matData.d_lambda = e["Lambda"].as<double>();
  if (e["Mu"]) d_materialDeck_p->d_matData.d_mu = e["Mu"].as<double>();
  if (e["Poisson_Ratio"])
    d_materialDeck_p->d_matData.d_nu = e["Poisson_Ratio"].as<double>();
  if (e["Gc"]) d_materialDeck_p->d_matData.d_Gc = e["Gc"].as<double>();
  if (e["KIc"]) d_materialDeck_p->d_matData.d_KIc = e["KIc"].as<double>();

  // read pairwise (bond) potential
  if (e["Bond_Potential"]) {
    auto f = e["Bond_Potential"];
    if (f["Type"])
      d_materialDeck_p->d_bondPotentialType = f["Type"].as<size_t>();
    if (f["Check_Sc_Factor"])
      d_materialDeck_p->d_checkScFactor = f["Check_Sc_Factor"].as<double>();
    if (f["Irreversible_Bond_Fracture"])
      d_materialDeck_p->d_irreversibleBondBreak =
          f["Irreversible_Bond_Fracture"].as<bool>();
    if (f["Parameters"])
      for (auto j : f["Parameters"])
        d_materialDeck_p->d_bondPotentialParams.push_back(j.as<double>());
  }

  // read hydrostatic (state) potential
  if (e["State_Potential"]) {
    auto f = e["State_Potential"];
    if (f["Type"])
      d_materialDeck_p->d_statePotentialType = f["Type"].as<size_t>();
    if (f["Contribution_From_Broken_Bond"])
      d_materialDeck_p->d_stateContributionFromBrokenBond =
          f["Contribution_From_Broken_Bond"].as<bool>();
    if (f["Parameters"])
      for (auto j : f["Parameters"])
        d_materialDeck_p->d_statePotentialParams.push_back(j.as<double>());
  }

  // read influence function
  if (e["Influence_Function"]) {
    auto f = e["Influence_Function"];
    if (f["Type"]) d_materialDeck_p->d_influenceFnType = f["Type"].as<size_t>();
    if (f["Parameters"])
      for (auto j : f["Parameters"])
        d_materialDeck_p->d_influenceFnParams.push_back(j.as<double>());
  }

  // read density
  if (e["Density"])
    d_materialDeck_p->d_density = e["Density"].as<double>();
  else {
    std::cerr << "Error: Please specify the density of the material.\n";
    exit(1);
  }

  // enable non-penetration condition, i.e. allow broken bonded nodes to have
  // normal contact force
  if (e["No_Penetration"])
    d_materialDeck_p->d_applyContact = e["No_Penetration"].as<bool>();

  if (e["Disserpation"]) {
    d_materialDeck_p->d_has_disserpation = e["Disserpation"].as<bool>();

    if (e["Disserpation_x"]) {
      d_materialDeck_p->d_vb_x = e["Disserpation_x"].as<double>();
    } else {
      std::cerr << "Error: Please provide the disserpation value "
                   "(Disserpation_x) if Diserpation is enabled"
                << std::endl;
    }

    if (e["Disserpation_y"]) {
      d_materialDeck_p->d_vb_y = e["Disserpation_y"].as<double>();
    } else {
      std::cerr << "Error: Please provide the disserpation value "
                   "(Disserpation_y) if Diserpation is enabled"
                << std::endl;
    }
  }

}  // setMaterialDeck

void inp::Input::setOutputDeck() {
  d_outputDeck_p = new inp::OutputDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  if (config["Output"]) {
    auto e = config["Output"];
    if (e["File_Format"])
      d_outputDeck_p->d_outFormat = e["File_Format"].as<std::string>();
    if (e["Path"]) d_outputDeck_p->d_path = e["Path"].as<std::string>();
    if (e["Tags"])
      for (auto f : e["Tags"])
        d_outputDeck_p->d_outTags.push_back(f.as<std::string>());

    if (e["Output_Interval"])
      d_outputDeck_p->d_dtOut = e["Output_Interval"].as<size_t>();
    d_outputDeck_p->d_dtOutOld = d_outputDeck_p->d_dtOut;
    d_outputDeck_p->d_dtOutCriteria = d_outputDeck_p->d_dtOut;
    if (e["Debug"]) d_outputDeck_p->d_debug = e["Debug"].as<size_t>();
    if (e["Perform_FE_Out"])
      d_outputDeck_p->d_performFEOut = e["Perform_FE_Out"].as<bool>();
    if (e["Compress_Type"])
      d_outputDeck_p->d_compressType = e["Compress_Type"].as<std::string>();
    if (e["Output_Criteria"]) {
      if (e["Output_Criteria"]["Type"])
        d_outputDeck_p->d_outCriteria =
            e["Output_Criteria"]["Type"].as<std::string>();
      if (e["Output_Criteria"]["New_Interval"])
        d_outputDeck_p->d_dtOutCriteria =
            e["Output_Criteria"]["New_Interval"].as<size_t>();
      if (e["Output_Criteria"]["Parameters"]) {
        auto ps = e["Output_Criteria"]["Parameters"];
        for (auto p : ps)
          d_outputDeck_p->d_outCriteriaParams.emplace_back(p.as<double>());
      }
    }
  }
}  // setOutputDeck

void inp::Input::setPolicyDeck() {
  d_policyDeck_p = new inp::PolicyDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  if (config["Policy"]["Memory_Consumption_Flag"])
    d_policyDeck_p->d_memControlFlag =
        config["Policy"]["Memory_Consumption_Flag"].as<int>();
  if (config["Policy"]["Enable_PostProcessing"])
    d_policyDeck_p->d_enablePostProcessing =
        config["Policy"]["Enable_PostProcessing"].as<bool>();
}  // setPolicyDeck

void inp::Input::setSolverDeck() {
  d_solverDeck_p = new inp::SolverDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  if (config["Solver"]) {
    auto e = config["Solver"];
    if (e["Type"]) d_solverDeck_p->d_solverType = e["Type"].as<std::string>();
    if (e["Max_Iteration"])
      d_solverDeck_p->d_maxIters = e["Max_Iteration"].as<int>();
    if (e["Tolerance"]) d_solverDeck_p->d_tol = e["Tolerance"].as<double>();
    if (e["Perturbation"])
      d_solverDeck_p->d_perturbation = e["Perturbation"].as<double>();
  }
}  // setSolverDeck

void inp::Input::setAbsorbingCondDeck() {
  d_absorbingCondDeck_p = new inp::AbsorbingCondDeck();
  YAML::Node config = YAML::LoadFile(d_inputFilename);

  auto ci = config["Absorbing_Condition"];
  if (!ci) return;

  // set damping active to true
  d_absorbingCondDeck_p->d_dampingActive = true;

  // get absorbing condition type
  if (ci["Is_Viscous"])
    d_absorbingCondDeck_p->d_isViscousDamping = ci["Is_Viscous"].as<bool>();

  // get damping coefficient type
  if (!ci["Damping_Coefficient"]) {
    std::cerr << "Error: Expecting data in block "
                 "Absorbing_Condition->Damping_Coefficient"
              << std::endl;
    exit(1);
  } else {
    d_absorbingCondDeck_p->d_dampingCoeffType =
        ci["Damping_Coefficient"]["Type"].as<std::string>();

    for (auto f : ci["Damping_Coefficient"]["Parameters"])
      d_absorbingCondDeck_p->d_dampingCoeffParams.push_back(f.as<double>());
  }

  // get damping regions
  if (!ci["Damping_Num_Regions"]) {
    std::cerr << "Error: Expecting number of damping regions data in "
                 "Absorbing_Condition->Damping_Num_Regions"
              << std::endl;
    exit(1);
  }
  int n = ci["Damping_Num_Regions"].as<int>();

  for (int i = 1; i <= n; i++) {
    // create tag for i^th region
    // prepare string Set_s to read file
    std::string read_set = "Region_";
    read_set.append(std::to_string(i));
    auto e = ci[read_set];

    auto dg = DampingGeomData();

    // get relative location of region
    if (!e["Relative_Loc"]) {
      std::cerr << "Error: Must specify relative location of damping region "
                   "with respect to whole simulation domain"
                << std::endl;
      exit(1);
    } else {
      dg.d_relativeLoc = e["Relative_Loc"].as<std::string>();
    }

    // get check info
    if (!e["Check_Flags"]) {
      std::cerr << "Error: Must specify values in Check_Flags" << std::endl;
      exit(1);
    } else {
      size_t lco = 0;
      for (auto f : e["Check_Flags"]) {
        if (lco == 0)
          dg.d_checkX = f.as<bool>();
        else if (lco == 1)
          dg.d_checkY = f.as<bool>();
        else if (lco == 2)
          dg.d_checkZ = f.as<bool>();

        lco++;
      }

      if (lco != 3) {
        std::cerr << "Error: Not sufficient parameters in Check_Flags\n";
        exit(1);
      }
    }

    // get layer thickness
    if (!e["Layer_Thickness"]) {
      std::cerr << "Error: Must specify values in Layer_Thickness" << std::endl;
      exit(1);
    } else {
      size_t lco = 0;
      for (auto f : e["Layer_Thickness"]) {
        if (lco == 0)
          dg.d_layerThicknessX = f.as<double>();
        else if (lco == 1)
          dg.d_layerThicknessY = f.as<double>();
        else if (lco == 2)
          dg.d_layerThicknessZ = f.as<double>();

        lco++;
      }

      if (lco != 3) {
        std::cerr << "Error: Not sufficient parameters in Layer_Thickness\n";
        exit(1);
      }
    }

    // For now we assume it has two corner points of rectangle region
    std::vector<double> locs;
    if (!e["Domain"]) {
      std::cerr << "Error: Domain of damping region is not provided.\n";
      exit(1);
    }

    for (auto f : e["Domain"]) locs.push_back(f.as<double>());

    if (locs.size() != 4 and locs.size() != 6) {
      std::cerr << "Error: We expect data Absorbing_Condition->" << read_set
                << " has four/six entries giving corner points of a rectangle"
                << std::endl;
      exit(1);
    }

    if (locs.size() == 4) {
      dg.d_p1 = util::Point3(locs[0], locs[1], 0.);
      dg.d_p2 = util::Point3(locs[2], locs[3], 0.);
    } else if (locs.size() == 6) {
      dg.d_p1 = util::Point3(locs[0], locs[1], locs[2]);
      dg.d_p2 = util::Point3(locs[3], locs[4], locs[5]);
    }

    d_absorbingCondDeck_p->d_dampingGeoms.push_back(dg);
  }

  if (d_absorbingCondDeck_p->d_dampingGeoms.size() != n) {
    std::cerr << "Error: Check damping region data" << std::endl;
    exit(1);
  }
}  // setAbsorbingCondDeck

std::string inp::Input::printStr(int nt, int lvl) const {
  auto tabS = util::io::getTabS(nt);
  std::ostringstream oss;
  oss << tabS << "------- Input --------" << std::endl << std::endl;
  oss << tabS << "Filename = " << d_inputFilename << std::endl;
  oss << tabS << std::endl;

  return oss.str();
}