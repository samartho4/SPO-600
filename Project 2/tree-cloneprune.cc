#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "basic-block.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include <unordered_map>
#include <string>

namespace {

const pass_data cloneprune_pass_data = {
    GIMPLE_PASS,                     // Pass type
    "cloneprune",                   // Name (used for -fdump-tree-cloneprune)
    OPTGROUP_NONE, TV_NONE,         // Opt group and timing var
    0, 0, 0, 0                       // Properties required/provided/destroyed/flags
};

class pass_cloneprune : public gimple_opt_pass {
public:
    pass_cloneprune(gcc::context *ctxt)
        : gimple_opt_pass(cloneprune_pass_data, ctxt) {}

    unsigned int execute(function *fun) override {
        if (!dump_file || !fun) return 0;

        std::string name = IDENTIFIER_POINTER(DECL_NAME(fun->decl));
        std::string sig = generate_signature(fun);

        if (seen_signatures.count(sig)) {
            // Already seen this structure
            fprintf(dump_file, "PRUNE: %s\n", name.c_str());
        } else {
            seen_signatures[sig] = name;
            fprintf(dump_file, "NOPRUNE: %s\n", name.c_str());
        }

        return 0;
    }

private:
    static std::unordered_map<std::string, std::string> seen_signatures;

    std::string generate_signature(function *fun) {
        std::string sig;
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);
                sig += gimple_code_name[gimple_code(stmt)];
            }
        }
        return sig;
    }
};

// Initialize the static variable
std::unordered_map<std::string, std::string> pass_cloneprune::seen_signatures;

} // anonymous namespace

// Factory function for GCC to create the pass
gimple_opt_pass *make_pass_cloneprune(gcc::context *ctxt) {
    return new pass_cloneprune(ctxt);
}
