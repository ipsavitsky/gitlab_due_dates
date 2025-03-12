{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    nom.url = "github:maralorn/nix-output-monitor";
    statix.url = "github:oppiliappan/statix";

    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      statix,
      nom,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        packages = rec {
          default = gitlab_due_date;

          libgitlab = pkgs.stdenv.mkDerivation {
            pname = "libgitlab";
            version = "0.1.0";
            src = ./subprojects/gitlab;
            mesonBuildType = "release";
            nativeBuildInputs = with pkgs; [
              meson
              ninja
              pkg-config
              cmake
            ];
            buildInputs = with pkgs; [
              libcpr
              nlohmann_json
            ];
          };

          gitlab_due_date = pkgs.stdenv.mkDerivation {
            pname = "gitlab_due_date";
            version = "0.1.0";
            src = ./.;
            nativeBuildInputs = with pkgs; [
              meson
              ninja
              pkg-config
              cmake
            ];

            mesonBuildType = "release";

            buildInputs =
              with pkgs;
              [
                libcpr
                spdlog
                nlohmann_json
              ]
              ++ [ self.packages.${system}.libgitlab ];
          };
        };

        devShells = {
          default = pkgs.mkShell {
            packages = with pkgs; [
              meson
              gcc
              ninja
              pkg-config
              clang-tools
              cmake
              libcpr
              nlohmann_json
              spdlog
              statix.packages.${system}.default
              nom.packages.${system}.default
            ];
          };
        };
      }
    )
    // {
      nixosModules = {
        gitlab_dd = import ./nix/service.nix { gitlab_due_date = self.packages; };
      };
    };
}
