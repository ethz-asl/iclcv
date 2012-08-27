/*********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLMath/KDTree.h                               **
 ** Module : ICLMath                                                **
 ** Authors: Christian Groszewski                                   **
 **                                                                 **
 **                                                                 **
 ** Commercial License                                              **
 ** ICL can be used commercially, please refer to our website       **
 ** www.iclcv.org for more details.                                 **
 **                                                                 **
 ** GNU General Public License Usage                                **
 ** Alternatively, this file may be used under the terms of the     **
 ** GNU General Public License version 3.0 as published by the      **
 ** Free Software Foundation and appearing in the file LICENSE.GPL  **
 ** included in the packaging of this file.  Please review the      **
 ** following information to ensure the GNU General Public License  **
 ** version 3.0 requirements will be met:                           **
 ** http://www.gnu.org/copyleft/gpl.html.                           **
 **                                                                 **
 ** The development of this software was supported by the           **
 ** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
 ** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
 ** Forschungsgemeinschaft (DFG) in the context of the German       **
 ** Excellence Initiative.                                          **
 **                                                                 **
 *********************************************************************/
#ifndef ICL_KDTREE_H_
#define ICL_KDTREE_H_

#include <ICLMath/DynMatrix.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Uncopyable.h>
#include <vector>

namespace icl{
  namespace math{
  
    /// Simple KD-Tree implementation
    /** This class implements a simple kd-tree. You can create an object of this class
        with or without given point data. After creating a kd-tree without data you can use
        the buildTree method to insert data points or to rebuild the kd-tree with new
        data points. 
        <b>Note:</b> If point data was passed to the KD-Tree constructor, it only references
        by the KD-Tree instance. Therefore, that tree instance will only stay valid as long as
        the referenced data does
    */
    class KDTree : public Uncopyable{
      private:
      ///Keeps data of node
      struct Node{
        ///left node
        Node *left;
        ///right node
        Node *right;
        ///point in leafnode, else null
        utils::DynMatrix<icl64f> *point;
        ///median of dimension
        double median;
        
        ///Constructor
        inline Node():left(0),right(0),point(0),median(0.0){}
        
        ///Destructor
        inline ~Node(){
          if(right != 0){
            delete right;
            right = 0;
          }
          if(left != 0){
            delete left;
            left = 0;
          }
          
        }
      };
      
      ///the root node of the tree
      Node root;
      
      ///internal print call
      void print(Node *node);
      ///internal call to fill the KDTree with data
      void buildTree(std::vector<utils::DynMatrix<icl64f> > &list,unsigned int depth, Node *node);
      ///internal call to fill the KDTree with data
      void buildTree(std::vector<utils::DynMatrix<icl64f> *> &list,unsigned int depth, Node *node);
      ///internal call to sort list by dimension of the vector (unused, instead std::sort)
      void sortList(std::vector<utils::DynMatrix<icl64f> > &list, unsigned int dim);
      ///internal call to sort list by dimension of the vector (unused, instead std::sort)
      void sortList(std::vector<utils::DynMatrix<icl64f>* > &list, unsigned int dim);
      ///internal call to release data from KDTree
      void releaseTree();
      
      public :
      ///Constructor
      /** Creates a new KDTree object, with data from list.
       * @param list list of points for  the kd-tree
       */
      KDTree(std::vector<utils::DynMatrix<icl64f> > &list);
  
      ///Constructor
      /** Creates a new KDTree object, with data from list.
       * @param list list of points for  the kd-tree
       */
      KDTree(std::vector<utils::DynMatrix<icl64f>* > &list);
  
      ///Constructor
      /** Creates a new KDTree object, with data from list.
       * @param list list of points for  the kd-tree
       */
      KDTree(){}
     
      ///Destructor
      ~KDTree();
      
      ///builds a kd-tree
      /** Fills empty kd-tree or the current one with new data.
       * @param list list of points for the kd-tree
       */
      inline void buildTree(std::vector<utils::DynMatrix<icl64f> *> &list){
        releaseTree();
        buildTree(list,0,&root);
      }
      
      ///builds a kd-tree
      /** Fills empty KDTree object or the current one with new data.
       * @param list list of points for the kd-tree
       */
      inline void buildTree(std::vector<utils::DynMatrix<icl64f> > &list){
        releaseTree();
        buildTree(list,0,&root);
      }
      
      ///Prints the tree on standard output.
      void print();
      
      ///Returns pointer to nearest neighbour to passed point.
      /** @param point the point to search nearest neighbor for
       *  @return the pointer to nearest neighbour */
      utils::DynMatrix<icl64f>* nearestNeighbour(const utils::DynMatrix<icl64f> &point);
      
      ///Returns pointer to nearest neighbour to passed point.
      /** @param point the point to search nearest neighbor for
       *  @return the pointer to nearest neighbour */
      utils::DynMatrix<icl64f>* nearestNeighbour(const utils::DynMatrix<icl64f> *point);
      
    };
  } // namespace utils
}
#endif /* ICL_KDTREE_H_ */

