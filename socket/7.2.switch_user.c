#include <unistd.h>

static int switch_to_user(uid_t user_id, gid_t gp_id) {
  // target user is not root
  if ((user_id == 0) && (gp_id == 0)) {
    return 0;
  }

  // current user
  gid_t gid = getgid();
  uid_t uid = getuid();
  if (((gid != 0) || (uid != 0)) && ((gid != gp_id) || (uid != user_id))) {
    return 0;
  }

  if (uid != 0) {
    return 1;
  }

  if ((setgid(gp_id) < 0) || (setuid(user_id) < 0)) {
    return 0;
  }

  return 1;
}

int main() { switch_to_user(1001, 1001); }
