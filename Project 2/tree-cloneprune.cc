#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "basic-block.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "cgraph.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>

namespace {

const pass_data cloneprune_pass_data = {
    GIMPLE_PASS,                    // Pass type
    "cloneprune",                   // Name (used for -fdump-tree-cloneprune)
    OPTGROUP_NONE, TV_NONE,         // Opt group and timing var
    0, 0, 0, 0                      // Properties required/provided/destroyed/flags
};

class pass_cloneprune : public gimple_opt_pass {
public:
    pass_cloneprune(gcc::context *ctxt)
        : gimple_opt_pass(cloneprune_pass_data, ctxt) {}

    unsigned int execute(function *fun) override {
        if (!dump_file || !fun) return 0;
        
        std::string name = IDENTIFIER_POINTER(DECL_NAME(fun->decl));
        std::string sig = generate_enhanced_signature(fun);
        
        // Output detailed function metrics
        fprintf(dump_file, "Function: %s\n", name.c_str());
        fprintf(dump_file, "  Blocks: %d\n", count_basic_blocks(fun));
        fprintf(dump_file, "  Statements: %d\n", count_statements(fun));
        
        // Clone detection logic
        if (seen_signatures.count(sig)) {
            // Already seen this structure - potential clone
            fprintf(dump_file, "PRUNE: %s (matches %s)\n", 
                    name.c_str(), seen_signatures[sig].c_str());
            
            // Track clone relationships
            std::string original = seen_signatures[sig];
            clone_relationships[original].push_back(name);
        } else {
            // First time seeing this structure
            seen_signatures[sig] = name;
            fprintf(dump_file, "NOPRUNE: %s\n", name.c_str());
        }
        
        // Check if this is likely the last function we'll process
        // and generate a summary report
        if (is_main_function(name) && dump_file) {
            generate_summary_report();
            detect_compiler_generated_clones();
        }
        
        return 0;
    }

private:
    // Store seen function signatures for clone detection
    static std::unordered_map<std::string, std::string> seen_signatures;
    
    // Track clone relationships (original -> list of clones)
    static std::unordered_map<std::string, std::vector<std::string>> clone_relationships;
    
    // Enhanced signature generation that captures more details
    std::string generate_enhanced_signature(function *fun) {
        std::string sig;
        basic_block bb;
        
        // Add basic block count as a prefix
        int bb_count = 0;
        FOR_EACH_BB_FN(bb, fun) {
            bb_count++;
        }
        sig += "B" + std::to_string(bb_count);
        
        // Process each basic block
        FOR_EACH_BB_FN(bb, fun) {
            // Add block marker
            sig += "|";
            
            // Process statements in this block
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple *stmt = gsi_stmt(gsi);
                // Add gimple code name
                sig += gimple_code_name[gimple_code(stmt)];
                
                // Add more detail for important statement types
                if (is_gimple_call(stmt)) {
                    // Count number of arguments to the call
                    int arg_count = gimple_call_num_args(as_a<gcall *>(stmt));
                    sig += std::to_string(arg_count);
                } else if (is_gimple_assign(stmt)) {
                    // Add assignment operation code
                    sig += std::to_string(gimple_assign_rhs_code(stmt));
                }
            }
        }
        
        return sig;
    }
    
    // Count basic blocks in a function
    int count_basic_blocks(function *fun) {
        int count = 0;
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            count++;
        }
        return count;
    }
    
    // Count statements in a function
    int count_statements(function *fun) {
        int count = 0;
        basic_block bb;
        FOR_EACH_BB_FN(bb, fun) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                count++;
            }
        }
        return count;
    }
    
    // Check if this is the main function (for summary report trigger)
    bool is_main_function(const std::string &name) {
        return (name == "main");
    }
    
    // Generate summary report of detected clones
    void generate_summary_report() {
        fprintf(dump_file, "\n=== CLONEPRUNE SUMMARY REPORT ===\n");
        
        // Count total functions and clones
        int total_functions = seen_signatures.size() + 
                             (int)clone_relationships.size();
        int total_clones = 0;
        
        for (const auto &entry : clone_relationships) {
            total_clones += entry.second.size();
        }
        
        fprintf(dump_file, "Total unique functions: %d\n", seen_signatures.size());
        fprintf(dump_file, "Total clones detected: %d\n", total_clones);
        
        if (total_clones > 0) {
            fprintf(dump_file, "\nClone groups:\n");
            
            for (const auto &entry : clone_relationships) {
                const std::string &original = entry.first;
                const std::vector<std::string> &clones = entry.second;
                
                fprintf(dump_file, "Original: %s\n", original.c_str());
                for (const auto &clone : clones) {
                    fprintf(dump_file, "  - Clone: %s\n", clone.c_str());
                }
                fprintf(dump_file, "\n");
            }
        }
        
        fprintf(dump_file, "=== END OF SUMMARY REPORT ===\n\n");
    }
    
    // Detect compiler-generated function clones
    void detect_compiler_generated_clones() {
        fprintf(dump_file, "=== COMPILER-GENERATED CLONES ===\n");
        
        std::unordered_map<std::string, std::vector<std::string>> compiler_clones;
        
        // Pattern for detecting compiler-generated clones
        std::regex clone_pattern(R"(^(.*)\.(default|resolver|variant\.\d+|clone\.\d+|constprop\.\d+)$)");
        
        struct cgraph_node *node;
        FOR_EACH_FUNCTION(node) {
            const char *name_cstr = IDENTIFIER_POINTER(DECL_NAME(node->decl));
            std::string name(name_cstr);
            
            std::smatch match;
            if (std::regex_match(name, match, clone_pattern)) {
                std::string base_name = match[1];
                compiler_clones[base_name].push_back(name);
            }
        }
        
        // Output compiler-generated clone groups
        for (const auto &entry : compiler_clones) {
            const std::string &base = entry.first;
            const std::vector<std::string> &clones = entry.second;
            
            fprintf(dump_file, "Clone group: %s (%zu variants)\n", base.c_str(), clones.size());
            for (const auto &clone : clones) {
                fprintf(dump_file, "  - %s\n", clone.c_str());
            }
            fprintf(dump_file, "\n");
        }
        
        fprintf(dump_file, "=== END OF COMPILER CLONE REPORT ===\n\n");
    }
};

// Initialize static members
std::unordered_map<std::string, std::string> pass_cloneprune::seen_signatures;
std::unordered_map<std::string, std::vector<std::string>> pass_cloneprune::clone_relationships;

} // anonymous namespace

// Factory function for GCC to create the pass
gimple_opt_pass *make_pass_cloneprune(gcc::context *ctxt) {
    return new pass_cloneprune(ctxt);
}

// Required plugin initialization
int plugin_is_GPL_compatible = 1;

int plugin_init(struct plugin_name_args *plugin_info,
               struct plugin_gcc_version *version) {
    // Register our new pass
    register_callback(plugin_info->base_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,
                      (void *) &make_pass_cloneprune);
    
    return 0;
}
