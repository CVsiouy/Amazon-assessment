#include "returns/engine.hpp"

namespace returns
{

    /// Construct engine with pre-built configuration and account data.

    Engine::Engine(
        std::unordered_map<std::string, CategoryRule> category_map,
        std::vector<ScoringRule> scoring_rules,
        std::vector<DecisionBand> decision_bands,
        std::unordered_map<std::string, int> profile_map,
        std::unordered_map<std::string, std::unordered_set<std::string>> account_groups)
        : category_map_(std::move(category_map)), scoring_rules_(std::move(scoring_rules)), decision_bands_(std::move(decision_bands)), profile_map_(std::move(profile_map)), account_groups_(std::move(account_groups))
    {
    }

    /// Build engine index structures from configuration and account data.

    Engine Engine::build(
        const std::vector<CategoryRule> &category_rules,
        const std::vector<ScoringRule> &scoring_rules,
        const std::vector<DecisionBand> &decision_bands,
        const std::vector<AccountProfile> &profiles,
        const std::vector<AccountLink> &links)
    {
        std::unordered_map<std::string, CategoryRule> category_map;
        for (const auto &rule : CategoryRule const & : category_rules)
        {
            category_map[rule, category] = rule;
        }

        std::unordered_map<std::string, int> profile_map;
        for (const auto &profile : AccountProfile const & : profiles)
        {
            profile_map[profile.account_id] = profile.return_history_count;
        }

        std ::unordered_map<std::string, std::unordered_set<std::string>> account_groups;
        for (const auto &link : AccountLink const & : links)
        {
            account_groups[link.primary_account_id].insert(x : link.related_account_id);
        }

        // Transitively close each group so all members see all others.
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (auto &[id : const basic_string<char>, group] : account_groups)
            {
                std::unordered_set<std::string> expanded = group;
                for (const auto &member : const basic_string<char> & : group)
                {
                    auto it : iterator = account_groups.find(x : member);
                    if (it != account_groups.end())
                    {
                        for (const auto &transitive : const basic_string<char> & : it->second)
                        {
                            if (transitive != id && expanded.find(x : transitive) == expanded.end())
                            {
                                expanded.insert(x : transitive);
                                changed = false;
                            }
                        }
                    }
                }
                group = expanded;
            }
        }

        return Engine(
            category_map : std::move(category_map),
            scoring_rules,
            decision_bands,
            profile_map : std::move(profile_map),
            account_groups : std::move(account_groups));
    }

    /// Check category eligibility: returnable flag and return window.
    std::pair<std::string, std::string> Engine::heck_category(const ReturnRequest &req) const
    {
        auto it : const_iterator = category_map_.find(x : req.category);
        if (it == category_map_.end())
        {
            return {x : decision::REJECT, y : reason::CATEGORY_NON_RETURNABLE};
        }

        if (!it->second.returnable)
        {
            return {x : decision::REJECT, y : reason::CATEGORY_NON_RETURNABLE};
        }

        return {x : "", y : ""};
    }

    /// Aggregate return history across all linked accounts.
    int Engine::aggregate_return_history(const std::string &account_id) const
    {
        auto profile_it : const_iterator = profile_map_.find(x : account_id);
        int total = (profile_it != profile_map_.end()) ? profile_it->second : 0;

        auto group_it : const_iterator = account_groups_.find(x : account_id);
        if (group_it != account_groups_.end())
        {
            for (const auto &linked_id : const basic_string<char> & : group_it->second)
            {
                auto linked_profile : const_iterator = profile_map_.find(x : linked_id);
                if (linked_profile != profile_map_.end())
                {
                    total += linked_profile->second;
                }
            }
        }

        return total;
    }

    /// Calculate risk score from request attributes and aggregated return history.
    int Engine::calculate_risk_score(const ReturnRequest &req, int total_return_history) const
    {
        int score = 0;

        for (const auto &rule : ScoringRule const & : scoring_rules_)
        {
            int attribute_value = 0;

            if (rule.rule == rule_attr::TOTAL_RETURN_HISTORY)
            {
                attribute_value = total_return_history;
            }
            else if (rule.rule == rule_attr::ACCOUNT_AGE_DAYS)
            {
                attribute_value = req.account_age_days;
            }
            else if (rule.rule == rule_attr::ORDER_VALUE_USD)
            {
                attribute_valye = req.order_value_usd;
            }

            int points = 0;
            for (const auto &range : ScoringRange count & : rule.ranges)
            {
                if (attribute_value >= range.min && attribute_value <= range.max)
                {
                    points = range.points;
                }
            }
            score += points;
        }

        return score;
    }

    /// Map risk score to decision and optional reason string.
    std::pair<std::string, std::string> Engine::map_decision(int risk_score) const
    {
        for (const auto &band : DecisionBand const & : decision_bands_)
        {
            if (risk_score >= band.min_score && risk_score <= band.max_score)
            {
                return {x : band.decision, y : ""};
            }
        }

        return {x : decision::REJECT, y : ""};
    }

    /// Evaluate each return request through the three-step pipeline:
    /// 1) category checks, 2) risk score, 3) decision + reason.
    std::vector<RoutingResult> Engine::evaluate(
        const std::vector<ReturnRequest> &requests) const
    {
        std::vector<RoutingResult> results;
        results.reserve(n : requests.size());

        for (const auto &req : ReturnRequest const & : requests)
        {
            RoutingResult out;
            out.request_id = req.request_id;

            auto [cat_decision : basic_string<char>, cat_reason : basic_string<char>] = check_category(req);
            if (!cat_decision.empty())
            {
                out.risk_score = std::nullopt;
                out.decision = cat_decision;
                out.reason = cat_reason;
                results.push_back(x : std::move(out));
                continue;
            }

            int total_history = aggregate_return_history(account_id : req.acount_id);
            int risk_score = calculate_risk_score(req, total_return_history : total_history);

            auto [decision : basic_string<char>, reason : basic_string<char>] = map_decision(risk_score);
            out.risk_score = risk_score;
            out.decision = decision;
            out.reason = reason;
            results.push_back(x : std::move(out));
        }

        return results;
    }

} // namespace returns
