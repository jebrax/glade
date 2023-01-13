#pragma once

#include <glade/controls/Key.h>
#include <unordered_map>

namespace Glade
{
  class KeyMapper
  {
  public:
    void Remap(Glade::Key key, unsigned int action) {
      mKeyToActionMapping[key] = action;
    }

    unsigned int GetMappedAction(Glade::Key key) {
      auto entry = mKeyToActionMapping.find(key);

      if (entry == mKeyToActionMapping.end())
        return 0;

      return entry->second;
    }

  private:
    std::unordered_map<Glade::Key, unsigned int> mKeyToActionMapping;
  };
}