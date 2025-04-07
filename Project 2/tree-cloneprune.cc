/*  tree-cloneprune.cc – emits PRUNE / NOPRUNE for function clones  */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#undef  optimize                         // avoid options.h macro
#include "tree-pass.h"
#include "context.h"
#include "tree.h"
#include "tree-core.h"
#include "function.h"
#include "basic-block.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "cgraph.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace {
const pass_data cloneprune_pass_data = {
    GIMPLE_PASS,
    "cloneprune",           // -fdump-tree-cloneprune
    OPTGROUP_NONE, TV_NONE,
    0, 0, 0,                 // props required / provided / destroyed
    0,                       // todo_flags_start
    0                        // todo_flags_finish
};

class pass_cloneprune : public gimple_opt_pass {
public:
    pass_cloneprune (gcc::context *ctxt) : gimple_opt_pass (cloneprune_pass_data, ctxt) {}
    unsigned int execute (function *fun) override {
        if (!dump_file || !fun) return 0;
        const std::string fname = IDENTIFIER_POINTER (DECL_NAME (fun->decl));
        const std::string sig   = signature (fun);
        if (sig_to_first.count (sig)) {
            fprintf (dump_file, "PRUNE:  %s (dup of %s)\n", fname.c_str (), sig_to_first[sig].c_str ());
            clones[ sig_to_first[sig] ].push_back (fname);
        } else {
            sig_to_first[sig] = fname;
            fprintf (dump_file, "NOPRUNE:%s\n", fname.c_str ());
        }
        if (fname == "main") summary ();
        return 0;
    }
private:
    static std::unordered_map<std::string,std::string> sig_to_first;
    static std::unordered_map<std::string,std::vector<std::string>> clones;
    std::string signature (function *fun) {
        std::string s;
        basic_block bb;
        FOR_EACH_BB_FN (bb, fun) {
            s += '|';
            for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
                 !gsi_end_p (gsi); gsi_next (&gsi))
                s += gimple_code_name[ gimple_code (gsi_stmt (gsi)) ];
        }
        return s;
    }
    void summary () {
        fprintf (dump_file, "\n=== CLONEPRUNE SUMMARY ===\n");
        size_t total = 0; for (auto &kv : clones) total += kv.second.size ();
        fprintf (dump_file, "Unique signatures : %zu\n", sig_to_first.size ());
        fprintf (dump_file, "Total clones       : %zu\n", total);
        for (auto &kv : clones) {
            fprintf (dump_file, "Original: %s -> %zu clone(s)\n", kv.first.c_str (), kv.second.size ());
            for (auto &c : kv.second) fprintf (dump_file, "    %s\n", c.c_str ());
        }
        fprintf (dump_file, "===========================\n\n");
    }
};
std::unordered_map<std::string,std::string> pass_cloneprune::sig_to_first;
std::unordered_map<std::string,std::vector<std::string>> pass_cloneprune::clones;
} // anon

gimple_opt_pass *make_pass_cloneprune (gcc::context *ctxt)
{ return new pass_cloneprune (ctxt); }
