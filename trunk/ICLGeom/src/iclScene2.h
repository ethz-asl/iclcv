#ifndef ICL_SCENE_2_H
#define ICL_SCENE_2_H

#include <iclObject2.h>
#include <iclCamera.h>
#include <iclImg.h>
#include <iclMouseHandler.h>
#include <iclDrawWidget3D.h>
#include <iclLockable.h>

namespace icl{
  
  /** \cond */
  class ICLDrawWidget;
  /** \endcond */

  class Scene2 : public Lockable{
    public:
    struct RenderPlugin;
    struct SceneMouseHandler;
    struct GLCallback;
    
    Scene2();
    ~Scene2();
    Scene2(const Scene2 &scene);
    Scene2 &operator=(const Scene2 &scene);
    
    void addCamera(const Camera &cam);
    void removeCamera(int index);
    Camera &getCamera(int camIndex = 0);
    const Camera &getCamera(int camIndex =0) const;

    void render(Img32f &image, int camIndex = 0);
    void render(ICLDrawWidget &widget, int camIndex = 0);

    /// renders the scene into current OpenGL context
    void render(int camIndex=0);

    
    void addObject(Object2 *object);
    void removeObject(int idx, bool deleteObject = true);
    
    void clear(bool camerasToo=false);
    
    MouseHandler *getMouseHandler(int camIndex=0);

    ICLDrawWidget3D::GLCallback *getGLCallback(int camIndex);

    private:

    void render(RenderPlugin &p, int camIndex);
    
    std::vector<Camera> m_cameras;
    std::vector<Object2*> m_objects;
    std::vector<std::vector<std::vector<Vec> > >m_projections;//[cam][obj][vertex]
    
    std::vector<SceneMouseHandler*> m_mouseHandlers;
    std::vector<GLCallback*> m_glCallbacks;
  };
}

#endif
