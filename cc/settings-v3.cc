#include "acmacs-base/settings-v3.hh"
#include "acmacs-base/settings-v3-env.hh"

// ----------------------------------------------------------------------

acmacs::settings::v3::Data::Data()
    : loaded_data_{}, environment_{}
{

} // acmacs::settings::v3::Data::Data

// ----------------------------------------------------------------------

acmacs::settings::v3::Data::~Data()
{
} // acmacs::settings::v3::Data::~Data

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::apply(std::string_view name)
{

} // acmacs::settings::v3::Data::apply

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::apply(const rjson::v3::value& entry)
{
    try {
        AD_LOG(acmacs::log::settings, "apply {}", entry);
        for (const auto& sub_entry : entry.array()) {
            sub_entry.visit([this, &sub_entry]<typename T>(const T& sub_entry_val) {
                if constexpr (std::is_same_v<T, rjson::v3::detail::string>)
                    apply(sub_entry_val.template to<std::string_view>());
                else if constexpr (std::is_same_v<T, rjson::v3::detail::object>)
                    push_and_apply(sub_entry_val);
                else if constexpr (std::is_same_v<T, rjson::v3::detail::null>)
                    ; // commented out item (using #), do nothing
                else
                    throw error{AD_FORMAT("cannot apply: {}\n", sub_entry)};
            });
        }
    }
    catch (rjson::v3::value_type_mismatch& err) {
        throw error{AD_FORMAT("cannot apply: {} : {}\n", err, entry)};
    }
    catch (std::exception& err) {
        throw error{fmt::format("{}\n    on applying {}", err.what(), entry)};
    }

} // acmacs::settings::v3::Data::apply

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::push_and_apply(const rjson::v3::detail::object& entry)
{

} // acmacs::settings::v3::Data::push_and_apply

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::load(std::string_view filename)
{
    using namespace std::string_view_literals;
    AD_LOG(acmacs::log::settings, "loading {}", filename);
    loaded_data_->load(filename);
    // apply_top("init"sv);

} // acmacs::settings::v3::Data::load

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::load(const std::vector<std::string_view>& filenames)
{

} // acmacs::settings::v3::Data::load

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::load_from_conf(const std::vector<std::string_view>& filenames) // load from ${ACMACSD_ROOT}/share/conf dir
{

} // acmacs::settings::v3::Data::load_from_conf

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::set_defines(const std::vector<std::string_view>& defines)
{

} // acmacs::settings::v3::Data::set_defines

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::reload() // reset environament, re-load previously loaded files, apply "init" in loaded files
{

} // acmacs::settings::v3::Data::reload

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
