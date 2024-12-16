#include <gitlab.hpp>
#include <chrono>
#include <format>
#include <stdexcept>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

namespace gitlab {
  user instance::get_current_user() {
    cpr::Response r = cpr::Get(cpr::Url{std::format("{}/user", this->base_url)},
                               cpr::Header{{"PRIVATE-TOKEN", this->token}});

    nlohmann::json j;
    if (r.status_code == 200) {
      j = nlohmann::json::parse(r.text);
    } else {
      throw std::runtime_error(
          std::format("Request return errror, return code: {}", r.status_code));
    }

    return {j["id"], j["username"]};
  }

  std::vector<issue> instance::get_today_issues_by_user(const user &user) {
    cpr::Response r =
        cpr::Get(cpr::Url{std::format("{}/issues", this->base_url)},
                 cpr::Parameters{{"assignee_id", std::to_string(user.id)},
                                 {"state", "opened"},
                                 {"due_date", "today"}},
                 cpr::Header{{"PRIVATE-TOKEN", this->token}});

    nlohmann::json j;
    if (r.status_code == 200) {
      j = nlohmann::json::parse(r.text);
    } else {
      throw std::runtime_error(
          std::format("Request return errror, return code: {}", r.status_code));
    }

    std::vector<issue> issues;
    for (auto &issue_json : j) {
      issues.push_back({issue_json["id"], issue_json["iid"],
                        issue_json["project_id"], issue_json["title"],
                        issue_json["due_date"], issue_json["labels"]});
    }

    return issues;
  }

  void instance::postpone_issue_by_week(const issue &iss) {
    std::istringstream in{iss.due_date};
    std::chrono::time_point<std::chrono::utc_clock> due_date;
    in >> std::chrono::parse("%F", due_date);
    std::chrono::duration week = std::chrono::days(7);
    due_date += week;

    cpr::Response r =
        cpr::Put(cpr::Url{std::format("{}/projects/{}/issues/{}",
                                      this->base_url, iss.project_id, iss.iid)},
                 cpr::Parameters{{"due_date", std::format("{:%F}", due_date)}},
                 cpr::Header{{"PRIVATE-TOKEN", this->token}});

    if (r.status_code != 200) {
      throw std::runtime_error(
          std::format("Request return errror, return code: {}", r.status_code));
    }
  }
} // namespace gitlab
