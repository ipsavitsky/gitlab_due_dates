_: {
  projectRootFile = "flake.nix";
  programs = {
    nixfmt.enable = true;
    statix.enable = true;
    deadnix.enable = true;
    meson.enable = true;
    clang-format.enable = true;
    mdformat.enable = true;
  };
}
