/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2013 Lawrence Livermore National Security, LLC
 * Description:   Asynchronous Berger-Rigoutsos clustering algorithm.
 *
 ************************************************************************/
#ifndef included_mesh_TileClustering
#define included_mesh_TileClustering

#include "SAMRAI/SAMRAI_config.h"

#include "SAMRAI/mesh/BoxGeneratorStrategy.h"
#include "SAMRAI/hier/Connector.h"
#include "SAMRAI/hier/BoxLevel.h"
#include "SAMRAI/hier/PatchLevel.h"
#include "SAMRAI/pdat/CellData.h"
#include "SAMRAI/tbox/Database.h"
#include "SAMRAI/tbox/Utilities.h"

#include "boost/shared_ptr.hpp"

namespace SAMRAI {
namespace mesh {

/*!
 * @brief Tiled patch clustering algorithm.
 *
 * User inputs (default):
 *
 * - IntVector @b box_size Box size in the index space of the tag level.
 *
 * Developer inputs (default):
 *
 * - bool @b DEV_barrier_and_time (false):
 *   Activate barriers and performance timers.
 *
 * - bool @b DEV_log_cluster_summary (false):
 *   Whether to briefly log the results of the clustering.
 *
 * - bool @b DEV_log_cluster (false):
 *   Whether to log the results of the clustering.
 */
class TileClustering:public BoxGeneratorStrategy
{

public:
   /*!
    * @brief Constructor.
    */
   explicit TileClustering(
      const tbox::Dimension& dim,
      const boost::shared_ptr<tbox::Database>& input_db =
         boost::shared_ptr<tbox::Database>());

   /*!
    * @brief Destructor.
    *
    * Deallocate internal data.
    */
   virtual ~TileClustering();

   /*!
    * @brief Implement the mesh::BoxGeneratorStrategy interface
    * method of the same name.
    *
    * Create a set of boxes that covers all integer tags on
    * the patch level that match the specified tag value.
    * Each box will be at least as large as the given minimum
    * size and the tolerances will be met.
    *
    * The efficiency tolerance is a threshold value for the percentage of
    * tagged cells in each box.  If this percentage is below the tolerance,
    * the box will continue to be split into smaller boxes.
    *
    * The combine tolerance is a threshold value for the sum of the volumes
    * of two boxes into which a box may be potentially split.  If ratio of
    * that sum and the volume of the original box, the box will not be split.
    */
   void
   findBoxesContainingTags(
      boost::shared_ptr<hier::BoxLevel>& new_box_level,
      boost::shared_ptr<hier::Connector>& tag_to_new,
      const boost::shared_ptr<hier::PatchLevel>& tag_level,
      const int tag_data_index,
      const int tag_val,
      const hier::BoxContainer& bound_boxes,
      const hier::IntVector& min_box,
      const double efficiency_tol,
      const double combine_tol,
      const hier::IntVector& max_gcw) const;

protected:
   /*!
    * @brief Read parameters from input database.
    *
    * @param input_db Input Database.
    */
   void
   getFromInput(
      const boost::shared_ptr<tbox::Database>& input_db);

private:

   /*!
    * Create, populate and return a coarsened version of the given tag data.
    *
    * The coarse cell values are set to tag_data if any corresponding
    * fine cell value is tag_data.  Otherwise, the coarse cell value
    * is set to zero.
    */
   boost::shared_ptr<pdat::CellData<int> >
   makeCoarsenedTagData(const pdat::CellData<int> &tag_data,
                        int tag_value) const;

   void setTimers();

   const tbox::Dimension d_dim;

   //! @brief Box size constraint.
   hier::IntVector d_box_size;

   bool d_log_cluster_summary;
   bool d_log_cluster;

   //@{
   //! @name Used for evaluating performance;
   bool d_barrier_and_time;
   boost::shared_ptr<tbox::Timer> t_find_boxes_containing_tags;
   boost::shared_ptr<tbox::Timer> t_cluster;
   boost::shared_ptr<tbox::Timer> t_get_connectors;
   boost::shared_ptr<tbox::Timer> t_global_reductions;
   //@}

};

}
}

#endif  // included_mesh_TileClustering