#include "lambda_params_helper.h"

#include "rapidjson/document.h"

namespace rdws::utils {

    constexpr auto jsonParseError = "JSON Parse error";
    constexpr auto lambdaParamsSizeError = "Wrong number of arguments";

    tl::expected<bool, std::string> LambdaParamsHelper::checkParams(const int argc, char *argv[]) {
        if (argc >= 3) {
            const LambdaParams lambdaParams = {
                .eventJson = argv[1],
                .contextJson = argv[2]
            };
            ::rapidjson::Document doc;
            if (doc.Parse(lambdaParams.eventJson.c_str()).HasParseError() == true) {
                return tl::unexpected(jsonParseError);
            }
            if (doc.Parse(lambdaParams.contextJson.c_str()).HasParseError() == true) {
                return tl::unexpected(jsonParseError);
            }
            return true;
        }
        return tl::unexpected(lambdaParamsSizeError);
    }

} // rdws
