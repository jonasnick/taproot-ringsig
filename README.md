# taproot-ringsig

This is a proof of concept that creates a ring signature over all taproot outputs in the UTXO set. The inputs of the signing algorithm are the output of bitcoind's `dumptxoutset`, a message and  a secret key corresponding to at least one of the taproot outputs. It uses *Borromean Ring Signatures* as implemented in [libsecp256k1-zkp](https://github.com/ElementsProject/secp256k1-zkp/pull/110).

It would be interesting to explore if this scheme can be more than a ["Bragging Rights Signature"](https://gist.github.com/AdamISZ/52aa2e4e48240dfbadebb316507d0749#bragging-rights-ring-signature). In any case, similar constructions can be used to implement Fidelity Bonds or [Stake Certificates](https://lists.linuxfoundation.org/pipermail/lightning-dev/2020-November/002884.html).

## Example

```
taproot-ringsig keygen
pubkey: 027A61C588FD357D8ED58F624FA7F97A651D1AC00B53B055E9B852507DD319A3D4
```

```
taproot-ringsig sign ./bitcoindir/signet/utxos.dat $(printf "@n1ckler was here" | sha256sum | awk '{ print $1 }')

UTXO dump for block 00000019d7cff09e9e516ce7c3cbfa8c83f72c248264f1c4588f4dad8c4be18c contains
5 unspent taproot outputs with more than 5000 sats:
[28f3ec8baac348afb43708003393d2bfc372580ef6ac090d7f923b3c2ed6d9e3, 7a61c588fd357d8ed58f624fa7f97a651d1ac00b53b055e9b852507dd319a3d4, acd385f4c428f2ce97644de474a579a77435f40b6161d1c1875f48f2626fccde, 70271d98a521d0e4102ebdbc40f3e553666fb5b85c8c3d2709138568c6c90b23, 28f3ec8baac348afb43708003393d2bfc372580ef6ac090d7f923b3c2ed6d9e3]

Signature:
3a08498a05e32bc614fadf02441ac038f550d5f2c2004491982e0d7ed2abfaffc4b2e863b510c3b0ad9968c96dc8b58ee4dc65b8319fd92f6d7e6e5725c3a802b3215ebf2b914e2545f6f962a784091fbdb163887d17aa0689a3310d13fe448830868c4b845e53c2679eb373cae7da904992c71a16f5d74a516faee5c34f578a78e61fba341c1856748760426eeb95524f04da01b00bb6f9ff6e0df65d7ccdf17d0b7142c29303b1bf305745168f77592605e3da2e46538966f8edc20969ffba
```

```
taproot-ringsig verify ./bitcoindir/signet/utxos.dat 3a08498a05e32bc614fadf02441ac038f550d5f2c2004491982e0d7ed2abfaffc4b2e863b510c3b0ad9968c96dc8b58ee4dc65b8319fd92f6d7e6e5725c3a802b3215ebf2b914e2545f6f962a784091fbdb163887d17aa0689a3310d13fe448830868c4b845e53c2679eb373cae7da904992c71a16f5d74a516faee5c34f578a78e61fba341c1856748760426eeb95524f04da01b00bb6f9ff6e0df65d7ccdf17d0b7142c29303b1bf305745168f77592605e3da2e46538966f8edc20969ffba $(printf "@n1ckler was here" | sha256sum | awk '{ print $1 }')

UTXO dump for block 00000019d7cff09e9e516ce7c3cbfa8c83f72c248264f1c4588f4dad8c4be18c contains
5 unspent taproot outputs with more than 5000 sats:
[28f3ec8baac348afb43708003393d2bfc372580ef6ac090d7f923b3c2ed6d9e3, 7a61c588fd357d8ed58f624fa7f97a651d1ac00b53b055e9b852507dd319a3d4, acd385f4c428f2ce97644de474a579a77435f40b6161d1c1875f48f2626fccde, 70271d98a521d0e4102ebdbc40f3e553666fb5b85c8c3d2709138568c6c90b23, 28f3ec8baac348afb43708003393d2bfc372580ef6ac090d7f923b3c2ed6d9e3]

Verifying ring signature for public keys and message...ok!
```

## Building

The easiest way is to install the nix package manager and run

```
nix-build -A taproot-ringsig
```

Alternatively you can make the Makefile work for you.
