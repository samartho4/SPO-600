# Custom GCC Pass Implementation

This repository contains my implementation of a custom GCC compiler pass for the SPO600 course (Software Portability and Optimization).

## Project Overview

The goal of this project was to create a GCC pass that:
1. Iterates through code being compiled
2. Prints the name of every function
3. Counts the number of basic blocks in each function
4. Counts the number of GIMPLE statements in each function

## Implementation Details

The pass is implemented as a GIMPLE pass that:
- Uses `function_name()` to print the name of each function
- Iterates through basic blocks using `FOR_EACH_BB_FN`
- Counts GIMPLE statements using gimple statement iterators
- Outputs information to the GCC dump file

## Integration with GCC

To integrate this pass into GCC, three main files needed modification:
1. `tree-pass.h`: Added the declaration of the pass factory function
2. `passes.def`: Registered the pass in the GCC compilation pipeline
3. `Makefile.in`: Updated to include the new source file in the build


For a detailed discussion of the troubleshooting process, see my blog posts:
- [Project 1: Creating a Custom GCC Pass](https://software-portability-and-optimization.hashnode.dev/project-1-creating-a-custom-gcc-pass)
- [Some Troubleshooting on the Way](https://software-portability-and-optimization.hashnode.dev/some-troubleshooting-on-the-way)

## Usage

If successfully built, the pass would be invoked with:
gcc -fdump-tree-my-pass source.c -o output
