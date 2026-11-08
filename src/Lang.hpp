#pragma once

// PrettySmooth - by LZ
// Translations: English (default) and Portuguese.

#include <string>
#include <string_view>

namespace ps {
    // Returns the text for `key` in the current language (falls back to the key).
    std::string const& tr(std::string_view key);
    bool isPortuguese();
}
