# nemai

Hi,
So this thing is an attempt to make my own programming language, with the goal to make it able to make a bootable efi application (that will be [here](https://github.com/aseivyy/nay/tree/main)).  

The reason I made is because if the stars will align and I will make that efi thing to the point where it will support user applications, I would want to compile on it, and then I would have to port a c compiler, but that is boring so I am here making my own language in advance yayy  

Also don't expect much yet, check the roadmap for what is implemented

# How to build

## Dependencies:
- clang
- git (optional but reccomended)

## Steps:
1. Clone the repository

```sh
git clone https://github.com/aseivyy/nemai.git
```

2. Build

```sh
make
```

The generated file is `./nemai`

# How to install

Run `sudo ./install.sh` after building. You can delete the repo after

# How to use
It can compile just one file at a time, which can be done with `nemai simple.ni`  

The output will have the name of provided file, but with an additional ".obj" extension, and itself be in the coff object format

If you wish to skip the object file generation (basically only check for errors), then use the -n flag, for example `nemai -n simple.ni`

# Bugs / issues
I try to test as many times as possible, however if was commited then it means it worked at least there

The source code only started having comments when I was finishing doing structs as I found out how to align them to look fine in emacs a bit too late, so it may be not really readable

Also no code made by ai

# Roadmap (for juuust the next 1000000 years)

- [x] Basic Lexer
- [x] Basic parser
  - [x] Variable definitions
  - [x] Function definitions
  - [x] Function body
  - [x] Returning from functions
- [x] Assembly generation
- [x] Creating object files
- [ ] Adding more functionality (with just the general order)
  - [x] Assigning to variables
  - [x] Math
  - [x] Function returning with a value
  - [x] Basic optimization from time to time
  - [ ] Calling functions
  - [x] Pointers
  - [ ] Conditionals
  - [x] Structs
  - [ ] Calling functions from a known address
  - [ ] Linking and making executable format (efi)
  - [x] Comments
  - [ ] Headers / using multiple files at once
  - [ ] Strings
  - [ ] Floats
  - [ ] Negative values
  - [ ] probably moreee but still a long time until here
- [ ] Linking and making executable format (for nay)
- [ ] Porting to nay
- [ ] Self compile
