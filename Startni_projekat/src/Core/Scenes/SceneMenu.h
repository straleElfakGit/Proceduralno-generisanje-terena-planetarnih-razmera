#ifndef SCENE_MENU_H
#define SCENE_MENU_H

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <glad/glad.h>

#include "Scene.h"
#include "imgui/imgui.h"

class SceneMenu : public Scene
{
private:
    std::vector<std::pair<std::string, std::function<Scene*()>>> scenes;
    Scene*& currentScene;

public:
    SceneMenu(ApplicationBase* app, Scene*& currentScenePtr);
    ~SceneMenu();
    
    virtual void Start() override { }
    virtual void Update(float dt) override { }
    virtual void Render() override;

    virtual bool IsReturnable() const override { return false; }

    virtual void OnImGuiRender() override;

    Scene* GetActiveScene() const { return currentScene; }
    void ResetActiveScene() { currentScene = this; }

    template<typename T>
    void RegisterScene(const std::string& name, ApplicationBase* app)
    {
        scenes.push_back(std::make_pair(name, [app]() { return new T(app); }));
    }
};

#endif // !SCENE_MENU_H
