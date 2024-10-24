#include "compare/Levenshtein.h"
#include "compare/LongestCommonSubsequence.h"
#include "RestController.h"
#include <boost/json.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    const int port = 8080; // This port should match with the port in the Dockerfile
    const int num_threads = 1;
    std::cout << "Server running on http://localhost:" << port << "." << std::endl;
    auto rest_controller = RestController::getInstance("/compare/index.html"); // Default Target

    rest_controller->add_routes(Method::get, "/api/hello", [](const BoostRequest& req, BoostResponse& res) {
        res.result(boost::beast::http::status::ok);
        res.set(boost::beast::http::field::content_type, "application/json");
        res.body() = R"({"message": "Welcome to the REST API", "status": "success"})";
    });

    rest_controller->add_routes(Method::get, "/status", [](const BoostRequest& req, BoostResponse& res) {
        res.result(boost::beast::http::status::ok);
        res.set(boost::beast::http::field::content_type, "text/plain");
        res.body() = "API is running smoothly";
    });

    rest_controller->add_routes(Method::post, "/compare", [](const BoostRequest& req, BoostResponse& res) {
        auto bad_request = [&res]() {
            res.result(boost::beast::http::status::bad_request);
            res.set(boost::beast::http::field::content_type, "application/json");
            res.body() = R"({"message": "Missing required fields", "status": "error"})";
        };

        try {
            boost::json::value json_body = boost::json::parse(req.body());
            boost::json::object json_obj = json_body.as_object();

            // print all elements inside the json object
            for (auto& element : json_obj) {
                std::cout << "\t" << element.key() << ": " << element.value() << std::endl;
            }

            bool elem1 = json_obj.find("str1") != json_obj.end();
            bool elem2 = json_obj.find("str2") != json_obj.end();
            if (!elem1 || !elem2) {
                bad_request();
                return;
            }

            std::string str1 = json_obj["str1"].as_string().c_str();
            std::string str2 = json_obj["str2"].as_string().c_str();

            // std::unique_ptr<Levenshtein> levenshtein = std::make_unique<Levenshtein>();
            // std::vector<Diff> diffs = levenshtein->levenshtein_distance(str1, str2);

            std::unique_ptr<LongestCommonSubsequence> lcs = std::make_unique<LongestCommonSubsequence>();
            std::vector<Diff> diffs = lcs->stringDiff(str1, str2);

            // print diffs in a single line
            std::cout << "Differences between '" << str1 << "' and '" << str2 << "':" << std::endl;
            std::cout << "[";
            boost::json::array responseArray;
            for (const auto &diff : diffs) {
                boost::json::object jsonDiffObj;
                jsonDiffObj["operation"] = diff.get_operation_string();
                jsonDiffObj["str"] = diff.get_text();
                responseArray.push_back(jsonDiffObj);
                std::cout << diff << " ";
            }
            std::cout << "]" << std::endl;
            res.result(boost::beast::http::status::ok);
            res.set(boost::beast::http::field::content_type, "application/json");

            boost::json::object body_obj;
            body_obj["result"] = responseArray;
            res.body() = boost::json::serialize(body_obj);
        } catch (const std::exception& e) {
            bad_request();
        }
    });

    try {
        rest_controller->start_server(port, num_threads);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error occurred" << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
