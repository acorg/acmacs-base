#pragma once

// ----------------------------------------------------------------------

#define DEBUG_LINE_FUNC_S (std::string{__FILE__} + ":" + std::to_string(__LINE__) + ": " + __PRETTY_FUNCTION__)
#define DEBUG_LINE_FUNC (std::string{" ["} + __FILE__ + ":" + std::to_string(__LINE__) + ": " + __PRETTY_FUNCTION__ + "]")

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
