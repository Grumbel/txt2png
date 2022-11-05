{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        packages = rec {
          default = txt2png;

          txt2png = pkgs.stdenv.mkDerivation {
            pname = "txt2png";
            version = "0.0.0";

            src = nixpkgs.lib.cleanSource ./.;

            postPatch = ''
              substituteInPlace src/txt2png.cpp \
                --replace '"VeraMono.ttf"' '"${pkgs.dejavu_fonts}/share/fonts/truetype/DejaVuSansMono.ttf"' \
                --replace 'VeraMono.ttf' 'DejaVuSansMono.ttf'
            '';

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
      }
    );
}
