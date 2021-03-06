////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2019 Prashant K. Jha
//  Copyright (c) 2019 Patrick Diehl
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include "writer.h"

#include "legacyVtkWriter.h"
#include "mshWriter.h"
#include "vtkWriter.h"

rw::writer::Writer::Writer()
    : d_vtkWriter_p(nullptr),
      d_legacyVtkWriter_p(nullptr),
      d_mshWriter_p(nullptr),
      d_format("vtu") {}

rw::writer::Writer::Writer(const std::string &filename,
                           const std::string &format,
                           const std::string &compress_type)
    : d_vtkWriter_p(nullptr),
      d_legacyVtkWriter_p(nullptr),
      d_mshWriter_p(nullptr),
      d_format("vtu") {
  open(filename, format, compress_type);
}

void rw::writer::Writer::open(const std::string &filename,
                              const std::string &format,
                              const std::string &compress_type) {
  d_format = format;
  if (d_format == "vtu")
    d_vtkWriter_p = new rw::writer::VtkWriter(filename, compress_type);
  else if (d_format == "msh")
    d_mshWriter_p = new rw::writer::MshWriter(filename, compress_type);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p =
        new rw::writer::LegacyVtkWriter(filename, compress_type);
}

rw::writer::Writer::~Writer() { delete (d_vtkWriter_p); }

void rw::writer::Writer::appendNodes(const std::vector<util::Point3> *nodes,
                                     const std::vector<util::Point3> *u) {
  if (d_format == "vtu")
    d_vtkWriter_p->appendNodes(nodes, u);
  else if (d_format == "msh")
    d_mshWriter_p->appendNodes(nodes, u);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendNodes(nodes, u);
}

void rw::writer::Writer::appendMesh(const std::vector<util::Point3> *nodes,
                                    const size_t &element_type,
                                    const std::vector<size_t> *en_con,
                                    const std::vector<util::Point3> *u) {
  if (d_format == "vtu")
    d_vtkWriter_p->appendMesh(nodes, element_type, en_con, u);
  else if (d_format == "msh")
    d_mshWriter_p->appendMesh(nodes, element_type, en_con, u);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendMesh(nodes, element_type, en_con, u);
}

void rw::writer::Writer::appendPointData(const std::string &name,
                                         const std::vector<uint8_t> *data) {
  checkLength(data->size(), name);

  if (d_format == "vtu")
    d_vtkWriter_p->appendPointData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendPointData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendPointData(name, data);
}

void rw::writer::Writer::appendPointData(const std::string &name,
                                         const std::vector<size_t> *data) {
  checkLength(data->size(), name);

  if (d_format == "vtu")
    d_vtkWriter_p->appendPointData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendPointData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendPointData(name, data);
}

void rw::writer::Writer::appendPointData(const std::string &name,
                                         const std::vector<int> *data) {
  checkLength(data->size(), name);
  if (d_format == "vtu")
    d_vtkWriter_p->appendPointData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendPointData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendPointData(name, data);
}

void rw::writer::Writer::appendPointData(const std::string &name,
                                         const std::vector<float> *data) {
  checkLength(data->size(), name);
  if (d_format == "vtu")
    d_vtkWriter_p->appendPointData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendPointData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendPointData(name, data);
}

void rw::writer::Writer::appendPointData(const std::string &name,
                                         const std::vector<double> *data) {
  checkLength(data->size(), name);

  if (d_format == "vtu")
    d_vtkWriter_p->appendPointData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendPointData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendPointData(name, data);
}

void rw::writer::Writer::appendPointData(
    const std::string &name, const std::vector<util::Point3> *data) {
  checkLength(data->size(), name);

  if (d_format == "vtu")
    d_vtkWriter_p->appendPointData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendPointData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendPointData(name, data);
}

void rw::writer::Writer::appendPointData(
    const std::string &name, const std::vector<util::SymMatrix3> *data) {
  checkLength(data->size(), name);
  if (d_format == "vtu")
    d_vtkWriter_p->appendPointData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendPointData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendPointData(name, data);
}

void rw::writer::Writer::appendPointData(
    const std::string &name,
    const std::vector<blaze::StaticMatrix<double, 3, 3> > *data) {
  checkLength(data->size(), name);
  if (d_format == "vtu") d_vtkWriter_p->appendPointData(name, data);
  // else if (d_format == "msh")
  //  d_mshWriter_p->appendPointData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendPointData(name, data);
}

void rw::writer::Writer::appendCellData(const std::string &name,
                                        const std::vector<float> *data) {
  checkLength(data->size(), name);
  if (d_format == "vtu")
    d_vtkWriter_p->appendCellData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendCellData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendCellData(name, data);
}

void rw::writer::Writer::appendCellData(
    const std::string &name, const std::vector<util::SymMatrix3> *data) {
  checkLength(data->size(), name);
  if (d_format == "vtu")
    d_vtkWriter_p->appendCellData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendCellData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendCellData(name, data);
}

void rw::writer::Writer::addTimeStep(const double &timestep) {
  if (d_format == "vtu")
    d_vtkWriter_p->addTimeStep(timestep);
  else if (d_format == "msh")
    d_mshWriter_p->addTimeStep(timestep);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->addTimeStep(timestep);
}

void rw::writer::Writer::appendFieldData(const std::string &name,
                                         const double &data) {
  if (d_format == "vtu")
    d_vtkWriter_p->appendFieldData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendFieldData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendFieldData(name, data);
}

void rw::writer::Writer::appendFieldData(const std::string &name,
                                         const float &data) {
  if (d_format == "vtu")
    d_vtkWriter_p->appendFieldData(name, data);
  else if (d_format == "msh")
    d_mshWriter_p->appendFieldData(name, data);
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->appendFieldData(name, data);
}

void rw::writer::Writer::close() {
  if (d_format == "vtu")
    d_vtkWriter_p->close();
  else if (d_format == "msh")
    d_mshWriter_p->close();
  else if (d_format == "legacy_vtk")
    d_legacyVtkWriter_p->close();
}

inline void rw::writer::Writer::checkLength(const size_t length,
                                            const std::string &name) {
  if (length == 0) {
    std::cerr << "Warning: Output field " << name << " is empty!" << std::endl;
  }
}

void rw::writer::Writer::writeInitialCrack(const std::string &filename,
                                           const std::string &compress_type,
                                           util::Point3 start,
                                           util::Point3 end) {
  if (d_format == "vtu")
    d_vtkWriter_p->writeInitialCrack(filename, compress_type, start, end);
  else
    std::cerr << "Warning: Writing the initial crack only avaible using the "
                 "VTK writer!"
              << std::endl;
}