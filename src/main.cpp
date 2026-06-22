#define ASIO_STANDALONE
#include "crow.h"
#include "fixlib.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

static std::string read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return {};
    auto sz = f.tellg();
    f.seekg(0);
    std::string buf(static_cast<size_t>(sz), '\0');
    f.read(&buf[0], sz);
    return buf;
}

static std::string mime_type(const std::string& path) {
    auto ext = path.rfind('.');
    if (ext == std::string::npos) return "application/octet-stream";
    std::string e = path.substr(ext);
    if (e == ".html") return "text/html";
    if (e == ".css")  return "text/css";
    if (e == ".js")   return "application/javascript";
    if (e == ".json") return "application/json";
    if (e == ".png")  return "image/png";
    if (e == ".jpg" || e == ".jpeg") return "image/jpeg";
    if (e == ".svg")  return "image/svg+xml";
    if (e == ".ico")  return "image/x-icon";
    return "application/octet-stream";
}

int main() {
    // Pre-load the WASM bundle once at startup so Crow owns the string
    // for its full lifetime — avoids per-request large-file reads.
    std::string fixJs = read_file("/public/fix.js");

    crow::SimpleApp app;

    // GET /fix.js — WASM bundle (served from memory, not catchall)
    CROW_ROUTE(app, "/fix.js")
    ([&fixJs]() -> crow::response {
        if (fixJs.empty()) return crow::response(404);
        crow::response res(fixJs);
        res.set_header("Content-Type", "application/javascript");
        return res;
    });

    // POST /api/v1/getFix
    // Body: { "directionErrorModel": 0, "nDirections": 3,
    //         "locations": [lon1,lat1, lon2,lat2, ...],
    //         "directions": [dir1,dir2,..., sigma1,sigma2,...] }
    CROW_ROUTE(app, "/api/v1/getFix").methods("POST"_method)
    ([](const crow::request& req) -> crow::response {
        try {
            auto body    = json::parse(req.body);
            int errModel = body.value("directionErrorModel", 0);
            int nDir     = body.at("nDirections").get<int>();
            auto locs    = body.at("locations").get<std::vector<double>>();
            auto dirs    = body.at("directions").get<std::vector<double>>();

            DfAnalyzer dfa(0);
            std::string result = dfa.getFixpoint(
                static_cast<DirectionErrorModel>(errModel),
                nDir,
                locs.data(),
                dirs.data()
            );

            crow::response res(200, result);
            res.set_header("Content-Type", "application/json");
            return res;
        } catch (const std::exception& e) {
            crow::response res(400,
                std::string("{\"error\":\"") + e.what() + "\"}");
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });

    // Root → index.html
    CROW_ROUTE(app, "/")
    ([]() -> crow::response {
        std::string content = read_file("/public/index.html");
        if (content.empty()) return crow::response(404);
        crow::response res(200, content);
        res.set_header("Content-Type", "text/html");
        return res;
    });

    // All other paths → /public/<path>
    CROW_ROUTE(app, "/<path>")
    ([](const std::string& path) -> crow::response {
        std::string p = path;
        if (p.empty() || p.back() == '/') p += "index.html";
        std::string content = read_file("/public/" + p);
        if (content.empty()) return crow::response(404);
        crow::response res(200, content);
        res.set_header("Content-Type", mime_type(p));
        return res;
    });

    app.port(8080).run();
    return 0;
}
