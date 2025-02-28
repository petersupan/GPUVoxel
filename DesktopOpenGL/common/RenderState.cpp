#include "RenderState.hpp"

unsigned int VolumeRenderingRenderState::layerNr = 0;
float ShadowMappingRenderState::shadowMagicNumber = 0.0f;
glm::vec3 SimpleRenderState::lightPositionWorldSpace = glm::vec3(20.0, 20.0, 20.0);

glm::vec3 LandscapeImRenderState::cameraPosition = glm::vec3(0.0, -5.0, 2.0);