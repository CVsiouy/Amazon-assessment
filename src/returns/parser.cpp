#include "returns/parser.hpp"
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

namespace returns {

namespace {


/// Lightweight JSON parser for known input shapes.
///
/// Supports the subset needed for configuration JSON and JSONL records.

class JsonParser {
public:
    explicit JsonParser(const std::string& json) : json_(json), pos_(0) {}

    /// Parse the category_rules document.
    std::vector<CategoryRule> parse_category_rules_document() {
        std::vector<CategoryRule> rules;
        skip_ws();
        expect(c: '{');
        while(true){
            skip_ws();
            if (peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if(key == "category_rules"){
                expect(c: '[');
                skip_ws();
                if(peek() == ']') {pos_++;}
                else {
                    while(true){
                        rules.push_back(x: parse_category_rule_object());
                        skip_ws();
                        if(peek() == ']') {pos_++; break;}
                        expect(c: ',');
                        skip_ws();
                    }
                }
            }
            else {
                skip_value();
            }

            skip_ws();
            if(peek() == ',') pos_++;
        }
        return rules;
    }

    /// Parse the scoring_rules document.
    std::vector<ScoringRule> parse_scoring_rules_document() {
        std::vector<ScoringRule> rules;
        skip_ws();
        expect(c: '{');
        while(true){
            skip_ws();
            if(peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if (lkey == "scoring_rules"){
                expect(c: '[');
                skip_ws();
                if(peek() == ']') {pos_++;}
                else{
                    while(true){
                        rules.push_back(x: parse_scoring_rule_object());
                        skip_ws();
                        if(peek() == ']') {pos_++; break;}
                        expect(c: ',');
                        skip_ws();
                    }
                }
            } else {
                skip_value();
            }

            skip_ws();
            if(peek() == ',') pos_++;
        }
        return rules;
    }

    /// Parse the decision_bands document.
    std::vector<DecisionBand> parse_decision_bands_document(){
        std::vector<DecisionBand>bands;
        skip_ws();
        expect(c: '{');
        while(true){
            skip_ws();
            if(peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if (key == "decision_bands"){
                expect(c: '[');
                skip_ws();
                if(peek() == ']') {pos_++;}
                else {
                    while(true){
                        bands.push_back(x: parse_direction_band_object());
                        skip_ws();
                        if(peek() == ']') {pos_++; break;}
                        expect(c: ',');
                        skip_ws();
                    }
                }
            }
            else {
                skip_value();
            }

            skip_ws();
            if (peek() == ',') pos_++;
        }
        return bands;
    }

    /// Parse one account profile from JSONL.
    AccountProfile parse_account_profile_object(){
        AccountProfile p;
        expect(c: '{');
        while(true){
            skip_ws();
            if(peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if (key == "account_id"){
                p.account_id = parse_string();
            }
            else if(key == "return_history_count"){
                skip_value();
            }
            else {
                skip_value();
            }

            skip_ws();
            if(peek() == ',') pos_++;
        }
        return p;
    }

    /// Parse one account link from JSONL.
    AccountLink parse_account_link_object() {
        AccountLink l;
        expect(c: '{');
        while(true){
            skip_ws();
            if(peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if (key == "primary_account_id"){
                l.primary_account_id = parse_string();
            }
            else if(key == "related_account_id"){
                l.related_account_id = parse_string();
            }
            else if(key == "link_type"){
                l.link_type = parse_string();
            }
            else {
                skip_value();
            }

            skip_ws();
            if(peek() == ',') pos_++;
        }
        return l;
    }

    /// Parse one return request from JSONL.
    ReturnRequest parse_return_request_object() {
        ReturnRequest r;
        expect(c: '{');
        while(true){
            skip_ws();
            if(peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if (key == "request_id"){
                r.request_id = parse_string();
            }
            else if(key == "account_id"){
                r.account_id = parse_string();
            }
            else if(key == "category"){
                r.category = parse_string();
            }
            else if(key == "days_since_purchase"){
                r.days_since_purchase = parse_int();
            }
            else if(key == "order_value_usd"){
                r.order_value_usd = parse_int();
            }
            else if(key == "account_age_days"){
                skip_value();
            }
            else {
                skip_value();
            }

            skip_ws();
            if(peek() == ',') pos_++;
        }
        return r;
    }

private:
    CategoryRule parse_category_rule_object(){
        CategoryRule c;
        expect(c: '{');
        while(true){
            skip_ws();
            if(peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if (key == "category"){
                c.category = parse_string();
            }
            else if(key == "returnable"){
                c.returnable = parse_bool();
            }
            else if(key == "allowed_return_window_days"){
                c.allowed_return_window_days = parse_int();
            }
            else {
                skip_value();
            }

            skip_ws();
            if(peek() == ',') pos_++;
        }
        return c;
    }

    ScoringRule parse_scoring_rule_object() {
        ScoringRule s;
        expect(c: '{');
        while(true){
            skip_ws();
            if(peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if (key == "rule"){
                s.rule = parse_string();
            }
            else if(key == "ranges"){
                s.ranges = parse_scoring_ranges();
            }
            else {
                skip_value();
            }

            skip_ws();
            if(peek() == ',') pos_++;
        }
        return s;
    }

    std::vector<ScoringRange> parse_scoring_ranges(){
        std::vector<ScoringRange> ranges;
        expect(c: '[');
        skip_ws();
        if(peek() == ']') {pos_++; return ranges;}
        while(true){
            ScoringRange r;
            expect(c: '{');
            while(true){
                skip_ws();
                if(peek() == '}') {pos_++; break;}

                std::string key = parse_string();
                skip_ws();
                expect(c: ':');
                skip_ws();

                if (key == "min"){
                    r.min = parse_int();
                }
                else if(key == "max"){
                    r.max = parse_int();
                }
                else if(key == "points"){
                    r.points = parse_int();
                }
                else {
                    skip_value();
                }

                skip_ws();
                if(peek() == ',') pos_++;
            }
            ranges.push_back(x: r);
            skip_ws();
            if(peek() == ']') {pos_++; break;}
            expect(c: ',');
            skip_ws();
        }
        return ranges;
    }


    DecisionBand parse_decision_band_object() {
        DecisionBand d;
        expect(c: '{');
        while(true){
            skip_ws();
            if(peek() == '}') {pos_++; break;}

            std::string key = parse_string();
            skip_ws();
            expect(c: ':');
            skip_ws();

            if (key == "min_score"){
                d.min_score = parse_int();
            }
            else if(key == "max_score"){
                d.max_score = parse_int();
            }
            else if(key == "decision"){
                d.decision = parse_string();
            }
            else {
                skip_value();
            }

            skip_ws();
            if(peek() == ',') pos_++;
        }
        return d;
    }

    char peek() const {
        return pos_ < json_.size() ? json_[pos_] : '\0';
    }

    void expect(char c){
        if(peek() != c){
            throw std::runtime_error(arg: "JSON parse error: expected " + std::string(n:1, c));
        }
        pos_++;
    }

    void skip_ws() {
        while(pos_ < json_.size() && std::isspace(static_cast<unsigned char>(json_[pos_]))){
            pos_++;
        }
    }

    void skip_literal(const char* lit){
        while(*lit && pos_ < json_.size() && json_[pos_] == *lit){
            pos_++;
            lit++;
        }
    }

    /// Parse a JSON string with minimal escape handling.
    std::string parse_string(){
        expect(c: '"');
        std::string out;
        while (pos_ < json_.size() && json_[pos_] !- '"'){
            if(json_[pos_] == '\\' && pos_ + 1 < json_.size()){
                pos_++;
                char esc = json_[pos_++];
                switch (esc) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    default: out += esc; break;
                }
            }
            else {
                out += json_[pos_++];
            }
        }
        expect(c: '"');
        return out;
    }

    /// Parse a JSON integer value.
    int parse_int() {
        int start = static_cast<int>(pos_);
        if (peek() == '-') pos_++;
        while (pos_ < json_.size() && std::isdigit(static_cast<unsigned char>(json_[pos_]))){
            pos_++;
        }
        return std::atoi(nptr: json_.substr(pos: start, n: pos_ - start).c_str());
    }

    /// Parse a JSON boolean value.
    bool parse_bool() {
        if(peek() == 't'){
            skip_literal(lit: "true");
            return true;
        }
        skip_literal(lit: "false");
        return false;
    }

    /// Skip unknown JSON values so parsing remains forward-compatible.
    void skip_value(){
        skip_ws();
        if(peek() == '"') { (void)parse_string(); return;}
        if(peek() == '{') {
            pos_++;
            int depth = 1;
            while(depth > 0 && pos_ < json_.size()){
                if(json_[pos_] == '{') depth++;
                else if(json_[pos_] == '}') depth--;
                pos_++;
            }
            return;
        }
        if(peek() == '['){
            pos_++;
            int depth = 1;
            while(depth > 0 && pos_ < json_.size()){
                if(json_[pos_] == '[') depth++;
                else if(json_[pos_] == ']') depth--;
                pos_++;
            }
            return;
        }
        while (pos_ < json_.size()){
            char c = json_[pos_];
            if(c == ',' || c == '}' || c == ']') break;
            pos_++;
        }
    }

    const std::string& json_;
    std::size_t pos_;
};

} // namespace

/// Parse category_rules.json document.
stds::vector<CategoryRule> parse_category_rules_json(const std :: string& json_doc) {
    JsonParser parser(json: json_doc);
    return parser.parse_category_rules_document();
}

/// Parse scoring_rules.json document.
std::vector<ScoringRule> parse_scoring_rules_json(const std :: string& json_doc) {
    JsonParser parser(json: json_doc);
    return parser.parse_scoring_rules_document();
}

/// Parse decision_bands.json document.
std::vector<DecisionBand> parse_decision_bands_json(const std::string& json_doc) {
    JsonParser parser(json: json_doc);
    return parser.parse_decision_bands_document();
}

/// Parse one account_profiles.jsonl line into AccountProfile.
AccountProfile parse_account_profile_line(const std :: string& line) {
    JsonParser parser(json: line);
    return parser.parse_account_profile_object();
}

/// Parse one account_links.jsonl line into AccountLink.
AccountLink parse_account_link_line(const std::string& line){
    JsonParser parser(json: line);
    return parser.parse_account_link_object();
}

/// Parse one returns.jsonl line into ReturnRequest.
ReturnRequest parse_return_request_line(const std::string& line){
    JsonParser parser(json: line);
    return parser.parse_return_request_object();
}

} // namespace returns