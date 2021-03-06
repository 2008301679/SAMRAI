/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and LICENSE.
 *
 * Copyright:     (c) 1997-2017 Lawrence Livermore National Security, LLC
 * Description:   Base class for geometry management in AMR hierarchy
 *
 ************************************************************************/
#include "SAMRAI/geom/GridGeometry.h"

#include "SAMRAI/pdat/NodeComplexInjection.h"
#include "SAMRAI/pdat/NodeDoubleInjection.h"
#include "SAMRAI/pdat/NodeFloatInjection.h"
#include "SAMRAI/pdat/NodeIntegerInjection.h"
#include "SAMRAI/pdat/OuternodeDoubleInjection.h"
#include "SAMRAI/pdat/CellComplexConstantRefine.h"
#include "SAMRAI/pdat/CellDoubleConstantRefine.h"
#include "SAMRAI/pdat/CellFloatConstantRefine.h"
#include "SAMRAI/pdat/CellIntegerConstantRefine.h"
#include "SAMRAI/pdat/EdgeComplexConstantRefine.h"
#include "SAMRAI/pdat/EdgeDoubleConstantRefine.h"
#include "SAMRAI/pdat/EdgeFloatConstantRefine.h"
#include "SAMRAI/pdat/EdgeIntegerConstantRefine.h"
#include "SAMRAI/pdat/FaceComplexConstantRefine.h"
#include "SAMRAI/pdat/FaceDoubleConstantRefine.h"
#include "SAMRAI/pdat/FaceFloatConstantRefine.h"
#include "SAMRAI/pdat/FaceIntegerConstantRefine.h"
#include "SAMRAI/pdat/OuterfaceComplexConstantRefine.h"
#include "SAMRAI/pdat/OuterfaceDoubleConstantRefine.h"
#include "SAMRAI/pdat/OuterfaceFloatConstantRefine.h"
#include "SAMRAI/pdat/OuterfaceIntegerConstantRefine.h"
#include "SAMRAI/pdat/SideComplexConstantRefine.h"
#include "SAMRAI/pdat/SideDoubleConstantRefine.h"
#include "SAMRAI/pdat/SideFloatConstantRefine.h"
#include "SAMRAI/pdat/SideIntegerConstantRefine.h"
#include "SAMRAI/pdat/CellComplexLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/CellDoubleLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/CellFloatLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/EdgeComplexLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/EdgeDoubleLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/EdgeFloatLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/FaceComplexLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/FaceDoubleLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/FaceFloatLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/NodeComplexLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/NodeDoubleLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/NodeFloatLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/OuterfaceComplexLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/OuterfaceDoubleLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/OuterfaceFloatLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/OutersideComplexLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/OutersideDoubleLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/OutersideFloatLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/SideComplexLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/SideDoubleLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/SideFloatLinearTimeInterpolateOp.h"
#include "SAMRAI/pdat/CellVariable.h"
#include "SAMRAI/pdat/EdgeVariable.h"
#include "SAMRAI/pdat/FaceVariable.h"
#include "SAMRAI/pdat/NodeVariable.h"
#include "SAMRAI/pdat/OuterfaceVariable.h"
#include "SAMRAI/pdat/OuternodeVariable.h"
#include "SAMRAI/pdat/OutersideVariable.h"
#include "SAMRAI/pdat/SideVariable.h"

#include <typeinfo>
#include <stdlib.h>
#include "boost/make_shared.hpp"

#define GEOM_BLOCK_GRID_GEOMETRY_VERSION (1)

#if !defined(__BGL_FAMILY__) && defined(__xlC__)
/*
 * Suppress XLC warnings
 */
#pragma report(disable, CPPC5334)
#pragma report(disable, CPPC5328)
#endif

namespace SAMRAI {
namespace geom {

/*
 *************************************************************************
 *
 * Constructors for GridGeometry.  Both set up operator
 * handlers.  However, one initializes data members based on arguments.
 * The other initializes the object based on input database information.
 *
 *************************************************************************
 */
GridGeometry::GridGeometry(
   const tbox::Dimension& dim,
   const std::string& object_name,
   const boost::shared_ptr<tbox::Database>& input_db,
   bool allow_multiblock):
   hier::BaseGridGeometry(dim, object_name, input_db, allow_multiblock)
{
   buildOperators();
}

/*
 *************************************************************************
 *
 * Constructors for GridGeometry.  Both set up operator
 * handlers.  However, one initializes data members based on arguments.
 * The other initializes the object based on input database information.
 *
 *************************************************************************
 */
GridGeometry::GridGeometry(
   const std::string& object_name,
   hier::BoxContainer& domain):
   hier::BaseGridGeometry(object_name, domain)
{
   buildOperators();
}

GridGeometry::GridGeometry(
   const std::string& object_name,
   hier::BoxContainer& domain,
   const boost::shared_ptr<hier::TransferOperatorRegistry>& op_reg):
   hier::BaseGridGeometry(object_name, domain, op_reg)
{
   buildOperators();
}

/*
 *************************************************************************
 *
 * Empty destructor.
 *
 *************************************************************************
 */

GridGeometry::~GridGeometry()
{
}

/*
 *************************************************************************
 *
 * Create and return pointer to coarsened version of this
 * grid geometry object coarsened by the given ratio.
 *
 *************************************************************************
 */

boost::shared_ptr<hier::BaseGridGeometry>
GridGeometry::makeCoarsenedGridGeometry(
   const std::string& coarse_geom_name,
   const hier::IntVector& coarsen_ratio) const
{
   const tbox::Dimension& dim(getDim());

   TBOX_ASSERT(!coarse_geom_name.empty());
   TBOX_ASSERT(coarse_geom_name != getObjectName());
   TBOX_ASSERT_DIM_OBJDIM_EQUALITY1(dim, coarsen_ratio);
   TBOX_ASSERT(coarsen_ratio > hier::IntVector::getZero(dim));

   hier::BoxContainer coarse_domain;

   coarse_domain = getPhysicalDomain();
   coarse_domain.coarsen(coarsen_ratio);

   /*
    * Need to check that domain can be coarsened by given ratio.
    */
   const hier::BoxContainer& fine_domain = getPhysicalDomain();
   const int nboxes = fine_domain.size();
   hier::BoxContainer::const_iterator coarse_domain_itr = coarse_domain.begin();
   hier::BoxContainer::const_iterator fine_domain_itr = fine_domain.begin();
   for (int ib = 0; ib < nboxes; ++ib, ++coarse_domain_itr, ++fine_domain_itr) {
      hier::Box testbox =
         hier::Box::refine(*coarse_domain_itr,
                           coarsen_ratio);
      if (!testbox.isSpatiallyEqual(*fine_domain_itr)) {
#ifdef DEBUG_CHECK_ASSERTIONS
         tbox::plog
         << "GridGeometry::makeCoarsenedGridGeometry : Box # "
         << ib << std::endl;
         tbox::plog << "   fine box = " << *fine_domain_itr << std::endl;
         tbox::plog << "f   coarse box = " << *coarse_domain_itr << std::endl;
         tbox::plog << "   refined coarse box = " << testbox << std::endl;
#endif
         TBOX_ERROR(
            "GridGeometry::makeCoarsenedGridGeometry() error...\n"
            << "    geometry object with name = " << getObjectName()
            << "\n    Cannot be coarsened by ratio " << coarsen_ratio
            << std::endl);
      }
   }

   boost::shared_ptr<hier::BaseGridGeometry> coarse_geometry(
      new GridGeometry(
         coarse_geom_name,
         coarse_domain,
         d_transfer_operator_registry));

   coarse_geometry->initializePeriodicShift(getPeriodicShift(
         hier::IntVector::getOne(dim)));

   return coarse_geometry;
}

/*
 *************************************************************************
 *
 * Create and return pointer to refined version of this
 * grid geometry object refined by the given ratio.
 *
 *************************************************************************
 */

boost::shared_ptr<hier::BaseGridGeometry>
GridGeometry::makeRefinedGridGeometry(
   const std::string& fine_geom_name,
   const hier::IntVector& refine_ratio) const
{
   const tbox::Dimension& dim(getDim());

   TBOX_ASSERT(!fine_geom_name.empty());
   TBOX_ASSERT(fine_geom_name != getObjectName());
   TBOX_ASSERT_DIM_OBJDIM_EQUALITY1(dim, refine_ratio);
   TBOX_ASSERT(refine_ratio > hier::IntVector::getZero(dim));

   hier::BoxContainer fine_domain(getPhysicalDomain());
   fine_domain.refine(refine_ratio);

   boost::shared_ptr<hier::BaseGridGeometry> fine_geometry(
      new GridGeometry(
         fine_geom_name,
         fine_domain,
         d_transfer_operator_registry));

   fine_geometry->initializePeriodicShift(getPeriodicShift(
         hier::IntVector::getOne(dim)));

   return fine_geometry;
}

void
GridGeometry::buildOperators()
{
   // Coarsening Operators
   addCoarsenOperator(
      typeid(pdat::NodeVariable<dcomplex>).name(),
      boost::make_shared<pdat::NodeComplexInjection>());
   addCoarsenOperator(
      typeid(pdat::NodeVariable<double>).name(),
      boost::make_shared<pdat::NodeDoubleInjection>());
   addCoarsenOperator(
      typeid(pdat::NodeVariable<float>).name(),
      boost::make_shared<pdat::NodeFloatInjection>());
   addCoarsenOperator(
      typeid(pdat::NodeVariable<int>).name(),
      boost::make_shared<pdat::NodeIntegerInjection>());
   addCoarsenOperator(
      typeid(pdat::OuternodeVariable<double>).name(),
      boost::make_shared<pdat::OuternodeDoubleInjection>());

   // Refinement Operators
   addRefineOperator(
      typeid(pdat::CellVariable<dcomplex>).name(),
      boost::make_shared<pdat::CellComplexConstantRefine>());
   addRefineOperator(
      typeid(pdat::CellVariable<double>).name(),
      boost::make_shared<pdat::CellDoubleConstantRefine>());
   addRefineOperator(
      typeid(pdat::CellVariable<float>).name(),
      boost::make_shared<pdat::CellFloatConstantRefine>());
   addRefineOperator(
      typeid(pdat::CellVariable<int>).name(),
      boost::make_shared<pdat::CellIntegerConstantRefine>());
   addRefineOperator(
      typeid(pdat::EdgeVariable<dcomplex>).name(),
      boost::make_shared<pdat::EdgeComplexConstantRefine>());
   addRefineOperator(
      typeid(pdat::EdgeVariable<double>).name(),
      boost::make_shared<pdat::EdgeDoubleConstantRefine>());
   addRefineOperator(
      typeid(pdat::EdgeVariable<float>).name(),
      boost::make_shared<pdat::EdgeFloatConstantRefine>());
   addRefineOperator(
      typeid(pdat::EdgeVariable<int>).name(),
      boost::make_shared<pdat::EdgeIntegerConstantRefine>());
   addRefineOperator(
      typeid(pdat::FaceVariable<dcomplex>).name(),
      boost::make_shared<pdat::FaceComplexConstantRefine>());
   addRefineOperator(
      typeid(pdat::FaceVariable<double>).name(),
      boost::make_shared<pdat::FaceDoubleConstantRefine>());
   addRefineOperator(
      typeid(pdat::FaceVariable<float>).name(),
      boost::make_shared<pdat::FaceFloatConstantRefine>());
   addRefineOperator(
      typeid(pdat::FaceVariable<int>).name(),
      boost::make_shared<pdat::FaceIntegerConstantRefine>());
   addRefineOperator(
      typeid(pdat::OuterfaceVariable<dcomplex>).name(),
      boost::make_shared<pdat::OuterfaceComplexConstantRefine>());
   addRefineOperator(
      typeid(pdat::OuterfaceVariable<double>).name(),
      boost::make_shared<pdat::OuterfaceDoubleConstantRefine>());
   addRefineOperator(
      typeid(pdat::OuterfaceVariable<float>).name(),
      boost::make_shared<pdat::OuterfaceFloatConstantRefine>());
   addRefineOperator(
      typeid(pdat::OuterfaceVariable<int>).name(),
      boost::make_shared<pdat::OuterfaceIntegerConstantRefine>());
   addRefineOperator(
      typeid(pdat::SideVariable<dcomplex>).name(),
      boost::make_shared<pdat::SideComplexConstantRefine>());
   addRefineOperator(
      typeid(pdat::SideVariable<double>).name(),
      boost::make_shared<pdat::SideDoubleConstantRefine>());
   addRefineOperator(
      typeid(pdat::SideVariable<float>).name(),
      boost::make_shared<pdat::SideFloatConstantRefine>());
   addRefineOperator(
      typeid(pdat::SideVariable<int>).name(),
      boost::make_shared<pdat::SideIntegerConstantRefine>());

   // Time Interpolation Operators
   addTimeInterpolateOperator(
      typeid(pdat::CellVariable<dcomplex>).name(),
      boost::make_shared<pdat::CellComplexLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::CellVariable<double>).name(),
      boost::make_shared<pdat::CellDoubleLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::CellVariable<float>).name(),
      boost::make_shared<pdat::CellFloatLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::EdgeVariable<dcomplex>).name(),
      boost::make_shared<pdat::EdgeComplexLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::EdgeVariable<double>).name(),
      boost::make_shared<pdat::EdgeDoubleLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::EdgeVariable<float>).name(),
      boost::make_shared<pdat::EdgeFloatLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::FaceVariable<dcomplex>).name(),
      boost::make_shared<pdat::FaceComplexLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::FaceVariable<double>).name(),
      boost::make_shared<pdat::FaceDoubleLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::FaceVariable<float>).name(),
      boost::make_shared<pdat::FaceFloatLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::NodeVariable<dcomplex>).name(),
      boost::make_shared<pdat::NodeComplexLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::NodeVariable<double>).name(),
      boost::make_shared<pdat::NodeDoubleLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::NodeVariable<float>).name(),
      boost::make_shared<pdat::NodeFloatLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::OuterfaceVariable<dcomplex>).name(),
      boost::make_shared<pdat::OuterfaceComplexLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::OuterfaceVariable<double>).name(),
      boost::make_shared<pdat::OuterfaceDoubleLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::OuterfaceVariable<float>).name(),
      boost::make_shared<pdat::OuterfaceFloatLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::OutersideVariable<dcomplex>).name(),
      boost::make_shared<pdat::OutersideComplexLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::OutersideVariable<double>).name(),
      boost::make_shared<pdat::OutersideDoubleLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::OutersideVariable<float>).name(),
      boost::make_shared<pdat::OutersideFloatLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::SideVariable<dcomplex>).name(),
      boost::make_shared<pdat::SideComplexLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::SideVariable<double>).name(),
      boost::make_shared<pdat::SideDoubleLinearTimeInterpolateOp>());
   addTimeInterpolateOperator(
      typeid(pdat::SideVariable<float>).name(),
      boost::make_shared<pdat::SideFloatLinearTimeInterpolateOp>());
}

}
}

#if !defined(__BGL_FAMILY__) && defined(__xlC__)
/*
 * Suppress XLC warnings
 */
#pragma report(enable, CPPC5334)
#pragma report(enable, CPPC5328)
#endif
