// Check if cmp_stmt exists and is a GIMPLE_COND statement
if (!cmp_stmt || gimple_code (cmp_stmt) != GIMPLE_COND)
  continue;

// Get the right and left hand sides of the condition
cmp_rhs = gimple_cond_rhs (cmp_stmt);
cmp_lhs = gimple_cond_lhs (cmp_stmt);

// Check if RHS is constant (0 or 1)
if (!TREE_CONSTANT (cmp_rhs)
    || !(integer_zerop (cmp_rhs) || integer_onep (cmp_rhs)))
  continue;

// Check if LHS is an SSA_NAME
if (TREE_CODE (cmp_lhs) != SSA_NAME)
  continue;

// Check if basic block has some annotation
if (!is_bb_annotated (bb, annotated_bb))
  continue;

// Follow definition chain through assignments
def_stmt = SSA_NAME_DEF_STMT (cmp_lhs);
while (def_stmt && gimple_code (def_stmt) == GIMPLE_ASSIGN
       && gimple_assign_single_p (def_stmt)
       && TREE_CODE (gimple_assign_rhs1 (def_stmt)) == SSA_NAME)
  def_stmt = SSA_NAME_DEF_STMT (gimple_assign_rhs1 (def_stmt));

if (!def_stmt)
  continue;

// Check if the definition is a phi node
gphi *phi_stmt = dyn_cast <gphi *> (def_stmt);
if (!phi_stmt)
  continue;

// Iterate through successor edges of the basic block
FOR_EACH_EDGE (e, ei, bb->succs)
