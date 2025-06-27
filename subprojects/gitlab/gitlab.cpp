#include "include/gitlab.hpp"
#include <cpr/cpr.h>
#include <format>
#include <stdexcept>

namespace gitlab {
user instance::get_current_user() {
  cpr::Response r = cpr::Get(cpr::Url{std::format("{}/user", this->base_url)},
                             cpr::Header{{"PRIVATE-TOKEN", this->token}});

  nlohmann::json j;
  if (r.status_code == 200) {
    j = nlohmann::json::parse(r.text);
  } else {
    throw std::runtime_error(std::format(
        "Request return errror, return code: {}; {}", r.status_code, r.text));
  }

  return j;
}

std::vector<issue> instance::get_overdue_issues_by_user(const user &user) {
  cpr::Response r =
      cpr::Get(cpr::Url{std::format("{}/issues", this->base_url)},
               cpr::Parameters{{"assignee_id", std::to_string(user.id)},
                               {"state", "opened"},
                               {"due_date", "overdue"},
                               {"scope", "all"}},
               cpr::Header{{"PRIVATE-TOKEN", this->token}});

  nlohmann::json j;
  if (r.status_code == 200) {
    j = nlohmann::json::parse(r.text);
  } else {
    throw std::runtime_error(std::format(
        "Request return errror, return code: {}; {}", r.status_code, r.text));
  }

  return j;
}

void instance::postpone_issue(const issue &iss,
                              const std::string &new_due_date) {
  cpr::Response r =
      cpr::Put(cpr::Url{std::format("{}/projects/{}/issues/{}", this->base_url,
                                    iss.project_id, iss.iid)},
               cpr::Parameters{{"due_date", new_due_date}},
               cpr::Header{{"PRIVATE-TOKEN", this->token}});

  if (r.status_code != 200) {
    throw std::runtime_error(std::format(
        "Request return errror, return code: {}; {}", r.status_code, r.text));
  }
}
} // namespace gitlab