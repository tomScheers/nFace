{
  description = "nFace build and dev environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          gcc
          ncurses
          tmux
          v4l-utils # for v4l2 support
          pkg-config
          libv4l
        ];
        shellHook = ''
          echo "Development shell for nFace ready."
          echo "You can now run 'make' to build the project."
        '';
      };
    });
}
