// If the target machine has a function to free scheduler context, call it
if (targetm.sched.free_sched_context)
    targetm.sched.free_sched_context (save->be_saved_data);

// If there's a function to restore state, free the frontend saved data
if (current_sched_info->restore_state)
    free (save->fe_saved_data);

// Free instruction lists in the instruction queue
for (i = 0; i <= max_insn_queue_index; i++)
    free_INSN_LIST_list (&save->insn_queue[i]);

// Free the instruction queue array itself
free (save->insn_queue);

// Free the current state
free (save->curr_state);

// Free the ready list vector
free (save->ready.vec);

// Finally, free the save structure itself
free (save);
