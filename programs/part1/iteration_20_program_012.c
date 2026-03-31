while (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN
       && gimple_assign_single_p (def_stmt)
       && TREE_CODE (gimple_assign_rhs1 (def_stmt)) == SSA_NAME)
  def_stmt = SSA_NAME_DEF_STMT (gimple_assign_rhs1 (def_stmt));
