#pragma once

#include "azer/render/render.h"

class CameraProvider : public azer::EffectParamsProvider {
 public:
  CameraProvider(const Camera* camera) : camera_(camera) {}
  const char* GetProviderName() const override { return "EffectParamsProvider";}

  const azer::Matrix4& GetProjViewMatrix() const { 
    return camera_->GetProjViewMatrix();
  }
  const azer::Vector3& GetCameraPos() const {
    return camera_->position();
  }
 private:
  const azer::Camera* camera_;
  DISALLOW_COPY_AND_ASSIGN(CameraProvider);
};
