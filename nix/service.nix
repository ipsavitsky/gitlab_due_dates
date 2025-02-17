{
  lib,
  pkgs,
  config,
  ...
}:
{
  options.services.gitlab_dd = {
    enable = lib.mkOption {
      type = lib.types.bool;
      default = false;
    };

    config = lib.mkOption {
      type = lib.types.path;
    };

    package = lib.mkPackageOption pkgs "" { };
  };

  config = lib.mkIf config.services.gitlab_dd.enable {
    systemd.timers."gitlab_due_date" = {
      wantedBy = [ "timers.target" ];

      timerConfig = {
        OnCalendar = "Wed *-*-* 00:00:00";
        Unit = "gitlab_due_date.service";
      };
    };

    systemd.services."gitlab_due_date" = {
      script = ''
        ${config.services.gitlab_dd.package}/bin/gitlab_due_date ${config.services.gitlab_dd.config}
      '';

      serviceConfig = {
        Type = "oneshot";
        User = "gitlab_dd";
      };
    };

    users.users.gitlab_dd = {
      isSystemUser = true;
      group = "gitlab_dd";
    };

    users.groups.gitlab_dd = { };
  };
}
