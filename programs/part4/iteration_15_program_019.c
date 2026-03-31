! insn_references_resource_p (next_trial, &set, true)      // Doesn't read resources set by insn
! insn_sets_resource_p (next_trial, &set, true)           // Doesn't write resources set by insn  
! insn_sets_resource_p (next_trial, &needed, true)        // Doesn't write resources needed by insn
