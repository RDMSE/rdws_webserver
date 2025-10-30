#include "lambda_params_helper.h"

#include "rapidjson/document.h"

namespace rdws::utils {
    bool LambdaParamsHelper::checkParams(const int argc, char *argv[]) {
        if (argc >= 3) {
            LambdaParams lambdaParams = {
                .eventJson = argv[1],
                .contextJson = argv[2]
            };
            ::rapidjson::Document doc;
            if (doc.Parse(lambdaParams.eventJson.c_str()).HasParseError() == true) {
                return false;
            }
            if (doc.Parse(lambdaParams.contextJson.c_str()).HasParseError() == true) {
                return false;
            }
            return true;
        }
        return false;
    }

} // rdws
