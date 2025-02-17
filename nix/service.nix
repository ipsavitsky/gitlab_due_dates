{ gitlab_due_date }:
{
  lib,
  config,
  ...
}:
{
  options.services.gitlab_dd = {
    enable = lib.mkEnableOption "gitlab_dd";

    config_path = lib.mkOption {
      type = lib.types.path;
    };

    package = lib.mkOption {
      type = lib.types.package;
      default = gitlab_due_date.${config.nixpkgs.system}.default;
    };
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
        ${config.services.gitlab_dd.package}/bin/gitlab_due_date ${config.services.gitlab_dd.config_path}
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
