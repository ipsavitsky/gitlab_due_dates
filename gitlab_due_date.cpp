#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <gitlab.hpp>
#include <fstream>
#include <string>

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

  auto i = g.get_overdue_issues_by_user(u);

  for (auto &iss : i) {
    spdlog::info("Issue due today: {}", iss.title);
    if (std::ranges::contains(iss.labels, "lane::staging")) {
      spdlog::info("it has lane::staging label, skipping");
      continue;
    }

    g.postpone_issue_by_week(iss);
  }

  return 0;
}
