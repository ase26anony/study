!insn_references_resource_p(next_trial, &set, true)    // Doesn't use resources set by jump
!insn_sets_resource_p(next_trial, &set, true)          // Doesn't set resources needed by jump
!insn_sets_resource_p(next_trial, &needed, true)       // Doesn't clobber needed resources
