/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/GenericSurfDetector.h            **
** Module : ICLMath                                          **
** Authors: Christian Groszewski, Christof Elbrechter              **
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
#ifndef ICL_GENERICSURFDETECTOR_H_
#define ICL_GENERICSURFDETECTOR_H_

#include <ICLCore/ImgBase.h>
#include <ICLUtils/SmartPtr.h>
#ifdef HAVE_QT
#include <ICLQt/DrawWidget.h>
#endif

namespace icl{
/**
 This class is a surf detector. It uses OpenCVSurfDetector or OpenSurfDetector
 to detect feature and/or matches between them.*/
class GenericSurfDetector{
private :

	///Forwarddeklaration.
	class Data;
	///Class for internal params and buffers.
	Data *m_data;
public :

	///Constructs an object of this class
	/** All default parameters are set. Use setters and getters for desired values.
            @param impl kind of the implementation (opencv or opensurf)
	 */
	GenericSurfDetector(const std::string &impl="opensurf");

	///Constructs an object of this class (opencv)
	/** Uses the OpenCVSurfDetector for detection  of features.
            @param obj the refence image
            @param threshold
            @param extended
            @param octaves
            @param octavelayer
	 */
	GenericSurfDetector(const ImgBase *obj=0,double threshold=500, int extended=1,
			int octaves=3, int octavelayer=4);

	///Constructs an object of this class
	/** Uses OpenSurfDetector for detection of features.
            @param obj the reference image
            @param upright rotationinvariance
            @param octaves number of octaves
            @param intervals number of intervals
            @param init_samples number of init samples
            @param thresh threshold
        */
	GenericSurfDetector(const ImgBase *obj=0,  bool upright=false, int octaves=4,
			int intervals=4, int init_samples=2, float thresh=0.004f);

	///Destructor
	~GenericSurfDetector();

	///Returns the kind of implementation used
	/** @return implementation */
	const std::string &getImpl();

	///pure virtual generic point structure that generalizes CvSURFPoint and Ipoint
        /** This structure is implemented for both underlying types opencv and opensurf.
            The Impl class Itself is not used itself. Instead the GenericPoint hull-class
            is used, which manages a shared pointer to the underlying Impl-instance.
        */
	struct GenericPointImpl{
          /// Destructor
          virtual ~GenericPointImpl();
          
          /// Which backend was used (either "opencv" or "opensurf")
          virtual const std::string &getBackend() const;
          
          /// Feature center
          virtual Point32f getCenter() const;
          
          /// Feature radius
          virtual int getRadius() const;
          
          /// Feature laplacian
          virtual int getLaplacian() const;
          
          /// Featur angle / direction
          virtual float getDir() const;

          /// feature scale (opensurf only)
          virtual float getScale() const;

          /// feature descriptor vector (opensurf only)
          virtual const float* getDescriptor() const;

          /// feature's dx (opensurf only)
          virtual float getDx() const;

          /// feature's dy (opensurf only)
          virtual float getDy() const;

          /// feature's cluster-index (opensurf only)
          virtual int getClusterIndex() const;
          
          /// feature size (opencv only)
          virtual int getSize() const;

          /// feature's hessian (opencv only)
          virtual float getHessian() const;
	};

	///Generic Point structure, that is used for the GenericSurfDetector
        /** Internally, the SmartPtr 'impl' is set up with either an OpenCV or
            an OpenSurf specific GenericPointImpl implementation. Some of the
            class methods are available for both implementations. However some
            other methods (those that throw and ICLException) are only available
            for either an opencv or an opensurf backend. */
	struct GenericPoint{
          ///Smartpointer holding the implementation of the point
          SmartPtr<GenericPointImpl> impl;
          ///Contructor
          GenericPoint(){}

          ///Constructor
          /** @param i implementation of GenericPointImpl (OpenCVGenericPointImpl or OpenSurfGenericPointImpl)*/
          GenericPoint(GenericPointImpl* i):impl(i){}

          ///Destructor
          virtual ~GenericPoint(){};
          ///Returns the center of the point
          /** @return center of the point */
          Point32f getCenter() const { return impl->getCenter(); }
          
          ///Returns the radius of the point
          /** @return the radius of the point */
          int getRadius() const { return impl->getRadius(); }
          
          ///Returns the laplacian of the point (feature)
          /** @return the laplacian of the feature */
          int getLaplacian() const {return impl->getLaplacian();}
          
          ///Returns the direction of the feature
          /** @return direction of feature */
          float getDir() const {return impl->getDir();}
          
          ///Returns the scale of the feature (opensurf only)
          /** Using this function on OpenCVGenericPointImpl, forces an Exception.
              @return the scale */
          virtual float getScale() const throw (ICLException){ return impl->getScale(); }

          ///Return the descriptor of the feature (opensurf only)
          /** An exception is thrown using this on OpenCVGenericPointImpl.
              @return the descriptor of the feature
              */
          const float* getDescriptor() const throw (ICLException){ return impl->getDescriptor(); }

          ///Return the pointmotion of the feature (opensurf only)
          /** An exception is thrown using this on OpenCVGenericPointImpl.
              @return pointmotion of the feature
          */
          float getDx() const throw (ICLException){ return impl->getDx(); }

          
          ///Return the pointmotion of the feature (opensurf only)
          /** An exception is thrown using this on OpenCVGenericPointImpl.
              @return pointmotion of the feature
          */
          float getDy() const throw (ICLException){ return impl->getDy(); }

          ///Return the clusterindex of the feature (opensurf only)
          /** An exception is thrown using this on OpenCVGenericPointImpl.
              @return clusterindex of the feature
          */
          int getClusterIndex() const throw (ICLException){ return impl->getClusterIndex(); }
          
          //OpenCV only
          ///Return the size of the feature (opencv only)
          /** An exception is thrown using this on OpenSurfGenericPointImpl.
              @return size of the feature
          */
          int getSize() const throw (ICLException){ return impl->getSize(); }

          ///Return the hessian of the feature (opencv only)
          /** An exception is thrown using this on OpenSurfGenericPointImpl.
              @return hessian of the feature */
          float getHessian() const throw (ICLException){ return impl->getHessian(); }
	};


	///Sets a new reference image (and automatically extract surf-features from that image internally)
	/** @param objectImg the new reference image */
	void setObjectImg(const ImgBase *objectImg) throw (ICLException);

	/// returns back converted image. 
	/** @return copy of reference image */
	SmartPtr<ImgBase> getObjectImg() throw (ICLException);

	///Return the features of the reference image
	/** If the reference image is not set, the returned vector is empty
            @return features of the reference image */
	const std::vector<GenericPoint> &getObjectImgFeatures();

	///Sets wether Rotationinvariant or not (opensurf only)
	/** Throws Exception if opencv is used, since this function is only for opencv.
	  @param upright */
	void setRotationInvariant(bool upright) throw (ICLException);

	///Sets the number of octaves
	/** @param octaves the number of octaves */
	void setOctaves(int octaves);

	///Sets the number of octavelayer
	/** @param octavelayer the number of octavelayer */
	void setOctavelayer(int octavelayer);

	///Sets the initsamples (opensurf only)
	/** Throws Exception if opencv is used, since this function is only for opencv.
            @param init_samples number of initsamples */
	void setInitSamples(int init_samplex) throw (ICLException);

	///Sets the threshold
	/** @param threshold */
	void setThreshold(double threshold);

	///Sets wether extended or not (opencv only)
	/** Throws Exc eption if opensurf is used, since this function is only for opencv.
            @param extended */
	void setExtended(int extended) throw (ICLException);

	///Returns wether rotationinvariant or not (opensurf only)
	/** Throws Exception if opencv is used, since this function is only for opensurf.
            @return rotationsinvariance */
	bool getRotationInvariant() throw (ICLException);

	///Returns the number of octaves
	/** @return number of octaves */
	int getOctaves();

	///Returns the number of octavelayer
	/** @return number of octavelayer */
	int getOctavelayer();

	///Returns the number of initsamples (opensurf only)
	/** Throws Exception if opencv is used, since this function is only for opensurf.
            @return number of initsamples */
	int getInitSamples() throw (ICLException);

	///return the threshold value
	/** @return threshold */
	double getThreshold();

	///Returns wether extended or not
	/** @return extended */
	int getExtended() throw (ICLException);

	///sets desired parameters (OPENCV only)
	/** Sets desired parameters and computes surf, but only if the
            parameters have really changed and if a reference image is
            available.
            @param threshold
            @param extended
            @param octaves
            @param octavelayer */
	void setParams(double threshold, int extended,
			int octaves, int octavelayer);

	///sets desired parameters (OPENSURF only)
	/** Sets desired parameters and computes surf, but only if the
            parameters have really changed and if a reference image is
            available.
            @param upright rotationinvariance
            @param octaves
            @param intervals
            @param init_samples
            @param thresh */
	void setParams(bool upright, int octaves, int intervals,
			int init_samples,float thresh);

	///extracs surf from given image
	/** Extracts surf from given image. If the image is null
            an empty vector is returned.
            @param src source image
            @return vector of computed features */
	const std::vector<GenericPoint> &extractFeatures(const ImgBase *src) throw (ICLException);

        /// Match Data-Type (just a shortcut)
        typedef std::pair<GenericPoint, GenericPoint> Match;

	///computes matches between reference image and passed image.
	/** If the passed image or the reference image is null an
            exception is thrown. First point of the pair is the point for the image,
            second for the reference image.
            @param image
            @return vector of std::pair */
	const std::vector<Match> &match(const ImgBase *image) throw (ICLException);

        
	///sets new newObjImage as new reference image and computes matches between passed reference image and passed image.
	/** If one of the passed images null an exception is thrown.
	  First point of the pair is the point for the image, second for the reference image.
	  @param image
	  @param newObjImage the new reference image
	  @return matches as std::pair in vector */
	const std::vector<Match> &match(const ImgBase *currentImage,const ImgBase *newObjImage) throw (ICLException){
          setObjectImg(newObjImage);
          return match(currentImage);
	}

#ifdef HAVE_QT
	///draws a point on a widget
	/** @param w drawwidget
            @param p point to be drawn */
	static void visualizeFeature(ICLDrawWidget &w,const GenericPoint &p);

	///draws all points in vector on a widget
	/** @param w drawwidget
            @param features vector of points to be drawn */
	static void visualizeFeatures(ICLDrawWidget &w, const std::vector<GenericPoint> &features);

	///draws matches on widgets
	/** @param w_object drawwidget for the reference image
            @param w_result drawwidget for image
            @param matches vector of matches to be drawn */
	static void visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result,
			const std::vector<std::pair<GenericPoint, GenericPoint> > &matches);
#endif
};
}

#endif /* ICL_GENERICSURFDETECTOR_H_ */
