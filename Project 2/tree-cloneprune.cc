/*  tree-cloneprune.cc  –  SPO600 Stage‑2 pass
    Detects duplicate (“clone”) functions and prints
      PRUNE:  <clone>
      NOPRUNE:<original>
    plus a summary at the end of the TU.

   Integration steps (in‑tree build):
     1.  Save this file as  gcc/tree-cloneprune.cc
     2.  gcc/Makefile.in  :  add  tree-cloneprune.o  to OBJS
     3.  gcc/tree-pass.h  :  extern gimple_opt_pass *make_pass_cloneprune(gcc::context *);
     4.  gcc/passes.def   :  NEXT_PASS(pass_cloneprune)   (after a late GIMPLE pass)
*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#undef  optimize                     /* avoid macro clash from options.h */

#include "tree-pass.h"
#include "context.h"

/* tree.h MUST precede gimple.h */
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

/*======================================================================*/
/*  Pass metadata                                                       */
/*======================================================================*/
namespace
{

const pass_data cloneprune_pass_data =
{
    GIMPLE_PASS,              /* type                    */
    "cloneprune",             /* -fdump-tree-cloneprune  */
    OPTGROUP_NONE, TV_NONE,
    0, 0, 0, 0                /* props req/prov/dest/flags */
};

/*======================================================================*/
/*  Pass implementation                                                 */
/*======================================================================*/
class pass_cloneprune : public gimple_opt_pass
{
public:
    pass_cloneprune (gcc::context *ctxt)
        : gimple_opt_pass (cloneprune_pass_data, ctxt) {}

    unsigned int execute (function *fun) override
    {
        if (!dump_file || !fun)
            return 0;

        const std::string fname =
            IDENTIFIER_POINTER (DECL_NAME (fun->decl));

        /* ---- metrics (optional) ----------------------------------- */
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

        /* ---- signature & clone logic ------------------------------ */
        const std::string sig = generate_signature (fun);

        if (seen_sig_to_name.count (sig))
        {
            fprintf (dump_file, "PRUNE:  %s  (dup of %s)\n\n",
                     fname.c_str (), seen_sig_to_name[sig].c_str ());
            clone_map[ seen_sig_to_name[sig] ].push_back (fname);
        }
        else
        {
            seen_sig_to_name[sig] = fname;
            fprintf (dump_file, "NOPRUNE:%s\n\n", fname.c_str ());
        }

        /* emit summary once we reach main() */
        if (fname == "main")
            emit_summary ();

        return 0;
    }

private:
    /* signature → first‑seen function  */
    static std::unordered_map<std::string,std::string> seen_sig_to_name;
    /* original → vector<clones>        */
    static std::unordered_map<std::string,std::vector<std::string>> clone_map;

    /* -------------------------------------------------------------- */
    std::string generate_signature (function *fun)
    {
        std::string sig;
        basic_block bb;
        FOR_EACH_BB_FN (bb, fun)
        {
            sig += '|';
            for (gimple_stmt_iterator gsi = gsi_start_bb (bb);
                 !gsi_end_p (gsi); gsi_next (&gsi))
            {
                gimple *st = gsi_stmt (gsi);
                sig += gimple_code_name[ gimple_code (st) ];

                if (is_gimple_call (st))
                    sig += std::to_string
                           (gimple_call_num_args (as_a<gcall *>(st)));
                else if (is_gimple_assign (st))
                    sig += std::to_string (gimple_assign_rhs_code (st));
            }
        }
        return sig;
    }

    /* -------------------------------------------------------------- */
    void emit_summary ()
    {
        fprintf (dump_file,
                 "\n=== CLONEPRUNE SUMMARY =========================\n");
        fprintf (dump_file, "Unique signatures : %zu\n",
                 seen_sig_to_name.size ());

        size_t total_clones = 0;
        for (auto &kv : clone_map) total_clones += kv.second.size ();
        fprintf (dump_file, "Total clones       : %zu\n", total_clones);

        for (auto &kv : clone_map)
        {
            fprintf (dump_file, "Original: %s  -> %zu clone(s)\n",
                     kv.first.c_str (), kv.second.size ());
            for (auto &c : kv.second)
                fprintf (dump_file, "    %s\n", c.c_str ());
        }
        fprintf (dump_file,
                 "================================================\n\n");
    }
};

/* static maps */
std::unordered_map<std::string,std::string>
    pass_cloneprune::seen_sig_to_name;
std::unordered_map<std::string,std::vector<std::string>>
    pass_cloneprune::clone_map;

} // anonymous namespace

/*======================================================================*/
/*  Factory required by tree-pass.h / passes.def                         */
/*======================================================================*/
gimple_opt_pass *
make_pass_cloneprune (gcc::context *ctxt)
{
    return new pass_cloneprune (ctxt);
}

