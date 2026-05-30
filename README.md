# nemai

Hi,
So this thing is an attempt to make my own programming language, with the goal to make it able to make a bootable efi application (that will be [here](https://github.com/aseivyy/nay/tree/main)).  

The reason I made is because if the stars will align and I will make that efi thing to the point where it will support user applications, I would want to compile on it, and then I would have to port a c compiler, but that is boring so I am here making my own language in advance yayy  

Also don't expect much yet, check the roadmap for what is implemented

# How to build
No makefile for now since kind of useless with just one file

1. Just compile the only c file, example `clang main.c`

# How to use
It takes just one and only argument, which is a file name, example `./a.out simple.ni`  

The output will be the same file as up here, but with ".obj" extension, in coff format (it actually is a valid one, works perfectly with objdump / lld-link, tested with efi as a target)

# Bugs
No bugs, I am testing things 10000 times to make sure I won't have to touch whatever is the thing that I made for as long as possible  

Also no code made by ai

# Roadmap (for juuust the next 1000000 years)

- [x] Lexer
- [x] Parser
  - [x] Variable definitions
  - [x] Function definitions
  - [x] Function body
  - [x] Returning from functions
- [x] Assembly generation
- [x] Creating object files
- [ ] Adding more functionality (with just general order)
  - [ ] Assigning to variables
  - [x] Math
  - [ ] Function returning with a value
  - [ ] Basic optimization from time to time
  - [ ] Calling functions
  - [ ] Pointers (to be honest i have been thinking that traditional pointers are stupid, what about just make a function that reads data from address from a variable, and function to get an address of a variable, no need for a literal sky with stars like "int ***whatever", no need for pointer types)
  - [ ] Conditionals
  - [ ] Structs
  - [ ] Calling functions from a known address
  - [ ] Linking and making executable format (efi)
  - [ ] Precompiler
    - [ ] Comments
    - [ ] Headers
  - [ ] Floats
  - [ ] Negative values
  - [ ] probably moreee but still a long time until here
- [ ] Linking and making executable format (for nay)
- [ ] Porting to nay
- [ ] Self compile
