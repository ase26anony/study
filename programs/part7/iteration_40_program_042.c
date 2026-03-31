// 1. Skip if not a conditional statement
if (!cmp_stmt || gimple_code (cmp_stmt) != GIMPLE_COND)
  continue;

// 2. Get comparison operands
cmp_rhs = gimple_cond_rhs (cmp_stmt);
cmp_lhs = gimple_cond_lhs (cmp_stmt);

// 3. Check RHS is constant 0 or 1
if (!TREE_CONSTANT (cmp_rhs)
    || !(integer_zerop (cmp_rhs) || integer_onep (cmp_rhs)))
  continue;

// 4. Check LHS is SSA name
if (TREE_CODE (cmp_lhs) != SSA_NAME)
  continue;

// 5. Check basic block annotation
if (!is_bb_annotated (bb, annotated_bb))
  continue;

// 6. Trace back through assignments to find original definition
def_stmt = SSA_NAME_DEF_STMT (cmp_lhs);
while (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN
       && gimple_assign_single_p (def_stmt)
       && TREE_CODE (gimple_assign_rhs1 (def_stmt)) == SSA_NAME)
  def_stmt = SSA_NAME_DEF_STMT (gimple_assign_rhs1 (def_stmt));

// 7. Skip if no definition found
if (!def_stmt)
  continue;

// 8. Check if definition is a phi node
gphi *phi_stmt = dyn_cast <gphi *> (def_stmt);
if (!phi_stmt)
  continue;

// 9. Iterate through successor edges
FOR_EACH_EDGE (e, ei, bb->succs)
