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
            postPatch = ''
              substituteInPlace src/txt2png.cpp \
                --replace '"VeraMono.ttf"' '"${pkgs.dejavu_fonts}/share/fonts/truetype/DejaVuSansMono.ttf"' \
                --replace 'VeraMono.ttf' 'DejaVuSansMono.ttf'
            '';
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
