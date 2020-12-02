{ stdenv, secp256k1-zkp }:

stdenv.mkDerivation {
  name = "coinproof";

  buildInputs = [ secp256k1-zkp ];

  src = ./.;

  installPhase = ''
    mkdir -p $out/bin
    cp main $out/bin/
  '';

 #  installPhase = ''
 #    mkdir -p $out/lib
 #    mkdir -p $out/include

 #    # the buildPhase has already produced libfoo.a
 #    cp libfoo.a $out/lib
 #    cp foo.h    $out/include
 # '';
}
