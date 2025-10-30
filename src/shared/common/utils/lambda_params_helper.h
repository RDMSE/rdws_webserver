#pragma once
#include <string>
#include <tl/expected.hpp>

namespace rdws::utils {

    struct LambdaParams {
        std::string eventJson;
        std::string contextJson;
    };

    class LambdaParamsHelper {
    public:
        static tl::expected<bool, std::string> checkParams(int argc, char *argv[]);
    };
} // rdws

