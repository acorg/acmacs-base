#include <iostream>
#include <cassert>
#include <cstring>

#include "argc-argv.hh"

// ----------------------------------------------------------------------

int main()
{
    int exit_code = 0;
    try {
        const char* const argv1[] = {"test-1", "-a", "--aa", "aaa", "aaaa"};
        argc_argv arg1(std::end(argv1) - std::begin(argv1), argv1, {"--aa"});
        assert(arg1.number_of_arguments() == 1);
        assert(strcmp(arg1[0], argv1[4]) == 0);
        assert(static_cast<bool>(arg1[argv1[1]]));
        assert(static_cast<bool>(arg1[argv1[2]]));
        assert(strcmp(static_cast<const char*>(arg1[argv1[2]]), argv1[3]) == 0);
        assert(static_cast<bool>(arg1["--not-found"]) == false);

        const char* const argv2[] = {"test-2", "-a", "7", "aaa", "aaaa"};
        argc_argv arg2(std::end(argv2) - std::begin(argv2), argv2, {"-a"});
        assert(arg2.number_of_arguments() == 2);
        assert(strcmp(arg2[0], argv2[3]) == 0);
        assert(strcmp(arg2[1], argv2[4]) == 0);
        assert(static_cast<bool>(arg2[argv2[1]]));
        assert(static_cast<int>(arg2[argv2[1]]) == 7);
        assert(static_cast<double>(arg2[argv2[1]]) == 7.0);
        assert(static_cast<bool>(arg2["--not-found"]) == false);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 1;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
