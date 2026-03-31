   if (!LABEL_P (insn) && !NOTE_P (insn) && !DEBUG_INSN_P (insn)
       && modified_in_p (test_expr, insn))
