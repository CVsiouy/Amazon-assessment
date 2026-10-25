#include "returns/engine.hpp"
#include "returns/file_io.hpp"
#include <iostream>
#include <string>

/// Run the return routing engine CLI
/// 
/// Required args:
/// --category-rules <path> --scoring-rules <path> --decision-bands <path>
/// --profiles <path> --links <path> --returns <path> --out <path>
int main(int argc, char** argv){
    std::string category_rules_path;
    std::string scoring_rules_path;
    std::string decision_bands_path;
    std::string profiles_path;
    std::string links_path;
    std::string returns_path;
    std::string out_path;

    for(int i=1;i<argc; ++i){
        std::string arg = argv[i];
        if(arg == "--category-rules" && i+1 < argc){
            category_rules_path = argv[++i];
        }
        else if(arg == "--scoring-rules" && i+1 < argc){
            sscoring_rules_path = argv[++i];
        }
        else if(arg == "--decision-bands" && i+1 < argc){
            decision_bands_path = argv[++i];
        }
        else if(arg == "--profiles" && i+1 < argc){
            profiles_path = argv[++i];
        }
        else if(arg == "--links" && i+1 < argc){
            links_path = argv[++i];
        }
        else if(arg == "--returns" && i+1 < argc){
            returns_path = argv[++i];
        }
        else if(arg == "--out" && i+1 < argc){
            out_path = argv[++i];
        }
    }

    if(category_rules_path.empty() || scoring_rules_path.empty() || decision_bands_path.empty() || profiles_path.empty() || links_path.empty() || returns_path.empty() || out_path.empty()){
        std::cerr << "All arguments are required: --category-rules --scoring-rules "
                     "--decision-bands --profiles --links --returns --out\n";

        return 2;
    }

    try {
        auto category_rules: std::vector<CategoryRule> = returns::read_category_rules(path: category_rules_path);
        auto scoring_rules: std::vector<ScoringRule> = returns::read_scoring_rules(path: scoring_rules_path);
        auto decision_bands: std::vector<DecisionBand> = returns::read_decision_bands(path: decision_bands_path);
        auto profiles: std::vector<AccountProfile> = returns::read_account_profiles(path: profiles_path);
        auto links: std::vector<AccountLink> = returns::read_account_links(path: links_path);
        auto requests: std::vector<ReturnRequest> = returns::read_return_requests(path: returns_path);

        auto engine: Engine = returns::Engine::build(
            category_rules, scoring_rules, decision_bands, profiles, links
        );

        auto results: std::vector<RoutingResult> engine.evaluate(requests);
        returns::write_results(path: out_path, results);
        return 0;
    }
    catch (const std::exception& e){
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
