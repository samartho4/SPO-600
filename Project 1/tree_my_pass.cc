#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree-pass.h"
#include "pass_manager.h"
#include "context.h"
#include "diagnostic-core.h"
#include "tree.h"
#include "tree-core.h"
#include "basic-block.h"
#include "gimple.h"
#include "gimple-iterator.h"

namespace {

const pass_data my_pass_data = {
    GIMPLE_PASS,         // Pass type: operates on GIMPLE.
    "tree-my-pass",       // Name of the pass.
    OPTGROUP_NONE,        // No optimization group.
    TV_TREE_OPS,          // Tree verifier id.
    0,                    // No properties required.
    0,                    // No properties provided.
    0,                    // No properties destroyed.
    0                     // No specific flags.
};

class my_pass : public gimple_opt_pass {
public:
    my_pass(gcc::context *ctxt)
        : gimple_opt_pass(my_pass_data, ctxt) { }

    // The gate method: always run this pass.
    bool gate(function *fun) override {
        return true;
    }

    // The execute method: this is called for every function.
    unsigned int execute(function *fun) override {
        // Print the function name.
        fprintf(stderr, "Processing function: %s\n", function_name(fun));

        // Count basic blocks.
        int basic_block_count = 0;
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            basic_block_count++;
        }
        fprintf(stderr, "Number of basic blocks: %d\n", basic_block_count);

        // Count GIMPLE statements.
        int gimple_stmt_count = 0;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb);
                 !gsi_end_p(gsi);
                 gsi_next(&gsi)) {
                gimple_stmt_count++;
            }
        }
        fprintf(stderr, "Number of GIMPLE statements: %d\n", gimple_stmt_count);

        return 0;
    }
};

// Factory function to create an instance of the pass.
gimple_opt_pass *make_tree_my_pass(gcc::context *ctxt) {
    return new my_pass(ctxt);
}

} 
