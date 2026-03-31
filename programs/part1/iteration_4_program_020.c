// Check if test_expr is modified in the specified range of then_bb
for each instruction insn in range(BB_HEAD(then_bb), then_last_head):
    if insn is not (label OR note OR debug_insn):
        if test_expr is modified by insn:
            return UNSAFE
return SAFE
