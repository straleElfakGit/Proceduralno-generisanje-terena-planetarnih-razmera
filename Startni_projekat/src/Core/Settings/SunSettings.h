#ifndef SUN_SETTINGS_H
#define SUN_SETTINGS_H

#include <memory>
#include "Lights/DirectionalLight.h"

struct SunSettings
{
    float sunAngle = 0.0f;
    float sunVelocity = 1.5f;
    float sunHeight = 2.0f;
    float sunDistance = 10.0f;
    std::unique_ptr<DirectionalLight> lightPtr;

    float planetAngle = 0.0f;
    float planetVelocity = 0.2f;
    
    bool rotatePlanet = true;

    SunSettings() 
    {
        lightPtr = std::make_unique<DirectionalLight>(
            glm::vec3(0.2f, 0.2f, 0.25f),
            glm::vec3(1.0f, 0.95f, 0.8f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f));
    }
};

#endif // !SUN_SETTINGS_H
