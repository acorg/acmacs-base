#include "acmacs-base/settings-v3-env.hh"
#include "acmacs-base/settings-v3.hh"

// ----------------------------------------------------------------------

void acmacs::settings::v3::detail::LoadedDataFiles::load(std::string_view filename)
{
    file_data_.insert(file_data_.begin(), rjson::v3::parse_file(filename));
    filenames_.insert(filenames_.begin(), std::string{filename});

} // acmacs::settings::v3::detail::LoadedDataFiles::load

// ----------------------------------------------------------------------

void acmacs::settings::v3::detail::LoadedDataFiles::reload(Data& settings)
{
    using namespace std::string_view_literals;
    for (auto index{filenames_.size()}; index > 0; --index) {
        if (!filenames_[index - 1].empty()) {
            AD_LOG(acmacs::log::settings, "re-loading {}", filenames_[index - 1]);
            file_data_[index - 1] = rjson::v3::parse_file(filenames_[index - 1]);
        }
        if (const auto& val = file_data_[index - 1]["init"sv]; !val.is_null())
            settings.apply(val);
    }

} // acmacs::settings::v3::detail::LoadedDataFiles::reload

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
