/*  tree-cloneprune.cc  –  SPO600 Stage‑2 pass
    Detects function clones and prints PRUNE / NOPRUNE diagnostics.

   Compile‑time integration (in‑tree):
     *  gcc/Makefile.in   :  add  tree-cloneprune.o  to OBJS list
     *  gcc/tree-pass.h   :  extern gimple_opt_pass *make_pass_cloneprune(gcc::context *);
     *  gcc/passes.def    :  NEXT_PASS(pass_cloneprune)
*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree-pass.h"
#include "context.h"

/*  Header order is critical: tree.h BEFORE gimple.h  */
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
#include <regex>

namespace
{

/*----------------------------------------------------------*
 |  Pass metadata                                            |
 *----------------------------------------------------------*/
const pass_data cloneprune_pass_data =
{
    GIMPLE_PASS,          /* type                               */
    "cloneprune",         /* -fdump-tree-cloneprune             */
    OPTGROUP_NONE,
    TV_NONE,
    /* properties  */ 0, 0, 0, 0
};

/*----------------------------------------------------------*
 |  Pass class                                               |
 *----------------------------------------------------------*/
class pass_cloneprune : public gimple_opt_pass
{
public:
    pass_cloneprune (gcc::context *ctxt)
        : gimple_opt_pass (cloneprune_pass_data, ctxt) {}

    /* run on every function */
    unsigned int execute (function *fun) override
    {
        if (!dump_file || !fun)
            return 0;

        const std::string fname =
            IDENTIFIER_POINTER (DECL_NAME (fun->decl));

        /* --- metrics ------------------------------------ */
        int bb_cnt = 0, stmt_cnt = 0;
        basic_block bb;
        FOR_EACH_BB_FN (bb, fun)
        {
            ++bb_cnt;
            for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
                 !gsi_end_p (gsi); gsi_next (&gsi))
                ++stmt_cnt;
        }

        fprintf (dump_file,
                 "Function: %s  (blocks=%d, stmts=%d)\n",
                 fname.c_str (), bb_cnt, stmt_cnt);

        /* --- signature ---------------------------------- */
        const std::string sig = generate_signature (fun);

        if (seen_sigs.count (sig))
        {
            fprintf (dump_file, "PRUNE: %s  (dup of %s)\n\n",
                     fname.c_str (), seen_sigs[sig].c_str ());
            clone_map[ seen_sigs[sig] ].push_back (fname);
        }
        else
        {
            seen_sigs[sig] = fname;
            fprintf (dump_file, "NOPRUNE: %s\n\n", fname.c_str ());
        }

        /* if this is main(), emit summary once */
        if (fname == "main")
            emit_summary ();

        return 0;
    }

private:
    /* map<signature, first‑function‑name>  */
    static std::unordered_map<std::string, std::string> seen_sigs;
    /* map<original, vector<clones>>        */
    static std::unordered_map<std::string, std::vector<std::string>> clone_map;

    /* ---------------------------------------------------- */
    std::string generate_signature (function *fun)
    {
        std::string sig;

        basic_block bb;
        FOR_EACH_BB_FN (bb, fun)
        {
            sig += '|';                     /* block delimiter */

            for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
                 !gsi_end_p (gsi); gsi_next (&gsi))
            {
                gimple *stmt = gsi_stmt (gsi);
                sig += gimple_code_name[ gimple_code (stmt) ];

                /* extra detail for calls & assigns */
                if (is_gimple_call (stmt))
                {
                    sig += std::to_string
                           (gimple_call_num_args (as_a<gcall *> (stmt)));
                }
                else if (is_gimple_assign (stmt))
                {
                    sig += std::to_string
                           (gimple_assign_rhs_code (stmt));
                }
            }
        }
        return sig;
    }

    /* ---------------------------------------------------- */
    void emit_summary ()
    {
        fprintf (dump_file,
                 "\n=== CLONEPRUNE SUMMARY =========================\n");
        fprintf (dump_file, "Unique signatures : %zu\n", seen_sigs.size ());
        size_t total_clones = 0;
        for (auto &kv : clone_map) total_clones += kv.second.size ();
        fprintf (dump_file, "Total clones       : %zu\n", total_clones);

        for (auto &kv : clone_map)
        {
            fprintf (dump_file, "Original: %s  ->  %zu clone(s)\n",
                     kv.first.c_str (), kv.second.size ());
            for (auto &c : kv.second)
                fprintf (dump_file, "    %s\n", c.c_str ());
        }
        fprintf (dump_file,
                 "================================================\n\n");
    }
};

/* static data definitions */
std::unordered_map<std::string, std::string>
    pass_cloneprune::seen_sigs;
std::unordered_map<std::string, std::vector<std::string>>
    pass_cloneprune::clone_map;

} // anonymous namespace

/*----------------------------------------------------------*
 |  Factory required by tree-pass.h / passes.def             |
 *----------------------------------------------------------*/
gimple_opt_pass *
make_pass_cloneprune (gcc::context *ctxt)
{
    return new pass_cloneprune (ctxt);
}
