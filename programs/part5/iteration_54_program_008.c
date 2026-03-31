! insn_references_resource_p (next_trial, &set, true)      // Doesn't read resources being set
! insn_sets_resource_p (next_trial, &set, true)           // Doesn't write resources being set  
! insn_sets_resource_p (next_trial, &needed, true)        // Doesn't write needed resources
