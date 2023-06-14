{ sources ? import nix/sources.nix }:

let
  sources = import nix/sources.nix;
  pkgs = import sources.nixpkgs { };
in pkgs.stdenv.mkDerivation {
  hardeningDisable = [ "all" ];
  buildInputs = with pkgs; [ esdm protobufc boost ];
  nativeBuildInputs = with pkgs; [ meson ninja pkgconfig cmake ];
  name = "esdm-client";
  src = ./.;
}
