{ stdenv, secp256k1-zkp }:

stdenv.mkDerivation {
  name = "taproot-ringsig";

  buildInputs = [ secp256k1-zkp ];

  src = ./.;

  installPhase = ''
    mkdir -p $out/bin
    cp taproot-ringsig $out/bin/
  '';
}
