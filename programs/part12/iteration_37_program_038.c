// Build argument list dynamically
std::vector<ValueType> args;
for (int i = 0; i < arg_count; i++) {
    args.push_back(ops[i].value);
}
// Call function with appropriate number of arguments
return call_function_with_args(icode, args);
