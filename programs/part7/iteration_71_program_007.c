// If the target machine has a custom scheduler context cleanup function, call it
if (targetm.sched.free_sched_context)
    targetm.sched.free_sched_context (save->be_saved_data);

// If the scheduler info has a restore_state function, free the frontend saved data
if (current_sched_info->restore_state)
    free (save->fe_saved_data);

// Clean up the instruction queue arrays
for (i = 0; i <= max_insn_queue_index; i++)
    free_INSN_LIST_list (&save->insn_queue[i]);

// Free the instruction queue array itself
free (save->insn_queue);

// Free other allocated scheduler state
free (save->curr_state);
free (save->ready.vec);

// Finally free the main scheduler save structure
free (save);
