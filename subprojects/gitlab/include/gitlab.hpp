#ifndef GITLAB_HPP
#define GITLAB_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace gitlab {
struct user {
  int id;
  std::string username;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(user, id, username)
};

struct issue {
  int id;
  int iid;
  int project_id;
  std::string title;
  std::string due_date;
  std::vector<std::string> labels;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(issue, id, iid, project_id, title, due_date,
                                 labels)
};

class instance {
private:
  std::string base_url;
  std::string token;

public:
  instance(std::string base_url_, std::string token_)
      : base_url(base_url_), token(token_) {};

  user get_current_user();
  std::vector<issue> get_overdue_issues_by_user(const user &user);
  void postpone_issue(const issue &iss, const std::string &new_due_date);
};
} // namespace gitlab

#endif // GITLAB_HPP
