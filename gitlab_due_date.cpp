#include <cpr/cpr.h>
#include <fstream>
#include <gitlab.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

void send_notification(std::string ntfy_url, std::string ntfy_token,
                       std::string text) {
  cpr::Response r = cpr::Post(
      cpr::Url{ntfy_url},
      cpr::Header{{"Content-Type", "text/plain"},
                  {"Authorization", std::format("Bearer {}", ntfy_token)}},
      cpr::Body{text});
  if (r.status_code != 200) {
    throw std::runtime_error(std::format(
        "Could not send notification, return code: {}", r.status_code));
  }
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  spdlog::info("starting...");

  std::string conf_filename = argc == 1 ? "conf.json" : argv[1];

  std::ifstream f(conf_filename);
  nlohmann::json conf = nlohmann::json::parse(f);

  std::string base_url =
      conf["base_url"].empty() ? "https://gitlab.com/api/v4" : conf["base_url"];

  std::string token;
  if (conf["token"].empty()) {
    if (const char *str_token = std::getenv("GITLAB_DD_TOKEN")) {
      spdlog::debug("reading the token from envvar");
      token = str_token;
    } else {
      throw std::runtime_error(
          "Token could not be read from either configuration or environment");
    }
  } else {
    spdlog::debug("using token from configuration");
    token = conf["token"];
  }

  gitlab::instance g(base_url, token);

  auto u = g.get_current_user();
  spdlog::info("username: {}; id: {}", u.username, u.id);

  auto i = g.get_overdue_issues_by_user(u);

  for (auto &iss : i) {
    spdlog::info("Overdue issues: {}", iss.title);

    g.postpone_issue_by_week(iss);
  }

  if (conf["ntfy_token"].empty()) {
    spdlog::warn("No ntfy token, skipping...");
  } else {
    if (conf["ntfy_url"].empty()) {
      spdlog::error("ntfy_token specified, but no ntfy_url in the config");
    } else {
      send_notification(conf["ntfy_url"], conf["ntfy_token"],
                        "Overdue issues postponed....");
    }
  }

  return 0;
}
