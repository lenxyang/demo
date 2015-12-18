#pragma once

#include "azer/render/render.h"
#include "azer/render/util.h"

namespace lord {
namespace sandbox {

using azer::Effect;
using azer::EffectParamsProvider;
using azer::EffectAdapterKey;
using azer::EffectParamsAdapter;

class MaterialEffectAdapter : public EffectParamsAdapter {
 public:
  MaterialEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(MaterialEffectAdapter);
};

class SceneRenderNodeEffectAdapter : public EffectParamsAdapter {
 public:
  SceneRenderNodeEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(SceneRenderNodeEffectAdapter);
};

class SceneRenderEnvNodeEffectAdapter : public EffectParamsAdapter {
 public:
  SceneRenderEnvNodeEffectAdapter();

  EffectAdapterKey key() const override;
  void Apply(Effect* e, const EffectParamsProvider* params) const override;
 private:
  DISALLOW_COPY_AND_ASSIGN(SceneRenderEnvNodeEffectAdapter);
};
}  // namespace sandbox
}  // namespace lord
