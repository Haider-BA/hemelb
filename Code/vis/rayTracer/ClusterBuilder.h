#ifndef HEMELB_VIS_RAYTRACER_CLUSTERBUILDER_H
#define HEMELB_VIS_RAYTRACER_CLUSTERBUILDER_H

#include <map>
#include <vector>		

#include "geometry/BlockTraverserWithVisitedBlockTracker.h"
#include "geometry/LatticeData.h"
#include "geometry/SiteTraverser.h"
#include "lb/LbmParameters.h"
#include "util/utilityFunctions.h"
#include "util/Vector3D.h"
#include "vis/rayTracer/ClusterBuilder.h"
#include "vis/rayTracer/ClusterTraverser.h"
#include "vis/rayTracer/RayTracer.h"
#include "vis/rayTracer/SiteData.h"
#include "log/Logger.h"

namespace hemelb
{
  namespace vis
  {
    namespace raytracer
    {
      template<typename ClusterType>
      class ClusterBuilder
      {
        public:
          ClusterBuilder(const geometry::LatticeData* latticeData) :
            mBlockTraverser(*latticeData)
          {
            mLatticeData = latticeData;

            //Each block is assigned a cluster id once it has been
            //assigned to a cluster
            mClusterIdOfBlock = new short int[mLatticeData->GetBlockCount()];
            for (site_t lId = 0; lId < mLatticeData->GetBlockCount(); lId++)
            {
              mClusterIdOfBlock[lId] = NOTASSIGNEDTOCLUSTER;
            }
          }

          ~ClusterBuilder()
          {
            delete[] mClusterIdOfBlock;
          }

          void BuildClusters()
          {
            //Initially locate all clusters, locating their
            //range by block span and site span
            LocateClusters();

            // Process the flow field for every cluster
            for (unsigned int lThisClusterId = 0; lThisClusterId < mClusters.size(); lThisClusterId++)
            {
              ProcessCluster(lThisClusterId);
            }
          }

          std::vector<ClusterType>& GetClusters()
          {
            return mClusters;
          }

          SiteData_t* GetClusterVoxelDataPointer(site_t iVoxelSiteId)
          {
            return GetDataPointerClusterVoxelSiteId(iVoxelSiteId);
          }

        private:
          void ResizeVectorsForBlock(ClusterType& cluster, site_t blockNum)
          {
            cluster.ResizeVectorsForBlock(blockNum,
                                          mLatticeData->GetSitesPerBlockVolumeUnit() * VIS_FIELDS);
          }

          // Locates all the clusters in the lattice structure and the
          void LocateClusters()
          {
            // Run through all unvisited blocks finding clusters
            do
            {
              //Mark the block visited 
              mBlockTraverser.MarkCurrentBlockVisited();

              //If there are sites assigned to the local processor, search for the
              //cluster of connected sites
              if (AreSitesAssignedToLocalProcessorRankInBlock(mBlockTraverser.GetCurrentBlockData()))
              {
                FindNewCluster();
              }
            }
            while (mBlockTraverser.GoToNextUnvisitedBlock());
          }

          //Locates all the clusters in the lattice structure and stores
          //their locations
          void FindNewCluster()
          {
            //These locations will eventually contain the bounds of the
            //rectangular cluster, both in terms of block number and
            //site numbers
            util::Vector3D<site_t> clusterBlockMin = util::Vector3D<site_t>::MaxLimit();
            util::Vector3D<site_t> clusterBlockMax = util::Vector3D<site_t>::MinLimit();
            util::Vector3D<site_t> clusterSiteMin = util::Vector3D<site_t>::MaxLimit();
            util::Vector3D<site_t> clusterSiteMax = util::Vector3D<site_t>::MinLimit();

            //To discover the cluster, we continually visit the neighbours 
            //of sequential blocks 
            //We keep a stack of all the sites that must be processed 
            //and sequentially add neighbours to it
            std::stack<util::Vector3D<site_t> > blocksToProcess;

            //Set up the initial condition
            blocksToProcess.push(mBlockTraverser.GetCurrentLocation());

            //Loop over the cluster via neighbours until
            //all blocks have been processed
            while (!blocksToProcess.empty())
            {
              //Get location off the top of the stack
              //(we could actually take anything off the stack)
              util::Vector3D<site_t> lCurrentLocation = blocksToProcess.top();
              blocksToProcess.pop();

              if (AreSitesAssignedToLocalProcessorRankInBlock(mBlockTraverser.GetBlockDataForLocation(lCurrentLocation)))
              {
                //Update block range of the cluster
                clusterBlockMin.UpdatePointwiseMin(lCurrentLocation);
                clusterBlockMax.UpdatePointwiseMax(lCurrentLocation);

                //Update the cluster id of the given block
                site_t blockId = mBlockTraverser.GetIndexFromLocation(lCurrentLocation);
                mClusterIdOfBlock[blockId] = (short int) mClusters.size();

                //Loop through all the sites on the block, to 
                //update the site bounds on the cluster
                geometry::SiteTraverser siteTraverser = mBlockTraverser.GetSiteTraverser();
                do
                {
                  //If the site is not a solid
                  if (mBlockTraverser.GetBlockDataForLocation(lCurrentLocation)->site_data[siteTraverser.GetCurrentIndex()]
                      != BIG_NUMBER3)
                  {
                    clusterSiteMin.UpdatePointwiseMin(siteTraverser.GetCurrentLocation()
                        + lCurrentLocation * mBlockTraverser.GetBlockSize());

                    clusterSiteMax.UpdatePointwiseMax(siteTraverser.GetCurrentLocation()
                        + lCurrentLocation * mBlockTraverser.GetBlockSize());
                  }
                }
                while (siteTraverser.TraverseOne());

                //Check all the neighbouring blocks to see if they need visiting. Add them to the stack.
                AddNeighbouringBlocks(lCurrentLocation, blocksToProcess);
              }
            }

            AddCluster(clusterBlockMin, clusterBlockMax, clusterSiteMin, clusterSiteMax);
          }

          //Adds neighbouring blocks of the input location to the input stack
          void AddNeighbouringBlocks(util::Vector3D<site_t> iCurrentLocation,
                                     std::stack<util::Vector3D<site_t> >& oBlocksToProcess)
          {
            // Loop over all neighbouring blocks
            for (int l = 0; l < 26; l++)
            {
              util::Vector3D<site_t> lNeighbouringBlock = iCurrentLocation + mNeighbours[l];

              //The neighouring block location might not exist
              //eg negative co-ordinates
              if (mBlockTraverser.IsValidLocation(lNeighbouringBlock))
              {
                //Ensure that the block hasn't been visited before
                if (!mBlockTraverser.IsBlockVisited(lNeighbouringBlock))
                {
                  //Add to the stack
                  oBlocksToProcess.push(lNeighbouringBlock);

                  //We must mark this locatoin as visited so it only 
                  //gets processed once
                  mBlockTraverser.MarkBlockVisited(lNeighbouringBlock);
                }
              }
            }
          }

          //Returns true if there are sites in the given block associated with the
          //local processor rank
          bool AreSitesAssignedToLocalProcessorRankInBlock(geometry::BlockData * block)
          {
            if (block->ProcessorRankForEachBlockSite == NULL)
            {
              return false;
            }

            for (unsigned int siteId = 0; siteId < mLatticeData->GetSitesPerBlockVolumeUnit(); siteId++)
            {
              if (topology::NetworkTopology::Instance()->GetLocalRank()
                  == block->ProcessorRankForEachBlockSite[siteId])
              {
                return true;
              }
            }
            return false;
          }

          //Adds a new cluster by taking in the required data in interger format
          //and converting it to that used by the raytracer
          //NB: Futher processing is required on the cluster before it can be used
          //by the ray tracer, which is handled by the ProcessCluster method
          void AddCluster(util::Vector3D<site_t> clusterBlockMin,
                          util::Vector3D<site_t> clusterBlockMax,
                          util::Vector3D<site_t> clusterVoxelMin,
                          util::Vector3D<site_t> clusterVoxelMax)
          {
            //The friendly locations must be turned into a format usable by the ray tracer
            ClusterType lNewCluster;
            lNewCluster.minBlock.x = (float) (clusterBlockMin.x * mLatticeData->GetBlockSize())
                - 0.5F * (float) mLatticeData->GetXSiteCount();
            lNewCluster.minBlock.y = (float) (clusterBlockMin.y * mLatticeData->GetBlockSize())
                - 0.5F * (float) mLatticeData->GetYSiteCount();
            lNewCluster.minBlock.z = (float) (clusterBlockMin.z * mLatticeData->GetBlockSize())
                - 0.5F * (float) mLatticeData->GetZSiteCount();

            lNewCluster.blocksX = (unsigned short) (1 + clusterBlockMax.x - clusterBlockMin.x);
            lNewCluster .blocksY = (unsigned short) (1 + clusterBlockMax.y - clusterBlockMin.y);
            lNewCluster .blocksZ = (unsigned short) (1 + clusterBlockMax.z - clusterBlockMin.z);

            lNewCluster.minSite = util::Vector3D<float>(clusterVoxelMin)
                - util::Vector3D<float>((float) mLatticeData->GetXSiteCount(),
                                        (float) mLatticeData->GetYSiteCount(),
                                        (float) mLatticeData->GetZSiteCount()) * 0.5F;

            lNewCluster.maxSite
                = util::Vector3D<float>(clusterVoxelMax + util::Vector3D<site_t>(1))
                    - util::Vector3D<float>(0.5F * (float) mLatticeData->GetXSiteCount(),
                                            0.5F * (float) mLatticeData->GetYSiteCount(),
                                            0.5F * (float) mLatticeData->GetZSiteCount());

            mClusters.push_back(lNewCluster);

            //We need to store the cluster block minimum in
            //order to process the cluster
            mClusterBlockMins.push_back(clusterBlockMin);
          }

          //Adds "flow-field" data to the cluster
          void ProcessCluster(unsigned int clusterId)
          {
            hemelb::log::Logger::Log<hemelb::log::Debug, hemelb::log::OnePerCore>("Examining cluster id = %u",
                                                                                  (unsigned int) clusterId);

            ClusterType& cluster(mClusters[clusterId]);

            cluster.ResizeVectors();

            ClusterTraverser<ClusterType> clusterTraverser(cluster);

            do
            {
              util::Vector3D<site_t> blockCoordinates = clusterTraverser.GetCurrentLocation()
                  + mClusterBlockMins[clusterId];

              site_t blockId = mLatticeData->GetBlockIdFromBlockCoords(blockCoordinates.x,
                                                                       blockCoordinates.y,
                                                                       blockCoordinates.z);

              if (mClusterIdOfBlock[blockId] == (short) clusterId)
              {
                ResizeVectorsForBlock(cluster, clusterTraverser.GetCurrentIndex());

                mClusterVoxelDataPointers.resize(mLatticeData->GetLocalFluidSiteCount());

                UpdateSiteData(blockId, clusterTraverser.GetCurrentIndex(), cluster);
              }
            }
            while (clusterTraverser.TraverseOne());
          }

          void UpdateSiteData(site_t blockId, site_t blockNum, ClusterType& cluster)
          {
            geometry::SiteTraverser siteTraverser(*mLatticeData);
            do
            {
              UpdateSiteDataAtSite(blockId, blockNum, cluster, siteTraverser.GetCurrentIndex());
            }
            while (siteTraverser.TraverseOne());
          }

          virtual void UpdateSiteDataAtSite(site_t blockId,
                                            site_t blockNum,
                                            ClusterType& cluster,
                                            site_t siteIdOnBlock)
          {
            geometry::BlockData * block = mLatticeData->GetBlock(blockId);
            unsigned int clusterVoxelSiteId = block->site_data[siteIdOnBlock];

            //If site not a solid and on the current processor [net.cc]
            if (clusterVoxelSiteId != BIG_NUMBER3)
            {
              UpdateDensityVelocityAndStress(blockNum, cluster, siteIdOnBlock, clusterVoxelSiteId);
              if (ClusterType::NeedsWallNormals())
              {
                UpdateWallNormalAtSite(block, blockNum, cluster, siteIdOnBlock);
              }
            }
          }

          void UpdateDensityVelocityAndStress(site_t blockNum,
                                              ClusterType& cluster,
                                              site_t siteIdOnBlock,
                                              unsigned int clusterVoxelSiteId)
          {
            cluster.SiteData[blockNum][siteIdOnBlock] = SiteData_t(1.0F);

            //For efficiency we want to store a pointer to the site data grouped by the ClusterVortexID
            //(1D organisation of sites)
            std::vector<SiteData_t>::iterator siteDataIterator = cluster.SiteData[blockNum].begin()
                + siteIdOnBlock;

            SiteData_t* siteDataLocation = & (*siteDataIterator);

            SetDataPointerForClusterVoxelSiteId(clusterVoxelSiteId, & (*siteDataLocation));
          }

          void UpdateWallNormalAtSite(geometry::BlockData * block,
                                      site_t blockNum,
                                      ClusterType& cluster,
                                      site_t siteIdOnBlock)
          {

            if (block->wall_data != NULL && block->wall_data[siteIdOnBlock].wall_nor[0] != -1.0F)
            {
              cluster.SetWallData(blockNum, siteIdOnBlock, block->wall_data[siteIdOnBlock].wall_nor);
            }
          }

          SiteData_t* GetDataPointerClusterVoxelSiteId(site_t clusterVortexSiteId)
          {
            return mClusterVoxelDataPointers[clusterVortexSiteId];
          }

          void SetDataPointerForClusterVoxelSiteId(site_t clusterVortexSiteId,
                                                   SiteData_t* dataPointer)
          {
            mClusterVoxelDataPointers[clusterVortexSiteId] = dataPointer;
          }

          //Caution: the data within mClusters is altered by means
          //of pointers obtained from the GetClusterVoxelDataPointer
          //method. No insertion or copying must therefore take place
          //on mClusters once building is complete
          std::vector<ClusterType> mClusters;

          const geometry::LatticeData* mLatticeData;

          geometry::BlockTraverserWithVisitedBlockTracker mBlockTraverser;

          std::vector<util::Vector3D<site_t> > mClusterBlockMins;

          //This allows a cluster voxel site ID (as part of the 1D structure for)
          //storing sites to be mapped to the data stored in the 3D structure
          //for the ray tracer by means of pointers.
          std::vector<SiteData_t*> mClusterVoxelDataPointers;

          short int *mClusterIdOfBlock;

          static const short int NOTASSIGNEDTOCLUSTER = -1;

          static const util::Vector3D<site_t> mNeighbours[26];
      };

      template<typename ClusterType>
      const util::Vector3D<site_t> ClusterBuilder<ClusterType>::mNeighbours[26] =
          { util::Vector3D<site_t>(-1, -1, -1),
            util::Vector3D<site_t>(-1, -1, 0),
            util::Vector3D<site_t>(-1, -1, 1),
            util::Vector3D<site_t>(-1, 0, -1),
            util::Vector3D<site_t>(-1, 0, 0),
            util::Vector3D<site_t>(-1, 0, 1),
            util::Vector3D<site_t>(-1, 1, -1),
            util::Vector3D<site_t>(-1, 1, 0),
            util::Vector3D<site_t>(-1, 1, 1),
            util::Vector3D<site_t>(0, -1, -1),
            util::Vector3D<site_t>(0, -1, 0),
            util::Vector3D<site_t>(0, -1, 1),
            util::Vector3D<site_t>(0, 0, -1),
            // 0 0 0 is same site
            util::Vector3D<site_t>(0, 0, 1),
            util::Vector3D<site_t>(0, 1, -1),
            util::Vector3D<site_t>(0, 1, 0),
            util::Vector3D<site_t>(0, 1, 1),
            util::Vector3D<site_t>(1, -1, -1),
            util::Vector3D<site_t>(1, -1, 0),
            util::Vector3D<site_t>(1, -1, 1),
            util::Vector3D<site_t>(1, 0, -1),
            util::Vector3D<site_t>(1, 0, 0),
            util::Vector3D<site_t>(1, 0, 1),
            util::Vector3D<site_t>(1, 1, -1),
            util::Vector3D<site_t>(1, 1, 0),
            util::Vector3D<site_t>(1, 1, 1) };

    }
  }
}

#endif // HEMELB_VIS_RAYTRACER_CLUSTERBUILDER_H