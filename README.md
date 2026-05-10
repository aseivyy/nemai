# nemai

Hi,
So this thing is an attempt to make my own programming language, with the goal to make it able to make a bootable efi application (that will be [here](https://github.com/aseivyy/nay/tree/main)).  

The reason I made is because if the stars will align and I will make that efi thing to the point where it will support user applications, I would want to compile on it, and then I would have to port a c compiler, but that is boring so I am here making my own language in advance yayy  

Also, this thing is not yet finished, so it doesn't do much practical things for now.

# How to build
No makefile for now since kind of useless with just one file

1. Just compile the only c file, example `clang main.c`

# How to use
It takes just one and only argument, which is a file name, example `./a.out test.ni`  

For now it will only work unmodified when using the given `test.ni` file, or you will get segfaults, to fix remove the debug printing in the main function in the c file

# Bugs
No bugs, I am testing things 10000 times to make sure I won't have to touch whatever is the thing that I made for as long as possible  

Also no code made by ai

# Roadmap (for juuust the next 1000000 years)

- [x] Lexer
- [ ] Parser
  - [x] Variable definitions
  - [x] Function definitions
  - [ ] Function body
  - [ ] Returning from functions
- [ ] Intermediate representation (probably will be skipped)
- [ ] Assembly and/or machine code generation
- [ ] Linking and making executable formats, like efi
- [ ] Adding more functionality
  - [ ] Assigning to variables
  - [ ] Math
  - [ ] Basic optimization from time to time
  - [ ] Calling functions
  - [ ] Pointers
  - [ ] Conditionals
  - [ ] Structs
  - [ ] Calling functions from a known address
  - [ ] Precompiler
    - [ ] Comments
    - [ ] Headers
  - [ ] probably moreee but still a long time until here
- [ ] Porting to nay
- [ ] Self compile
