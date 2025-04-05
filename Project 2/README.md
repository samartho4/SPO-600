# SPO600 Winter 2025 - Project Stage 2
## Project Description
This project extends the basic GCC compiler pass developed in Stage 1. The primary goal for Stage 2 is to create a GCC pass that identifies cloned functions and determines whether they're identical in logic (ignoring superficial differences). Based on this analysis, the pass will indicate if clones should be pruned.

## Objectives
Detect cloned functions (function.variant naming pattern).
Compare the GIMPLE representations of original and cloned functions.
Ignore superficial differences (SSA identifiers, labels, basic block numbering).
Output the decision: either PRUNE: function or NOPRUNE: function.

https://software-portability-and-optimization.hashnode.dev/project-stage-2-initialised
