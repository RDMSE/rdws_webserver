#pragma once
#include <string>

namespace rdws::utils {

    struct LambdaParams {
        std::string eventJson;
        std::string contextJson;
    };

    class LambdaParamsHelper {
    public:
        static bool checkParams(const int argc, char *argv[]);

    };
} // rdws

