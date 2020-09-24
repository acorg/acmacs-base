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
                    throw error{AD_FORMAT("cannot apply: {} (string or object expected)\n", sub_entry)};
            });
        }
    }
    catch (rjson::v3::value_type_mismatch& err) {
        throw error{AD_FORMAT("cannot apply: {} : {}\n", err, entry)};
    }
    catch (std::exception& err) {
        throw error{fmt::format("{}\n    on applying {}", err, entry)};
    }

} // acmacs::settings::v3::Data::apply

// ----------------------------------------------------------------------

void acmacs::settings::v3::Data::push_and_apply(const rjson::v3::detail::object& entry)
{
    class Subenvironment
    {
      public:
        Subenvironment(detail::Environment& env, bool push) : env_{env}, push_{push}
        {
            if (push_)
                env_.push();
        }
        ~Subenvironment()
        {
            if (push_)
                env_.pop();
        }

      private:
        detail::Environment& env_;
        const bool push_;
    };

    using namespace std::string_view_literals;
    AD_LOG_INDENT;
    AD_LOG(acmacs::log::settings, "push_and_apply {}", entry);
    try {
        if (const auto& command_v = entry["N"sv]; !command_v.is_null()) {
            const auto command{command_v.to<std::string_view>()};
            AD_LOG(acmacs::log::settings, "push_and_apply command {} -> {}", command_v, command);
            Subenvironment sub_env(*environment_, command != "set");
            for (const auto& [key, val] : entry) {
                if (key != "N"sv) {
                    // AD_LOG(acmacs::log::settings, "environment_.add key:{} val:{}", key, val);
                    environment_->add(key, val);
                }
            }
            if (command != "set")
                apply(command);
            else if (warn_if_set_used_)
                AD_WARNING("\"set\" command has no effect (used inside \"if\" or \"for-each\"?): {}", entry);
        }
        else if (!entry["?N"sv].is_null() || !entry["? N"sv].is_null()) {
            // command is commented out
        }
        else
            throw error(AD_FORMAT("cannot apply (no \"N\"): {}\n", entry));
    }
    catch (rjson::v3::value_type_mismatch& err) {
        throw error(AD_FORMAT("cannot apply: {} : {}\n", err, entry));
    }

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
