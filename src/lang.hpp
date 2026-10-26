#pragma once
#include <string_view>

namespace Lang {
  inline constexpr std::string_view KEYWORDS[] = {
      "if", "else", "while", "for", "return", "fn", "virtual", "override",
      "auto", "en", "exc", "overload", "apply", "patch", "struct", "class"
  };

  inline bool is_keyword(std::string_view in) {
    for (const auto& key : KEYWORDS) {
      if (in == key) {
        return true;
      }
    }
    return false;
  }
}
