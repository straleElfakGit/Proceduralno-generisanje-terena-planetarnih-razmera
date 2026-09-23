#ifndef SCENE_H
#define SCENE_H

class ApplicationBase;

class Scene {
protected:
    ApplicationBase* app;

public:
    Scene(ApplicationBase* app) : app(app) {}
    virtual ~Scene() {};

    virtual bool IsReturnable() const { return true; }
    virtual bool HasImGui() const { return true; }

    virtual void Start() = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;

    virtual void OnImGuiRender() { }

    virtual void OnScroll(double xoffset, double yoffset) {};
};

#endif // SCENE_H