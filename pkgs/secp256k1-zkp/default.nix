{ stdenv, fetchFromGitHub, autoreconfHook }:

stdenv.mkDerivation {
  pname = "secp256k1-zkp";

  version = "0.1";

  src = fetchFromGitHub {
    owner = "apoelstra";
    repo = "secp256k1-mw";
    rev = "74f52a2a0d11a15afbc594af626bfef1c1875f1a";
    sha256 = "0mnsbychs67mjp06hvhkcjm1f6kx7zcsxf6bh145cm3apd0xvqpp";
  };

  nativeBuildInputs = [ autoreconfHook ];

  configureFlags = [ "--enable-experimental --enable-module-generator --enable-module-rangeproof"  ];
}
