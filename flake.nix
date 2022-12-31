{
  description = "Nimble Util";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }: (utils.lib.eachDefaultSystem (system: 
    let 
      pkgs = import nixpkgs { inherit system; };
    in {
      devShell = with pkgs; mkShell {
        buildInputs = [
          bison cmake flex mcpp doxygen graphviz makeWrapper perl 
          bash-completion ncurses zlib sqlite libffi clang_14
        ];
      };
    }));
}

