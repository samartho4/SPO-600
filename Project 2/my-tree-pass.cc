/*  tree-my-pass.cc – counts BBs and GIMPLE stmts for every function  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#undef  optimize                         // avoid options.h macro

#include "tree-pass.h"
#include "context.h"

#include "tree.h"
#include "tree-core.h"
#include "basic-block.h"
#include "gimple.h"
#include "gimple-iterator.h"

namespace {

const pass_data my_pass_data = {
    GIMPLE_PASS,
    "tree-my-pass",          // -fdump-tree-my-pass
    OPTGROUP_NONE, TV_TREE_OPS,
    0, 0, 0,                  // props required / provided / destroyed
    0,                        // todo_flags_start
    0                         // todo_flags_finish  (field 9!)
};

class my_pass : public gimple_opt_pass {
public:
    my_pass (gcc::context *ctxt) : gimple_opt_pass (my_pass_data, ctxt) {}

    bool gate (function *) override { return true; }

    unsigned int execute (function *fun) override {
        fprintf (stderr, "Processing function: %s\n", function_name (fun));
        int bb_cnt = 0, stmt_cnt = 0;
        basic_block bb;
        FOR_EACH_BB_FN (bb, fun) {
            ++bb_cnt;
            for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
                 !gsi_end_p (gsi); gsi_next (&gsi))
                ++stmt_cnt;
        }
        fprintf (stderr, "  Basic blocks : %d\n", bb_cnt);
        fprintf (stderr, "  GIMPLE stmts : %d\n\n", stmt_cnt);
        return 0;
    }
};

} // anon

gimple_opt_pass *make_tree_my_pass (gcc::context *ctxt)
{ return new my_pass (ctxt); }
