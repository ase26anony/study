// Extract first argument if it exists, otherwise NULL_TREE
tree arg0 = (call_expr_nargs (t) > 0
             ? CALL_EXPR_ARG (t, 0)
             : NULL_TREE);

// Extract second argument if it exists, otherwise NULL_TREE  
tree arg1 = (call_expr_nargs (t) > 1
             ? CALL_EXPR_ARG (t, 1)
             : NULL_TREE);

// Check if this function call returns an integer-valued real number
return integer_valued_real_call_p (get_call_combined_fn (t),
                                   arg0, arg1, depth);
