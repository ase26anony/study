   op0 = TREE_OPERAND (node, 0);  // Get the array expression
   if (op_prio (op0) < op_prio (node))  // Check operator precedence
     pp_left_paren (pp);  // Add '(' if needed
   dump_generic_node (pp, op0, spc, flags, false);  // Print array expression
   if (op_prio (op0) < op_prio (node))
     pp_right_paren (pp);  // Add ')' if needed
