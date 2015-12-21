#include "demo/base/effect_dict.h"

EffectDict::EffectDict() {
}

EffectDict::~EffectDict() {
}


bool EffectDict::HasEffect(const std::string& name) const {
  auto iter = dict_.find(name);
  return iter != dict_.end();
}

void EffectDict::RegisterEffect(azer::Effect* effect) {
  DCHECK(!HasEffect(effect->GetEffectName()));
  dict_.insert(std::make_pair(effect->GetEffectName(), effect));
}

azer::Effect* EffectDict::GetEffect(const std::string& name) {
  DCHECK(HasEffect(name));
  auto iter = dict_.find(name);
  return iter->second.get();
}
