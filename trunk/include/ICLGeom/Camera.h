/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/Camera.h                               **
** Module : ICLGeom                                                **
** Authors: Erik Weitnauer, Christof Elbrechter                    **
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

#ifndef ICL_MY_CAMERA_H
#define ICL_MY_CAMERA_H

#include <ICLUtils/Size.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Rect.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Array2D.h>
#include <ICLGeom/PlaneEquation.h>
#include <ICLGeom/ViewRay.h>


// the icl namespace
namespace icl {
  /// Sophisticated Camera class
  /** This camera class implements a model of a central projection camera with
      finite focal length. It is very general and can be applied to most cameras,
      e.g. CCD cameras. Because it assumes a linear projection, any distortion in
      the camera image should be corrected before using it in this class.

      The projection which describes the camera can be characterized by the
      following parameters.
      - External Parameters (position of camera in world space)
        - <b>pos</b> the camera position vector.
        - <b>norm</b> the image plane's normal vector (sometimes called the view-vector)
          the norm vector is directed from the camera center to the scene.
        - <b>up</b> which defines the "roll"-angle of the camera. It points into
          the positive y-direction of the image-plane (which means that it will
          normally, despite its name, point from the center of the camera towards
          its bottom side) and is perpendicular to the norm vector.
        - <b>horiz</b> the horizontal vector pointing to the positive x-direction of the
          image plane is computed based on norm and up vector. It forms a right-handed
          coordinate system together with them.
      - Internal Parameters
        - <b>focal length</b> is the distance between the lense and the image plane
          of the camera
        - <b>mx, my</b> sampling resolution on the camera image. In case of CCD
          cameras this is the resolution of the sensor chip in [pixel/mm]
        - <b>px, py</b> the offset between the center of the image plane and the
          principal point of the camera, in [pixel]
        - <b>skew</b> this parameter is zero, when the x- and y-axis of the
          image plane are perpendicular to each other. This should normally be
          the case.

      These parameters can also be estimated by using a number of point references
      between points with known position in the world coord. sys. and their
      corresponding projections on the image plane.

      Additionally to calculating the projection of points in the world coord. sys.
      onto the image plane and the back-projection of image points onto view rays
      in the world coord. sys., the camera class can also be used to render scenes
      on the screen. In order to render it on the screen, the size of the camera
      chip and the distance to the far clipping plane must be specified, so that
      the image can be clipped. Also, the position and size of the viewport (the rectangle on the screen in which the
      camera image should be rendered) in x, y and z coordinates must be specified.
      - Rendering Parameters
        - <b>chip size</b> in [pixel] used for transformation to clip coordinates
        - <b>zFar</b> in [mm] position of the far clipping plane
        - <b>viewport size</b> in [pixel]
        - <b>zMin and zMax</b> for the viewport
  */

  class Camera {
    public:

    struct RenderParams {
      Size chipSize;    //!< chip size in [mm] for transformation to clip coordinates
      float clipZNear;  //!< position of the near clipping plane in [mm]
      float clipZFar;   //!< position of the far clipping plane in [mm]
      Rect viewport;    //!< in [pixel]
      float viewportZMin, viewportZMax; //!< interval for the z coordinates in viewport

      RenderParams(const Size &chipSize=Size::VGA, float clipZNear=0., float clipZFar=10000.,
        const Rect &viewport=Rect(0,0,640,480), float viewportZMin=0.,
        float viewportZMax=1.):
        chipSize(chipSize), clipZNear(clipZNear), clipZFar(clipZFar), viewport(viewport),
        viewportZMin(viewportZMin), viewportZMax(viewportZMax) {}
    };

    /// We need at least 6 Data points in general positions
    struct NotEnoughDataPointsException  : public ICLException{
      NotEnoughDataPointsException():ICLException(__FUNCTION__){}
    };

    /// internal typedef
    typedef FixedMatrix<icl32f,3,3> Mat3x3;

    /** @{ @name constructors */

    /// basic constructor that gets all possible parameters
    Camera(const Vec &pos=Vec(0,0,10,1),
             const Vec &norm=Vec(0,0,-1,1),
             const Vec &up=Vec(1,0,0,1),
             float f=3,
             const Point32f &principalPointOffset=Point32f(320,240),
             float sampling_res_x = 200,
             float sampling_res_y = 200,
             float skew = 0,
             const RenderParams &renderParams = RenderParams()): m_pos(pos), m_norm(norm), m_up(up), m_f(f),
           m_px(principalPointOffset.x), m_py(principalPointOffset.y),
           m_mx(sampling_res_x), m_my(sampling_res_y), m_skew(skew), m_renderParams(renderParams) {}

    /// loads a camera from given file
    /** @param filename file name of valid configuration file (in ICL's ConfigFile format)
        @param prefix valid prefix that determines wheret to find the camera within the
               given config file (note, that this prefix must end with '.')
        */
    Camera(const std::string &filename, const std::string &prefix="config.")  throw (ParseException);
    /// loads a camera from given input stream
    /** @param filename file name of valid configuration file (in ICL's ConfigFile format)
        @param prefix valid prefix that determines wheret to find the camera within the
               given config file (note, that this prefix must end with '.')
        */
    Camera(std::istream &configDataStream, const std::string &prefix="config.")  throw (ParseException);


    /** @} @{ @name static creation functions */
    /// Compute all camera parameters from the 4x3 projection matrix.
    static Camera createFromProjectionMatrix(const FixedMatrix<icl32f,4,3> &Q, float focalLength=1);

    /// Uses the passed world point -- image point references to estimate the projection parameters.
    /** At least 6 data points references are needed. It is not possible to estimate the
        focal length f directly, but only the products f*m_x and f*m_y (which is sufficient for
        defining the projection). Therefore an arbitrary value for f != 0 may be passed to the
        function.
        The method minimizes the algebraic error with the direct linear transform algorithm in
        which a SVD is used.
        If IPP is not available, this function uses calibrate_pinv(std::vector<Vec>,std::vector<Point32f>,float)
    */
    static Camera calibrate(std::vector<Vec> Xws, std::vector<Point32f> xis, float focalLength=1)
    throw (NotEnoughDataPointsException,SingularMatrixException);

    /// Uses the passed world point -- image point references to estimate the projection parameters.
    /** Same as the method calibrate, but using a pseudoinvers instead of the SVD for the estimation.
        This method is less stable and less exact. */
    static Camera calibrate_pinv(std::vector<Vec> Xws, std::vector<Point32f> xis, float focalLength=1)
    throw (NotEnoughDataPointsException,SingularMatrixException);

    /** @} @{ @name projection functions */

    // projections normal
    /// Project a world point onto the image plane. Caution: Set last component of world points to 1.
    Point32f project(const Vec &Xw) const;
    /// Project a vector of world points onto the image plane.  Caution: Set last component of world points to 1.
    void project(const std::vector<Vec> &Xws, std::vector<Point32f> &dst) const;
    /// Project a vector of world points onto the image plane. Caution: Set last component of world points to 1.
    const std::vector<Point32f> project(const std::vector<Vec> &Xws) const;



    // projections OpenGL
    /// Project a world point onto the image plane.
    Vec projectGL(const Vec &Xw) const;
    /// Project a vector of world points onto the image plane.
    void projectGL(const std::vector<Vec> &Xws, std::vector<Vec> &dst) const;
    /// Project a vector of world points onto the image plane.
    const std::vector<Vec> projectGL(const std::vector<Vec> &Xws) const;


    // projection magic
    /// Returns a view-ray equation of given pixel location
    ViewRay getViewRay(const Point32f &pixel) const;

    /// returns a list of viewrays corresponding to a given set of pixels
    /** This method is much faster than using getViewRay several times since the
        projection matrix inversion that is necessary must only be done once */
    std::vector<ViewRay> getViewRays(const std::vector<Point32f> &pixels) const;

    /// returns a 2D array of all viewrays
    /** This method is much faster than using getViewRay several times since the
        projection matrix inversion that is necessary must only be done once */
    Array2D<ViewRay> getAllViewRays() const;
    
    /// Returns a view-ray equation of given point in the world
    ViewRay getViewRay(const Vec &Xw) const;
    /// returns estimated 3D point for given pixel and plane equation
    Vec estimate3DPosition(const Point32f &pixel, const PlaneEquation &plane) const throw (ICLException);
    /// calculates the intersection point between this view ray and a given plane
    /** Throws an ICLException in case of parallel plane and line
        A ViewRay is defined by  \f$V: \mbox{offset} + \lambda \cdot \mbox{direction} \f$
        A Plane is given by \f$ P: < (X - \mbox{planeOffset}), \mbox{planeNormal}> = 0 \f$
        Intersection is described by
        \f$<(\mbox{offset} + \lambda \cdot \mbox{direction}) - \mbox{planeOffset},planeNormal> = 0\f$
        which yields:
        \f[ \lambda = - \frac{<\mbox{offset}-\mbox{planeOffset},\mbox{planeNormal}>}{<\mbox{direction},\mbox{planeNormal}>} \f]
        and .. obviously, we get no intersection if direction is parallel to planeNormal
    */
    static Vec getIntersection(const ViewRay &v,
                               const PlaneEquation &plane) throw (ICLException);

    /** @} @{ @name complex setter and getter functions */

    // setters and getters
    /// set the norm and up vectors according to the passed rotation matrix
    void setRotation(const Mat3x3 &rot);
    /// set norm and up vectors according to the passed yaw, pitch and roll
    void setRotation(const Vec &rot);

    /// get world to image coordinate system transformation matrix
    Mat getCSTransformationMatrix() const;
    /// get world to image coordinate system transformation matrix
    Mat getCSTransformationMatrixGL() const;
    /// get projection matrix
    Mat getProjectionMatrix() const;
    /// get the projection matrix as expected by OpenGL
    Mat getProjectionMatrixGL() const;
    Mat getViewportMatrixGL() const;

    /// returns the common 4x3 camera matrix
    FixedMatrix<icl32f,4,3> getQMatrix() const;
    
    /// returns the inverse QMatrix
    FixedMatrix<icl32f,3,4> getInvQMatrix() const;

    /// translates the current position vector
    inline void translate(const Vec &d) { m_pos += d; }

    std::string toString() const;

    /** @} @{ @name simple getter functions */

    const std::string &getName() const { return m_name; }
    const Vec &getPosition() const { return m_pos; }
    const Vec &getNorm() const { return m_norm; }
    const Vec &getUp() const { return m_up; }
    Vec getHoriz() const { return cross(m_up, m_norm); }
    float getFocalLength() const { return m_f; }
    Point32f getPrincipalPointOffset() const { return Point32f(m_px, m_py); }
    float getPrincipalPointOffsetX() const { return m_px; }
    float getPrincipalPointOffsetY() const { return m_py; }
    float getSamplingResolutionX() const { return m_mx; }
    float getSamplingResolutionY() const { return m_my; }
    float getSkew() const { return m_skew; }
    const RenderParams &getRenderParams() const { return m_renderParams; }
    RenderParams &getRenderParams() { return m_renderParams; }

    /** @} @{ @name simple setter functions */

    void setName(const std::string &name) { m_name = name; }
    void setPosition(const Vec &pos) { m_pos = pos; }
    void setNorm(const Vec &norm) { m_norm = norm; m_norm[3] = 0; m_norm.normalize(); m_norm[3] = 1; } //!< gets automatically normalized
    void setUp(const Vec &up) { m_up = up; m_up[3] = 0; m_up.normalize(); m_up[3] = 1; } //!< gets automatically normalized
    void setFocalLength(float value) { m_f = value; }
    void setPrincipalPointOffset(float px, float py) { m_px = px; m_py = py; }
    void setPrincipalPointOffset(const Point32f &p) { m_px = p.x; m_py = p.y; }
    void setSamplingResolutionX(float value) { m_mx = value; }
    void setSamplingResolutionY(float value) { m_my = value; }
    void setSamplingResolution(float x, float y) { m_mx = x; m_my = y; }
    void setSkew(float value) { m_skew = value; }
    void setRenderParams(const RenderParams &rp) { m_renderParams = rp; }


    /** @} @{ @name more complex manipulation functions */

    /// Changes the camera resolution and adapts dependent values
    /** Internally, this function also adapts the render parameters chipSize and viewport
        Furthermore, the prinizipal-point-offset is automatically set to the center of
        the new screen
    */
    void setResolution(const Size &newScreenSize);

    /// Changes the camera resolution and adapts dependent values
    /** Internally, this function also adapts the render parameters chipSize and viewport
        Furthermore, the prinizipal-point-offset is set to the new given value
    */
    void setResolution(const Size &newScreenSize, const Point &newPrincipalPointOffset);

    /** @} @{ @name 3D-position estimation */

    /// computes the 3D position of a n view from n cameras
    /** @param cams list of cameras
        @param UVs list of image points (in image coordinates)
        @param removeInvalidPoints if this flag is set to true,
               all given points are checked to be within the cameras
               viewport. If not, these points are removed internally.

        <h3> Method: </h3>

        This function uses a standard linear approach using a pseudo-inverse to
        solve the problem in a "least-square"-manner:

        The camera is essentially represented by the 3x4-Q-Matrix, which can be
        obtained using icl::Camera::getQMatrix() const. Q is defined as follows:

        <pre>
                    | -- x --  tx |
        Q = [R|t] = | -- y --  ty |
                    | -- z --  tz |
        </pre>

        The camera projection is trivial now. For a given projected point [u,v]'
        (in image coordinates) and a position in the world Pw (homogeneous):

        <pre>
        [u,v,1*]' = hom( Q Pw )
        </pre>

        Where '1*' becomes a real 1.0 just because of the homogenization step
        using hom(). Component-wise, this can be re-written as:

        <pre>
        u  = x Pw + tx
        v  = y Pw + ty
        1* = z Pw + tz
        </pre>

        In order to ensure, '1*' becomes a real 1.0, the upper two equations
        have to be devided by (z Pw + tz), which provides us the following two
        equations:

        <pre>
        u = (x Pw + tx) / (z Pw + tz)
        v = (y Pw + ty) / (z Pw + tz)
        </pre>

        These equations have to be reorganized so that Pw can be factored out:

        <pre>
        (u z - x) Pw = tx - u tz
        (v z - y) Pw = ty - v tz
        </pre>

        Now we can write this in matrix notation again:

        <pre>
        A Pw = B      ,where


        A = | u z - x |
            | v z - y |

        B = | tx - u tz |
            | ty - v tz |

        </pre>

        The obviously under-determined equation-system above uses only a single camera. If
        we put the results from several views together into a single equation system, it
        becomes unambigoulsly solvable using a pseudo-inverse approach:

        <pre>
        | A1 |      | B1 |             | A1 |+ | B1 |
        | A2 | Pw = | B2 |    => Pw =  | A2 |  | B2 |
        |....|      |....|             |....|  |....|
        </pre>

        where 'A+' means the pseudo-inverse of A
    */
    static Vec estimate_3D(const std::vector<Camera*> cams,
                           const std::vector<Point32f> &UVs,
                           bool removeInvalidPoints=false) throw (ICLException);

    /// multiview 3D point estimation using svd-based linear optimization (should not be used)
    /** This functions seems to provide false results for more than 2 views:
        use estimate_3D instead*/
    static Vec estimate_3D_svd(const std::vector<Camera*> cams,
                               const std::vector<Point32f> &UVs);

    /** @}*/

    protected:
    static Mat createTransformationMatrix(const Vec &norm, const Vec &up, const Vec &pos);

    private:
    // General Parameters
    std::string m_name;   //!< name of the camera (visualized in the scene if set)

    // External Parameters
    Vec m_pos;        //!< center position vector
    Vec m_norm;       //!< normal vector of image plane
    Vec m_up;         //!< vector pointing to pos. y axis on image plane

    // Internal Parameters
    float m_f;        //!< focal length
    float m_px, m_py; //!< principal point offset
    float m_mx, m_my; //!< sampling resolution
    float m_skew;     //!< skew parameter in the camera projection, should be zero

    // Rendering Parameters
    RenderParams m_renderParams;

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    /// internally used utility function
    static void checkAndFixPoints(std::vector<Vec> &worldPoints, std::vector<Point32f> &imagePoints)
      throw (NotEnoughDataPointsException);
    /// intenal helper function
    static void load_camera_from_stream(std::istream &is,
                                        const std::string &prefix,
                                        Camera &cam);
  };

  /// ostream operator (writes camera in XML format)
  std::ostream &operator<<(std::ostream &os, const Camera &cam);

  /// istream operator parses a camera from an XML-string
  std::istream &operator>>(std::istream &is, Camera &cam) throw (ParseException);

} // namespace icl

#endif
