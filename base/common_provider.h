#pragma once

#include "azer/render/render.h"
#include "lordaeron/effect/light.h"

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
  WorldProvider() : world_(azer::Matrix4::kIdentity) {}
  const char* GetProviderName() const override { return "WorldProvider";}

  void SetWorld(const azer::Matrix4& mat) { world_ = mat;}
  const azer::Matrix4& world() const { return world_; }
 private:
  mutable azer::Matrix4 world_;
  DISALLOW_COPY_AND_ASSIGN(WorldProvider);
};

class TransformProvider : public azer::EffectParamsProvider {
 public:
  TransformProvider() : world_(azer::Matrix4::kIdentity) {}
  const char* GetProviderName() const override { return "TransformProvider";}

  azer::TransformHolder* mutable_holder() { return &holder_;}
  const azer::Matrix4& world() const { 
    world_ = std::move(holder_.GenWorldMatrix());
    return world_;
  }
 private:
  mutable azer::Matrix4 world_;
  azer::TransformHolder holder_;
  DISALLOW_COPY_AND_ASSIGN(TransformProvider);
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
