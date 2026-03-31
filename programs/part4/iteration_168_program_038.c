if (slots_to_fill != slots_filled  // Still have delay slots to fill
    && trial  // trial instruction exists
    && jump_to_label_p (trial)  // trial is a jump to a label
    && simplejump_p (trial)  // trial is a simple unconditional jump
    && (next_trial = next_active_insn (JUMP_LABEL_AS_INSN (trial))) != 0  // Get instruction after jump target
    && ! (NONJUMP_INSN_P (next_trial)  // next_trial is not a complex insn with SEQUENCE
          && GET_CODE (PATTERN (next_trial)) == SEQUENCE)
    && !JUMP_P (next_trial)  // next_trial is not a jump
    && ! insn_references_resource_p (next_trial, &set, true)  // Doesn't reference resources in 'set'
    && ! insn_sets_resource_p (next_trial, &set, true)  // Doesn't set resources in 'set'
    && ! insn_sets_resource_p (next_trial, &needed, true)  // Doesn't set resources in 'needed'
    && ! (maybe_never && may_trap_or_fault_p (PATTERN (next_trial)))  // Not potentially trapping
    && (next_trial = try_split (PATTERN (next_trial), next_trial, 0))  // Can be split if needed
    && eligible_for_delay (insn, slots_filled, next_trial, flags)  // Eligible for delay slot
    && ! can_throw_internal (trial))  // trial doesn't throw exceptions
