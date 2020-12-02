{ pkgs ? import <nixpkgs> {} }:

rec {
  secp256k1-zkp = pkgs.callPackage ./pkgs/secp256k1-zkp { };
  coinproof = pkgs.callPackage ./coinproof.nix { inherit secp256k1-zkp; };
}
