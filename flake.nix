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
      # Buildable and runnable package
      packages.nface = pkgs.stdenv.mkDerivation {
        pname = "nface";
        version = "unstable";

        src = ./.;

        nativeBuildInputs = with pkgs; [pkg-config];
        buildInputs = with pkgs; [gcc ncurses libv4l];

        buildPhase = "make";
        installPhase = ''
          mkdir -p $out/bin
          cp bin/nface $out/bin/
        '';

        meta = {
          description = "Terminal-based face tracking using v4l2";
          license = pkgs.lib.licenses.gpl3Plus;
          maintainers = with pkgs.lib.maintainers; [];
          platforms = pkgs.lib.platforms.linux;
        };
      };

      # Default package (for `nix run`)
      packages.default = self.packages.${system}.nface;
    });
}
