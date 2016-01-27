#pragma once

#include "azer/render/render.h"

class CameraProvider : public azer::EffectParamsProvider {
 public:
  CameraProvider(const azer::Camera* camera) : camera_(camera) {}
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


class WorldProvider : public azer::EffectParamsProvider {
 public:
  WorldProvider() {}
  const char* GetProviderName() const override { return "WorldProvider";}

  void SetTransform(const azer::Matrix4& mat) { world_ = mat;}
  const azer::Matrix4& world() const { return world_;}
 private:
  azer::Matrix4 world_;
  DISALLOW_COPY_AND_ASSIGN(WorldProvider);
};

class LightProvider : public azer::EffectParamsProvider {
 public:
  LightProvider() {}
  const char* GetProviderName() const override { return "LightProvider";}

  void SetDirLight(const lord::DirLight& l) { dir_light_ = l;}
  void SetSpotLight(const lord::SpotLight& l) { spot_light_ = l;}
  const lord::DirLight& dir_light() const { return dir_light_;}
  const lord::SpotLight& spot_light() const { return spot_light_;}
 private:
  lord::DirLight dir_light_;
  lord::SpotLight spot_light_;
  DISALLOW_COPY_AND_ASSIGN(LightProvider);
};
