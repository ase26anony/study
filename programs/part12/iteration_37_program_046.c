// Helper function to call with variable number of arguments
static rtx call_with_n_args(rtx (*fcn)(), rtx *ops, int n) {
    switch (n) {
        case 0: return fcn();
        case 1: return fcn(ops[0].value);
        case 2: return fcn(ops[0].value, ops[1].value);
        // ... etc ...
        default: 
            // For many arguments, consider a different approach
            // or code generation
            break;
    }
}
