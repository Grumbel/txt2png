{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree {
          txt2png = pkgs.stdenv.mkDerivation {
            pname = "txt2png";
            version = "0.0.0";
            src = nixpkgs.lib.cleanSource ./.;
            enableParallelBuilding = true;
            nativeBuildInputs = with pkgs; [
              cmake
              pkg-config
            ];
            buildInputs = with pkgs; [
              freetype
              libjpeg
            ];
           };
        };
        defaultPackage = packages.txt2png;
      }
    );
}
