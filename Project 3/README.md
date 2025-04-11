# AFMV Multi-Function Testing Suite

## Overview
This project extends the Automatic Function Multi-Versioning (AFMV) functionality to process multiple sets of cloned functions in a single program, making PRUNE/NOPRUNE recommendations for each function independently. The testing suite demonstrates that our implementation correctly handles mixed scenarios where some functions benefit from cloning (NOPRUNE) while others don't (PRUNE).

## Requirements
- GCC with AFMV support (built with the Stage II codebase)
- Linux environment with both x86_64 and aarch64 test capabilities
- GNU Make

## Directory Structure
```
afmv-multifunc/
├── Makefile            # Main makefile for all test cases
├── README.md           # This file
├── tests/
│   ├── test_mixed/     # Test with both PRUNE and NOPRUNE functions
│   │   ├── test.c      # Source file with multiple cloned functions
│   │   └── Makefile    # Build instructions
│   ├── test_all_prune/ # Test with all functions recommended for PRUNE
│   │   ├── test.c
│   │   └── Makefile
│   └── test_all_noprune/ # Test with all functions recommended for NOPRUNE 
│       ├── test.c
│       └── Makefile
└── scripts/
    ├── run_all_tests.sh # Script to run all tests on both architectures
    └── parse_results.py # Script to parse and summarize test results
```

## Test Cases Description

1. **Mixed PRUNE/NOPRUNE Test**
   - Contains functions with different characteristics:
     - `scalar_math_function()`: Simple scalar operations (typically PRUNE)
     - `vector_process_function()`: Array processing with vectorization (typically NOPRUNE)

2. **All PRUNE Test**
   - Multiple functions, all with characteristics that should lead to PRUNE recommendations

3. **All NOPRUNE Test**
   - Multiple functions, all with characteristics that should lead to NOPRUNE recommendations

## How to Run Tests

### Build and Run All Tests
```bash
cd afmv-multifunc
make test
```

### Run Tests on Specific Architecture
```bash
# For x86_64
make test_x86

# For aarch64
make test_aarch64
```

### Run Individual Test
```bash
cd tests/test_mixed
make
./test
```

## Expected Results

The test output will show:
- Which functions were cloned
- The PRUNE/NOPRUNE recommendation for each function
- The dispatching behavior for each function across different ISA levels

Example expected output for mixed test:
```
Testing multiple functions with AFMV:
Function: scalar_math_function
  Recommendation: PRUNE (cloning not beneficial)
  Preferred variant: baseline
  
Function: vector_process_function
  Recommendation: NOPRUNE (cloning beneficial)
  Preferred variants: avx2 > avx > sse4.2 > baseline
  
Dispatching tests:
  scalar_math_function dispatched to generic variant on all ISAs
  vector_process_function dispatched to optimized variant based on available ISA
```

## Explanation of Test Files

Each test.c file includes:
- Multiple functions with `__attribute__((target_clones(...)))` 
- Various workloads that represent different optimization scenarios
- Main function that exercises all cloned functions
- Measuring and reporting code to demonstrate dispatch behavior

## Troubleshooting

If you encounter issues:
1. Ensure your GCC build includes the AFMV functionality from Stage II
2. Check environment variables with `make check_env`
3. For detailed logs, run `make test VERBOSE=1`
