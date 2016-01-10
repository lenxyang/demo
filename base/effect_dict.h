#pragma once

#include <map>
#include <string>
#include "azer/render/render.h"

class EffectDict {
 public:
  EffectDict();
  ~EffectDict();

  bool HasEffect(const std::string& name) const;
  void RegisterEffect(azer::Effect* effect);
  azer::Effect* GetEffect(const std::string& name);
 private:
  std::map<std::string, azer::EffectPtr> dict_;
  DISALLOW_COPY_AND_ASSIGN(EffectDict);
};
