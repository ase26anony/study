if (slots_to_fill != slots_filled
    && trial
    && jump_to_label_p (trial)
    && simplejump_p (trial)
    && (next_trial = next_active_insn (JUMP_LABEL_AS_INSN (trial))) != 0
    && ! (NONJUMP_INSN_P (next_trial)
          && GET_CODE (PATTERN (next_trial)) == SEQUENCE)
    && !JUMP_P (next_trial)
    && ! insn_references_resource_p (next_trial, &set, true)
    && ! insn_sets_resource_p (next_trial, &set, true)
    && ! insn_sets_resource_p (next_trial, &needed, true)
    && ! (maybe_never && may_trap_or_fault_p (PATTERN (next_trial)))
    && (next_trial = try_split (PATTERN (next_trial), next_trial, 0))
    && eligible_for_delay (insn, slots_filled, next_trial, flags)
    && ! can_throw_internal (trial))
