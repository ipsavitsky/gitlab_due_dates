#include <cpr/cert_info.h>
#include <cpr/cpr.h>
#include <fstream>
#include <gitlab.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

void send_notification(std::string ntfy_url, std::string ntfy_topic,
                       std::string ntfy_token, std::string text) {
  cpr::Response r = cpr::Post(
      cpr::Url{std::format("{}/{}", ntfy_url, ntfy_topic)},
      cpr::Header{{"Content-Type", "text/plain"},
                  {"Authorization", std::format("Bearer {}", ntfy_token)}},
      cpr::Body{text});
  if (r.status_code != 200) {
    throw std::runtime_error(
        std::format("Could not send notification, return code: {}; {}",
                    r.status_code, r.text));
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
    spdlog::info("Overdue issue: {}", iss.title);

    g.postpone_issue_by_week(iss);
  }

  auto ntfy_conf = conf["ntfy"];

  if (ntfy_conf.empty()) {
    spdlog::warn("No ntfy config, skipping notification");
  } else {
    std::string ntfy_url = ntfy_conf["url"];
    std::string ntfy_topic = ntfy_conf["topic"];
    std::string ntfy_token = ntfy_conf["token"];
    send_notification(
        ntfy_url, ntfy_topic, ntfy_token,
        i.size() > 0 ? std::format("{} overdue issues postponed", i.size())
                     : "No overdue issues postponed");
  }

  return 0;
}
