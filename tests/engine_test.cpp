#include "returns/engine.hpp"
#include "returns/parser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace returns;

namespace {

int g_failures = 0;
bool g_current_failed = false;
std::string g_current_detail;

struct TestResult {
    std::string name;
    bool passed;
    std::string detail;
};

std::vector<TestResult> g_results;

// ---------------------------------------------------------------------------
// Shared test configuration builders
// ---------------------------------------------------------------------------

std::vector<CategoryRule> base_category_rules() {
    std::string json = R"({"category_rules":[
        {"category":"ELECTRONICS","returnable":true,"allowed_return_window_days":30},
        {"category":"CLOTHING","returnable":true,"allowed_return_window_days":15},
        {"category":"BOOKS","returnable":true,"allowed_return_window_days":10},
        {"category":"GROCERY","returnable":false,"allowed_return_window_days":0},
        {"category":"SOFTWARE","returnable":false,"allowed_return_window_days":0},
        {"category":"FURNITURE","returnable":true,"allowed_return_window_days":45},
        {"category":"JEWELRY","returnable":true,"allowed_return_window_days":7},
        {"category":"TOYS","returnable":true,"allowed_return_window_days":20}
    ]})";
    return parse_category_rules_json(json_doc: json);
}

std::vector<ScoringRule> base_scoring_rules() {
    std::string json = R"({"scoring_rules":[
        {"rule":"total_return_history","ranges":[
            {"min":0,"max":2,"points":0},{"min":3,"max":5,"points":10},
            {"min":6,"max":10,"points":25},{"min":11,"max":1000000,"points":40}]},
        {"rule":"account_age_days","ranges":[
            {"min":0,"max":29,"points":40},{"min":30,"max":179,"points":20},
            {"min":180,"max":1000000,"points":0}]},
        {"rule":"order_value_usd","ranges":[
            {"min":0,"max":49,"points":0},{"min":50,"max":199,"points":5},
            {"min":200,"max":499,"points":10},{"min":500,"max":1000000,"points":20}]}
    ]})";
    return parse_scoring_rules_json(json_doc: json);
}

std::vector<DecisionBand> base_decision_bands() {
    std::string json = R"({"decision_bands":[
        {"min_score":0,"max_score":49,"decision":"AUTO_APPROVE"},
        {"min_score":50,"max_score":74,"decision":"MANUAL_REVIEW"},
        {"min_score":75,"max_score":100,"decision":"REJECT"}
    ]})";
    return parse_decision_bands_json(json_doc: json);
}

// u101:2, u102:8, u103:4, u104:0, u105:12, u106:1,
// u107:6, u108:3, u109:15, u110:0, u111:5
std::vector<AccountProfile> base_profiles() {
    std::vector<std::string> lines = {
        [0]=R"({"account_id":"u101","return_history_count":2})",
        [1]=R"({"account_id":"u102","return_history_count":8})",
        [2]=R"({"account_id":"u103","return_history_count":4})",
        [3]=R"({"account_id":"u104","return_history_count":0})",
        [4]=R"({"account_id":"u105","return_history_count":12})",
        [5]=R"({"account_id":"u106","return_history_count":1})",
        [6]=R"({"account_id":"u107","return_history_count":6})",
        [7]=R"({"account_id":"u108","return_history_count":3})",
        [8]=R"({"account_id":"u109","return_history_count":15})",
        [9]=R"({"account_id":"u110","return_history_count":0})",
        [10]=R"({"account_id":"u111","return_history_count":5})"
    };
    std::vector<AccountProfile> profiles;
    for (const auto& line: basic_string<char> const & : lines) {
        profiles.push_back(x: parse_account_profile_line(line));
    }
    return profiles;
}

// Groups: u101↔u102↔u103, u105↔u106, u107↔u108↔u109
std::vector<AccountLink> base_links() {
    std::vector<std::string> lines = {
        [0]=R"({"primary_account_id":"u101","related_account_id":"u102","link_type":"SHARED_SHIPPING_ADDRESS"})",
        [1]=R"({"primary_account_id":"u101","related_account_id":"u103","link_type":"SHARED_PAYMENT_METHOD"})",
        [2]=R"({"primary_account_id":"u105","related_account_id":"u106","link_type":"SHARED_SHIPPING_ADDRESS"})",
        [3]=R"({"primary_account_id":"u107","related_account_id":"u108","link_type":"SHARED_PAYMENT_METHOD"})",
        [4]=R"({"primary_account_id":"u107","related_account_id":"u109","link_type":"SHARED_SHIPPING_ADDRESS"})"
    };
    std::vector<AccountLink> links;
    for (const auto& line: basic_string<char> const & : lines) {
        links.push_back(x: parse_account_link_line(line));
    }
    return links;
}

// ---------------------------------------------------------------------------
// XML report generation
// ---------------------------------------------------------------------------

std::string xml_escape(const std::string& s) {
    std::string out;
    out.reserve(res_arg: s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out += c;
        }
    }
    return out;
}

void write_report(const std::string& path) {
    std::ofstream f(s: path);
    int total = static_cast<int>(g_results.size());
    f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    f << "<testsuites>\n";
    f << "  <testsuite name=\"returns_tests\" tests=\"" << total
      << "\" failures=\"" << g_failures
      << "\" errors=\"0\" skipped=\"0\" time=\"0.000\">\n";
    for (const auto& r: TestResult const & : g_results) {
        if (r.passed) {
            f << "    <testcase name=\"" << r.name
              << "\" classname=\"returns_tests\" time=\"0.000\" />\n";
        } else {
            std::string first_line = r.detail;
            auto nl: size_type = first_line.find(c: '\n');
            if (nl != std::string::npos) first_line = first_line.substr(pos: 0, n: nl);
            f << "    <testcase name=\"" << r.name
              << "\" classname=\"returns_tests\" time=\"0.000\">\n";
            f << "      <failure message=\"" << xml_escape(s: first_line) << "\">"
              << xml_escape(s: r.detail) << "</failure>\n";
            f << "    </testcase>\n";
        }
    }
    f << "  </testsuite>\n";
    f << "</testsuites>\n";
}

// ---------------------------------------------------------------------------
// Test macros
// ---------------------------------------------------------------------------

#define ASSERT_EQ(a, b) do { \
    auto _av = (a); auto _ev = (b); \
    if (_av != _ev) { \
        std::ostringstream _ss; \
        _ss << #a << ": expected \"" << _ev << "\", got \"" << _av \
            << "\" (line " << __LINE__ << ")"; \
        std::cerr << "  \033[1;31mFAIL: " << _ss.str() << "\033[0m" << '\n'; \
        if (!g_current_detail.empty()) g_current_detail += "\n"; \
        g_current_detail += _ss.str(); \
        g_current_failed = true; \
    } \
} while(0)

#define ASSERT_TRUE(a) do { \
    if (!(a)) { \
        std::ostringstream _ss; \
        _ss << #a << ": expected true (line " << __LINE__ << ")"; \
        std::cerr << "  \033[1;31mFAIL: " << _ss.str() << "\033[0m" << '\n'; \
        if (!g_current_detail.empty()) g_current_detail += "\n"; \
        g_current_detail += _ss.str(); \
        g_current_failed = true; \
    } \
} while(0)

#define RUN_TEST(fn) do { \
    g_current_failed = false; \
    g_current_detail.clear(); \
    fn(); \
    g_results.push_back({#fn, !g_current_failed, g_current_detail}); \
    if (g_current_failed) { \
        std::cerr << "\033[1;31mFAIL: " << #fn << "\033[0m" << '\n'; \
        g_failures++; \
    } else { \
        std::cout << "\033[32mPASS: " << #fn << "\033[0m" << '\n'; \
    } \
} while(0)

// --- Step 1: Category eligibility and return window ---

/// Returnable category within the allowed window passes to risk scoring.
void test_returnable_category_passes_to_scoring() {
    auto engine: Engine = Engine::build(
        category_rules: base_category_rules(), scoring_rules: base_scoring_rules(), decision_bands… base_decision_bands(),
        profiles: base_profiles(), links: base_links()
    );

    // u104: 0 returns, no links → total_return_history=0 → 0pts
    // age 365 → 0pts, order $150 → 5pts → score=5 → AUTO_APPROVE
    std::vector<ReturnRequest> requests = {
        [0]=parse_return_request_line(line: R"({"request_id":"r001","account_id":"u104","category":"ELECTRONICS","days_since_purchase":10,"order_value_usd":150,"account_age_days":365})")
    };
    auto results: std::vector<RoutingResult> = engine.evaluate(requests);

    ASSERT_TRUE(results[0].risk_score.has_value());
    if (results[0].risk_score.has_value()) {
        ASSERT_EQ(results[0].risk_score.value(), 5);
    }
    ASSERT_EQ(results[0].decision, std::string(s: "AUTO_APPROVE"));
    ASSERT_EQ(results[0].reason, std::string(s: ""));
}

/// Request exceeding the allowed return window is rejected without a risk score.
void test_return_window_expired() {
    auto engine: Engine = Engine::build(
        category_rules: base_category_rules(), scoring_rules: base_scoring_rules(), decision_bands… base_decision_bands(),
        profiles: {[0]=parse_account_profile_line(line: R"({"account_id":"u200","return_history_count":0})")}, links: {}
    );

    // ELECTRONICS window=30 days, request at 31 → RETURN_WINDOW_EXPIRED
    std::vector<ReturnRequest> requests = {
        [0]=parse_return_request_line(line: R"({"request_id":"r004","account_id":"u200","category":"ELECTRONICS","days_since_purchase":31,"order_value_usd":150,"account_age_days":365})")
    };
    auto results: std::vector<RoutingResult> = engine.evaluate(requests);

    ASSERT_TRUE(!results[0].risk_score.has_value());
    ASSERT_EQ(results[0].decision, std::string(s: "REJECT"));
    ASSERT_EQ(results[0].reason, std::string(s: "RETURN_WINDOW_EXPIRED"));
}

// --- Step 2: Linked account aggregation and risk scoring ---

/// Primary account aggregates return history across its entire linked group.
void test_linked_account_aggregation_primary() {
    auto engine: Engine = Engine::build(
        category_rules: base_category_rules(), scoring_rules: base_scoring_rules(), decision_bands… base_decision_bands(),
        profiles: base_profiles(), links: base_links()
    );

    // u101(2)+u102(8)+u103(4)=14 → 40pts, age 400 → 0pts, order $250 → 10pts → score=50
    std::vector<ReturnRequest> requests = {
        [0]=parse_return_request_line(line: R"({"request_id":"r005","account_id":"u101","category":"ELECTRONICS","days_since_purchase":12,"order_value_usd":250,"account_age_days":400})")
    };
    auto results: std::vector<RoutingResult> = engine.evaluate(requests);

    ASSERT_TRUE(results[0].risk_score.has_value());
    if (results[0].risk_score.has_value()) {
        ASSERT_EQ(results[0].risk_score.value(), 50);
    }
    ASSERT_EQ(results[0].decision, std::string(s: "MANUAL_REVIEW"));
    ASSERT_EQ(results[0].reason, std::string(s: "MEDIUM_RISK_SCORE"));
}

/// Transitive links: u201→u202→u203 means u203 sees u201 and u202 in its group.
void test_transitive_link_aggregation() {
    std::vector<AccountProfile> profiles = {
        [0]=parse_account_profile_line(line: R"({"account_id":"u201","return_history_count":5})"),
        [1]=parse_account_profile_line(line: R"({"account_id":"u202","return_history_count":3})"),
        [2]=parse_account_profile_line(line: R"({"account_id":"u203","return_history_count":4})")
    };
    std::vector<AccountLink> links = {
        [0]=parse_account_link_line(line: R"({"primary_account_id":"u201","related_account_id":"u202","link_type":"SHARED_SHIPPING_ADDRESS"})"),
        [1]=parse_account_link_line(line: R"({"primary_account_id":"u202","related_account_id":"u203","link_type":"SHARED_PAYMENT_METHOD"})")
    };

    auto engine: Engine = Engine::build(
        category_rules: base_category_rules(), scoring_rules: base_scoring_rules(), decision_bands… base_decision_bands(),
        profiles, links
    );

    // u203(4)+u202(3)+u201(5)=12 → 40pts, age 150 → 20pts, order $100 → 5pts → score=65
    std::vector<ReturnRequest> requests = {
        [0]=parse_return_request_line(line: R"({"request_id":"r006","account_id":"u203","category":"ELECTRONICS","days_since_purchase":10,"order_value_usd":100,"account_age_days":150})")
    };
    auto results: std::vector<RoutingResult> = engine.evaluate(requests);

    ASSERT_TRUE(results[0].risk_score.has_value());
    if (results[0].risk_score.has_value()) {
        ASSERT_EQ(results[0].risk_score.value(), 65);
    }
    ASSERT_EQ(results[0].decision, std::string(s: "MANUAL_REVIEW"));
    ASSERT_EQ(results[0].reason, std::string(s: "MEDIUM_RISK_SCORE"));
}

// --- Step 3: Decision mapping and edge cases ---

/// High-risk linked group maps to REJECT with HIGH_RISK_SCORE reason.
void test_high_risk_reject_with_reason() {
    auto engine: Engine = Engine::build(
        category_rules: base_category_rules(), scoring_rules: base_scoring_rules(), decision_bands… base_decision_bands(),
        profiles: base_profiles(), links: base_links()
    );

    // u107(6)+u108(3)+u109(15)=24 → 40pts, age 60 → 20pts, order $800 → 20pts → score=80
    std::vector<ReturnRequest> requests = {
        [0]=parse_return_request_line(line: R"({"request_id":"r008","account_id":"u107","category":"FURNITURE","days_since_purchase":20,"order_value_usd":800,"account_age_days":60})")
    };
    auto results: std::vector<RoutingResult> = engine.evaluate(requests);

    ASSERT_TRUE(results[0].risk_score.has_value());
    if (results[0].risk_score.has_value()) {
        ASSERT_EQ(results[0].risk_score.value(), 80);
    }
    ASSERT_EQ(results[0].decision, std::string(s: "REJECT"));
    ASSERT_EQ(results[0].reason, std::string(s: "HIGH_RISK_SCORE"));
}

/// Risk score exceeding 100 is clamped to 100.
void test_risk_score_clamped_to_100() {
    // Inflated rules: 60+50+40=150, must clamp to 100
    std::string inflated_json = R"({"scoring_rules":[
        {"rule":"total_return_history","ranges":[{"min":0,"max":1000000,"points":60}]},
        {"rule":"account_age_days","ranges":[{"min":0,"max":1000000,"points":50}]},
        {"rule":"order_value_usd","ranges":[{"min":0,"max":1000000,"points":40}]}
    ]})";
    auto inflated_rules: std::vector<ScoringRule> = parse_scoring_rules_json(json_doc: inflated_json);

    auto engine: Engine = Engine::build(
        category_rules: base_category_rules(), scoring_rules: inflated_rules, decision_bands… base_decision_bands(),
        profiles: {[0]=parse_account_profile_line(line: R"({"account_id":"u300","return_history_count":50})")}, links: {}
    );

    std::vector<ReturnRequest> requests = {
        [0]=parse_return_request_line(line: R"({"request_id":"r009","account_id":"u300","category":"ELECTRONICS","days_since_purchase":5,"order_value_usd":500,"account_age_days":10})")
    };
    auto results: std::vector<RoutingResult> = engine.evaluate(requests);

    ASSERT_TRUE(results[0].risk_score.has_value());
    if (results[0].risk_score.has_value()) {
        ASSERT_EQ(results[0].risk_score.value(), 100);
    }
    ASSERT_EQ(results[0].decision, std::string(s: "REJECT"));
    ASSERT_EQ(results[0].reason, std::string(s: "HIGH_RISK_SCORE"));
}

} // namespace

int main(int argc, char* argv[]) {
    std::string xml_path;
    for (int i = 1; i < argc; ++i) {
        if (std::string(s: argv[i]) == "--xml" && i + 1 < argc) {
            xml_path = argv[++i];
        }
    }

    // Step 1: Category eligibility and return window
    RUN_TEST(test_returnable_category_passes_to_scoring);
    RUN_TEST(test_return_window_expired);

    // Step 2: Linked account aggregation and risk scoring
    RUN_TEST(test_linked_account_aggregation_primary);
    RUN_TEST(test_transitive_link_aggregation);

    // Step 3: Decision mapping and edge cases
    RUN_TEST(test_high_risk_reject_with_reason);
    RUN_TEST(test_risk_score_clamped_to_100);

    if (!xml_path.empty()) {
        write_report(path: xml_path);
    }

    if (g_failures > 0) {
        std::cerr << "\n\033[1;31m" << g_failures << " test(s) failed.\033[0m\n";
        return 1;
    }
    std::cout << "\n\033[32mAll 6 tests passed!\033[0m\n";
    return 0;
}
