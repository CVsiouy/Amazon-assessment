#include "returns/file_io.hpp"
#include "returns/parser.hpp"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace returns {

namespace {

/// Trim leading and trailing whitespace.
std::string trim(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        start++;
    }
    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        end--;
    }
    return value.substr(pos: start, n: end - start);
}

/// Escape UTF-8 string content for JSON output.
std::string json_escape(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (char c : value) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    out << "\\u"
                        << std::hex << std::setfill(c: '0') << std::setw(n: 4)
                        << static_cast<int>(static_cast<unsigned char>(c))
                        << std::dec;
                } else {
                    out << c;
                }
        }
    }
    out << '"';
    return out.str();
}

/// Ensure destination parent directory exists before writing output.
void ensure_parent_dir(const std::string& path) {
    std::filesystem::path p(source: path);
    std::filesystem::path parent = p.parent_path();
    if (parent.empty()) return;

    std::error_code ec;
    std::filesystem::create_directories(p: parent, &: ec);
    if (ec) {
        throw std::runtime_error(arg: "failed to create output directory: " + parent.string());
    }
}

/// Read entire file content into a string.
std::string read_file_content(const std::string& path) {
    std::ifstream file(s: path);
    if (!file.is_open()) {
        throw std::runtime_error(arg: "failed to open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

} // namespace

/// Read and parse category_rules.json file.
std::vector<CategoryRule> read_category_rules(const std::string& path) {
    std::string content = read_file_content(path);
    return parse_category_rules_json(json_doc: content);
}

/// Read and parse scoring_rules.json file.
std::vector<ScoringRule> read_scoring_rules(const std::string& path) {
    std::string content = read_file_content(path);
    return parse_scoring_rules_json(json_doc: content);
}

/// Read and parse decision_bands.json file.
std::vector<DecisionBand> read_decision_bands(const std::string& path) {
    std::string content = read_file_content(path);
    return parse_decision_bands_json(json_doc: content);
}

/// Read account_profiles.jsonl as one JSON object per line.
std::vector<AccountProfile> read_account_profiles(const std::string& path) {
    std::ifstream file(s: path);
    if (!file.is_open()) {
        throw std::runtime_error(arg: "failed to open account profiles file: " + path);
    }

    std::vector<AccountProfile> profiles;
    std::string line;
    while (std::getline(&is: file, &str: line)) {
        std::string stripped = trim(value: line);
        if (stripped.empty()) continue;
        profiles.push_back(x: parse_account_profile_line(line: stripped));
    }
    return profiles;
}

/// Read account_links.jsonl as one JSON object per line.
std::vector<AccountLink> read_account_links(const std::string& path) {
    std::ifstream file(s: path);
    if (!file.is_open()) {
        throw std::runtime_error(arg: "failed to open account links file: " + path);
    }

    std::vector<AccountLink> links;
    std::string line;
    while (std::getline(&is: file, &str: line)) {
        std::string stripped = trim(value: line);
        if (stripped.empty()) continue;
        links.push_back(x: parse_account_link_line(line: stripped));
    }
    return links;
}

/// Read returns.jsonl as one return request JSON object per line.
std::vector<ReturnRequest> read_return_requests(const std::string& path) {
    std::ifstream file(s: path);
    if (!file.is_open()) {
        throw std::runtime_error(arg: "failed to open returns file: " + path);
    }

    std::vector<ReturnRequest> requests;
    std::string line;
    while (std::getline(&is: file, &str: line)) {
        std::string stripped = trim(value: line);
        if (stripped.empty()) continue;
        requests.push_back(x: parse_return_request_line(line: stripped));
    }
    return requests;
}

/// Serialize routing results to JSONL, preserving input request order.
void write_results(const std::string& path, const std::vector<RoutingResult>& results) {
    ensure_parent_dir(path);
    std::ofstream file(s: path);
    if (!file.is_open()) {
        throw std::runtime_error(arg: "failed to open output file: " + path);
    }

    for (const auto& result: RoutingResult const & : results) {
        std::ostringstream line;
        line << '{';
        line << "\"request_id\":" << json_escape(value: result.request_id) << ',';

        if (result.risk_score.has_value()) {
            line << "\"risk_score\":" << result.risk_score.value() << ',';
        } else {
            line << "\"risk_score\":null,";
        }

        line << "\"decision\":" << json_escape(value: result.decision);

        if (!result.reason.empty()) {
            line << ",\"reason\":" << json_escape(value: result.reason);
        }

        line << '}';
        file << line.str() << '\n';
    }
}

} // namespace returns
