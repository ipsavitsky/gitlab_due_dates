#include <chrono>
#include <cpr/cpr.h>
#include <format>
#include <gitlab.hpp>
#include <stdexcept>

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

  return j;
}

std::vector<issue> instance::get_overdue_issues_by_user(const user &user) {
  cpr::Response r =
      cpr::Get(cpr::Url{std::format("{}/issues", this->base_url)},
               cpr::Parameters{{"assignee_id", std::to_string(user.id)},
                               {"state", "opened"},
                               {"due_date", "overdue"}},
               cpr::Header{{"PRIVATE-TOKEN", this->token}});

  nlohmann::json j;
  if (r.status_code == 200) {
    j = nlohmann::json::parse(r.text);
  } else {
    throw std::runtime_error(
        std::format("Request return errror, return code: {}", r.status_code));
  }

  return j;
}

void instance::postpone_issue_by_week(const issue &iss) {
  const std::chrono::time_point now{std::chrono::system_clock::now()};
  std::chrono::sys_days current_date{
      std::chrono::floor<std::chrono::days>(now)};

  for (int i = 1; i <= 7; ++i) {
    std::chrono::weekday tuesday{2};

    ++current_date;
    if (std::chrono::weekday{current_date} == tuesday) {
      break;
    }
  }

  cpr::Response r =
      cpr::Put(cpr::Url{std::format("{}/projects/{}/issues/{}", this->base_url,
                                    iss.project_id, iss.iid)},
               cpr::Parameters{{"due_date", std::format("{:%F}", current_date)}},
               cpr::Header{{"PRIVATE-TOKEN", this->token}});

  if (r.status_code != 200) {
    throw std::runtime_error(
        std::format("Request return errror, return code: {}", r.status_code));
  }
}
} // namespace gitlab
