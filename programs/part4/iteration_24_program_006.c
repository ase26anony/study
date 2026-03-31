slots_to_fill != slots_filled  // There are still delay slots to fill
&& trial                       // 'trial' instruction exists
&& jump_to_label_p (trial)     // 'trial' is a jump to a label
&& simplejump_p (trial)        // 'trial' is a simple unconditional jump
