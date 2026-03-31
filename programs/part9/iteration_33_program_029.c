// 1. Free target-specific scheduling context if the target machine has a hook for it
if (targetm.sched.free_sched_context)
    targetm.sched.free_sched_context (save->be_saved_data);

// 2. Free frontend saved data if there's a restore_state function
if (current_sched_info->restore_state)
    free (save->fe_saved_data);

// 3. Free instruction lists in the instruction queue
for (i = 0; i <= max_insn_queue_index; i++)
    free_INSN_LIST_list (&save->insn_queue[i]);

// 4. Free the instruction queue array itself
free (save->insn_queue);

// 5. Free the current state
free (save->curr_state);

// 6. Free the ready list vector
free (save->ready.vec);

// 7. Finally free the save structure itself
free (save);
