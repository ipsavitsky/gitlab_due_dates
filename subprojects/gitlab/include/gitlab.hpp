#include <string>
#include <vector>

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
      : base_url(base_url_), token(token_){};

  user get_current_user();
  std::vector<issue> get_today_issues_by_user(const user &user);
  void postpone_issue_by_week(const issue &iss);
};
} // namespace gitlab
