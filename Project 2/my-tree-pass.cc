include <unordered_map>

static std::unordered_map<std::string, std::string> function_signatures;

std::string generate_signature(function *fun) {
    std::hash<std::string> hasher;
    size_t combined_hash = 0;
    basic_block bb;
    FOR_EACH_BB_FN(bb, fun) {
        for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
            gimple *stmt = gsi_stmt(gsi);
            combined_hash ^= hasher(gimple_code_name[gimple_code(stmt)]) + 0x9e3779b9 + (combined_hash << 6) + (combined_hash >> 2);
        }
    }
    return std::to_string(combined_hash);
}

unsigned int pass_ctyler::execute(function *fun) {
    if (!fun || !dump_file) 
        return 0;

    std::string func_name = IDENTIFIER_POINTER(DECL_NAME(fun->decl));
    std::string signature = generate_signature(fun);

    if (function_signatures.find(signature) != function_signatures.end()) {
        fprintf(dump_file, "PRUNE: %s (Duplicate of %s)\n", func_name.c_str(), function_signatures[signature].c_str());
    } else {
        function_signatures[signature] = func_name;
        fprintf(dump_file, "NOPRUNE: %s\n", func_name.c_str());
    }

    return 0;
}
