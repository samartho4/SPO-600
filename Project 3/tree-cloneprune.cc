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
#include <regex>
#include <unordered_set>

namespace {

const pass_data cloneprune_pass_data = {
    GIMPLE_PASS,
    "cloneprune",           // -fdump-tree-cloneprune
    OPTGROUP_NONE, TV_NONE,
    0, 0, 0,                 // props required / provided / destroyed
    0,                       // todo_flags_start
    0                        // todo_flags_finish
};

// Structure to store function information
struct FunctionInfo {
    std::string original_name;  // Base name without variant
    std::string full_name;      // Full function name with variant
    std::string signature;      // Function signature
    bool is_clone;              // Whether this is a clone
    bool is_resolver;           // Whether this is a resolver
};

class pass_cloneprune : public gimple_opt_pass {
public:
    pass_cloneprune(gcc::context *ctxt) : gimple_opt_pass(cloneprune_pass_data, ctxt) {}

    // Gate function - always run
    bool gate(function *) override { return true; }

    // Execute function - runs for each function
    unsigned int execute(function *fun) override {
        if (!dump_file || !fun) return 0;

        // Extract function name information
        FunctionInfo info = extract_function_info(fun);
        
        // For resolvers, skip the analysis
        if (info.is_resolver) {
            fprintf(dump_file, "INFO: Found resolver function: %s\n", info.full_name.c_str());
            return 0;
        }

        // Generate function signature that ignores superficial differences
        info.signature = generate_signature(fun);

        // Store function info for later processing
        if (info.is_clone) {
            // If we already know the original function
            if (original_functions.count(info.original_name)) {
                // Compare signatures
                if (original_functions[info.original_name].signature == info.signature) {
                    fprintf(dump_file, "PRUNE: %s\n", info.original_name.c_str());
                    clone_groups[info.original_name].push_back(info.full_name);
                } else {
                    fprintf(dump_file, "NOPRUNE: %s\n", info.original_name.c_str());
                    noprune_functions.insert(info.original_name);
                }
            } else {
                // We haven't seen the original function yet, store this clone
                pending_clones[info.original_name].push_back(info);
            }
        } else {
            // This is an original function (or we're treating it as one)
            original_functions[info.full_name] = info;
            
            // Check if we have any pending clones for this function
            if (pending_clones.count(info.full_name)) {
                for (const auto& clone_info : pending_clones[info.full_name]) {
                    if (clone_info.signature == info.signature) {
                        fprintf(dump_file, "PRUNE: %s\n", info.full_name.c_str());
                        clone_groups[info.full_name].push_back(clone_info.full_name);
                    } else {
                        fprintf(dump_file, "NOPRUNE: %s\n", info.full_name.c_str());
                        noprune_functions.insert(info.full_name);
                    }
                }
                pending_clones.erase(info.full_name);
            }
        }

        // If this is the main function, output summary
        if (info.full_name == "main") {
            output_summary();
        }

        return 0;
    }
opt_pass *clone() override {
    return new pass_cloneprune(m_ctxt);
}

private:
    // Maps to track functions
    std::unordered_map<std::string, FunctionInfo> original_functions;
    std::unordered_map<std::string, std::vector<FunctionInfo>> pending_clones;
    std::unordered_map<std::string, std::vector<std::string>> clone_groups;
    std::unordered_set<std::string> noprune_functions;

    // Extract information about the function
    FunctionInfo extract_function_info(function *fun) {
        FunctionInfo info;
        info.full_name = IDENTIFIER_POINTER(DECL_NAME(fun->decl));
        
        // Use regex to check if this is a clone (has .variant suffix)
        std::regex clone_pattern("(.+)\\.[^.]+$");
        std::smatch match;
        if (std::regex_search(info.full_name, match, clone_pattern)) {
            info.original_name = match[1];
            info.is_clone = true;
            info.is_resolver = info.full_name.find(".resolver") != std::string::npos;
        } else {
            info.original_name = info.full_name;
            info.is_clone = false;
            info.is_resolver = false;
        }
        
        return info;
    }

    // Generate a signature for a function that ignores superficial differences
    std::string generate_signature(function *fun) {
        std::string signature;
        
        // Track basic block count
        int bb_count = 0;
        
        // Iterate through basic blocks
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            signature += "|BB" + std::to_string(bb_count++) + ":";
            
            // Iterate through GIMPLE statements
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);
                
                // Add statement code to signature
                signature += gimple_code_name[gimple_code(stmt)];
                
                // Add specific details for some statement types
                switch (gimple_code(stmt)) {
                    case GIMPLE_ASSIGN:
                        // For assignments, note the operation but not the operands
                        signature += ":" + std::to_string(gimple_assign_rhs_code(stmt));
                        break;
                    case GIMPLE_CALL:
                        // For calls, include the function name but not argument names
                        if (gimple_call_fndecl(stmt)) {
                            tree fndecl = gimple_call_fndecl(stmt);
                            const char* callee_name = IDENTIFIER_POINTER(DECL_NAME(fndecl));
                            signature += ":" + std::string(callee_name);
                        }
                        signature += ":" + std::to_string(gimple_call_num_args(stmt));
                        break;
                    case GIMPLE_COND:
                        // For conditionals, note the predicate
                        signature += ":" + std::to_string(gimple_cond_code(stmt));
                        break;
                    default:
                        break;
                }
                
                signature += ";";
            }
        }
        
        return signature;
    }

    // Output summary of clone analysis
    void output_summary() {
        fprintf(dump_file, "\n=== CLONEPRUNE SUMMARY ===\n");
        
        // Count total clones
        size_t total_clones = 0;
        for (const auto& group : clone_groups) {
            total_clones += group.second.size();
        }
        
        fprintf(dump_file, "Original functions: %zu\n", original_functions.size());
        fprintf(dump_file, "Functions with PRUNE recommendation: %zu\n", clone_groups.size());
        fprintf(dump_file, "Functions with NOPRUNE recommendation: %zu\n", noprune_functions.size());
        fprintf(dump_file, "Total clones to be pruned: %zu\n\n", total_clones);
        
        // Output detailed information about clone groups
        for (const auto& group : clone_groups) {
            fprintf(dump_file, "Original: %s -> %zu clone(s) to be pruned\n", 
                group.first.c_str(), group.second.size());
            for (const auto& clone : group.second) {
                fprintf(dump_file, "    %s\n", clone.c_str());
            }
        }
        
        // Output functions that shouldn't be pruned
        if (!noprune_functions.empty()) {
            fprintf(dump_file, "\nFunctions that shouldn't be pruned:\n");
            for (const auto& func : noprune_functions) {
                fprintf(dump_file, "    %s\n", func.c_str());
            }
        }
        
        fprintf(dump_file, "===========================\n\n");
    }
};

} // anonymous namespace

// Factory function to create an instance of the pass
gimple_opt_pass *make_pass_cloneprune(gcc::context *ctxt) {
    return new pass_cloneprune(ctxt);
}
