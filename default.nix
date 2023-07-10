{ sources ? import nix/sources.nix }:

let
  sources = import nix/sources.nix;
  pkgs = import sources.nixpkgs { };
in pkgs.stdenv.mkDerivation {
  hardeningDisable = [ "all" ];
  buildInputs = with pkgs; [ esdm protobufc boost jsoncpp ];
  nativeBuildInputs = with pkgs; [ meson ninja pkgconfig cmake ];
  name = "esdm-client";
  src = ./.;

  # https://github.com/NixOS/nixpkgs/issues/86131
  BOOST_INCLUDEDIR = "${pkgs.lib.getDev pkgs.boost}/include";
  BOOST_LIBRARYDIR = "${pkgs.lib.getLib pkgs.boost}/lib";
}
