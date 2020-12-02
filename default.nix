{ pkgs ? import <nixpkgs> {} }:

rec {
  secp256k1-zkp = pkgs.callPackage ./pkgs/secp256k1-zkp { };
  taproot-ringsig = pkgs.callPackage ./taproot-ringsig.nix { inherit secp256k1-zkp; };
}
