#include "json-importer.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

json_importer::storers::_i::Msg json_importer::storers::_i::Msg::sMsg;

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
