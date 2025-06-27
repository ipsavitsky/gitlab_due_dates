#include "subprojects/gitlab/include/gitlab.hpp"
#include <algorithm> // For std::transform
#include <cctype>    // For std::tolower
#include <chrono>    // For date and time calculations
#include <cpr/cpr.h>
#include <format> // For std::format
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex> // For regex parsing of date strings
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>

// Helper to convert string to lowercase
std::string to_lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

std::string calculate_due_date(const std::string &relative_date_str) {
  using namespace std::chrono;

  sys_days today = floor<days>(system_clock::now());
  sys_days target_date = today;

  std::string lower_str = to_lower(relative_date_str);

  if (lower_str == "today") {
    target_date = today;
  } else if (lower_str == "tomorrow") {
    target_date = today + days(1);
  } else if (lower_str.rfind("next ", 0) == 0) { // Starts with "next "
    std::string day_name = lower_str.substr(5); // Get "tuesday", "monday", etc.
    weekday target_weekday;

    if (day_name == "monday")
      target_weekday = Monday;
    else if (day_name == "tuesday")
      target_weekday = Tuesday;
    else if (day_name == "wednesday")
      target_weekday = Wednesday;
    else if (day_name == "thursday")
      target_weekday = Thursday;
    else if (day_name == "friday")
      target_weekday = Friday;
    else if (day_name == "saturday")
      target_weekday = Saturday;
    else if (day_name == "sunday")
      target_weekday = Sunday;
    else {
      throw std::runtime_error(std::format(
          "Unknown day of week in 'next <day>': {}", relative_date_str));
    }

    // Calculate next occurrence of the target weekday
    weekday today_weekday = today;
    sys_days next_occurrence = today;

    if (today_weekday == target_weekday) {
      next_occurrence = today + days(7); // If today is the target day, it means
                                         // "next" means next week.
    } else {
      next_occurrence = today + (target_weekday - today_weekday);
      if (next_occurrence <=
          today) { // If the calculated day is today or in the past (this week)
        next_occurrence += days(7); // Move to next week
      }
    }
    target_date = next_occurrence;

  } else {
    // Try parsing "[number] days"
    std::smatch matches;
    std::regex days_regex("^(\\d+)\\s*days?$");
    if (std::regex_match(lower_str, matches, days_regex) &&
        matches.size() > 1) {
      int num_days = std::stoi(matches[1].str());
      target_date = today + days(num_days);
    } else {
      // If it's not a known relative format, assume it's an absolute date
      // (YYYY-MM-DD) and return it as is.
      return relative_date_str;
    }
  }

  return std::format("{:%F}", target_date); // Format as YYYY-MM-DD
}

void send_notification(std::string ntfy_url, std::string ntfy_topic,
                       std::string ntfy_token, std::string text) {
  cpr::Response r = cpr::Post(
      cpr::Url{std::format("{}/{}", ntfy_url, ntfy_topic)},
      cpr::Header{{"Content-Type", "text/plain"},
                  {"X-Title", "Gitlab DD"},
                  {"X-Priority", "4"},
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
  if (!f.is_open()) {
    throw std::runtime_error(
        std::format("Could not open configuration file: {}", conf_filename));
  }
  nlohmann::json conf = nlohmann::json::parse(f);

  std::string base_url = conf.value("base_url", "https://gitlab.com/api/v4");

  std::string new_due_date_str = conf.value("new_due_date", "");
  if (new_due_date_str.empty()) {
    throw std::runtime_error(
        "new_due_date must be specified in the configuration (e.g., 'next "
        "Tuesday', '7 days', or 'YYYY-MM-DD').");
  }

  // Calculate the actual due date based on the relative string
  std::string actual_due_date = calculate_due_date(new_due_date_str);

  std::string token = conf.value("token", "");
  if (token.empty()) {
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

    g.postpone_issue(iss, actual_due_date);
  }

  if (!conf.contains("ntfy")) {
    spdlog::warn("No ntfy config, skipping notification");
  } else {
    const auto &ntfy_conf_obj = conf["ntfy"];
    std::string ntfy_url = ntfy_conf_obj.value("url", "");
    std::string ntfy_topic = ntfy_conf_obj.value("topic", "");
    std::string ntfy_token = ntfy_conf_obj.value("token", "");
    send_notification(
        ntfy_url, ntfy_topic, ntfy_token,
        i.size() > 0 ? std::format("{} overdue issue(s) postponed", i.size())
                     : "No overdue issues postponed");
  }

  return 0;
}
