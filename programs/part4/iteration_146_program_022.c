if (op_prio (op0) < op_prio (node))
  pp_left_paren (pp);
dump_generic_node (pp, op0, spc, flags, false);
if (op_prio (op0) < op_prio (node))
  pp_right_paren (pp);
