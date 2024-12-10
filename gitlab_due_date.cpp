#include <algorithm>
#include <chrono>
#include <cpr/cpr.h>
#include <format>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

namespace gitlab {
struct user {
  int id;
  std::string username;
};

struct issue {
  int id;
  int iid;
  int project_id;
  std::string name;
  std::string due_date;
  std::vector<std::string> labels;
};

class instance {
private:
  std::string base_url;
  std::string token;

public:
  instance(std::string base_url_, std::string token_)
      : base_url(base_url_), token(token_) {}

  user get_current_user() {
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

  std::vector<issue> get_today_issues_by_user(const user &user) {
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

  void postpone_issue_by_week(const issue &iss) {
    std::istringstream in{iss.due_date};
    std::chrono::time_point<std::chrono::utc_clock> due_date;
    in >> std::chrono::parse("%F", due_date);
    std::chrono::duration week = std::chrono::days(7);
    due_date += week;
    spdlog::debug("calculated due date: {}", std::format("{:%F}", due_date));

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
};
} // namespace gitlab

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  spdlog::info("starting...");

  std::string conf_filename = argc == 1 ? "conf.json" : argv[1];

  std::ifstream f(conf_filename);
  nlohmann::json conf = nlohmann::json::parse(f);

  std::string base_url =
      conf["base_url"].empty() ? "https://gitlab.com/api/v4" : conf["base_url"];

  gitlab::instance g(base_url, conf["token"]);

  auto u = g.get_current_user();
  spdlog::info("username: {}; id: {}", u.username, u.id);

  auto i = g.get_today_issues_by_user(u);

  for (auto &iss : i) {
    spdlog::info("Issue due today: {}", iss.name);
    if (std::ranges::contains(iss.labels, "lane::staging")) {
      spdlog::info("it has lane::staging label, skipping");
      continue;
    }

    g.postpone_issue_by_week(iss);
  }

  return 0;
}
