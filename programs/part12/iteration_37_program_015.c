// Build arguments dynamically
std::vector<value_type> args;
for (int i = 0; i < arg_count; i++) {
    args.push_back(ops[i].value);
}

// Call function with appropriate number of arguments
// This would require a different dispatch mechanism
