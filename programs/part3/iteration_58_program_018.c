This code appears to be from the GCC compiler's scheduler module, specifically cleaning up saved scheduling state. Here's what each part does:

1. **`if (targetm.sched.free_sched_context)`** - If the target machine has a function to free scheduler context, call it with `save->be_saved_data` (backend saved data).

2. **`if (current_sched_info->restore_state)`** - If the current scheduling info has a restore_state function, free the `save->fe_saved_data` (frontend saved data).

3. **`for (i = 0; i <= max_insn_queue_index; i++)`** - Loop through all instruction queue entries and free the INSN_LIST linked lists.

4. **`free (save->insn_queue)`** - Free the array that holds the instruction queues.

5. **`free (save->curr_state)`** - Free the current state data structure.

6. **`free (save->ready.vec)`** - Free the ready list vector (instructions ready for scheduling).

7. **`free (save)`** - Finally, free the main save structure itself.

This is a cleanup routine that's called when discarding saved scheduling state, likely during:
- Undoing speculative scheduling
- Rolling back to a previous scheduling point
- Cleaning up after scheduling is complete

The structure being freed (`save`) contains both frontend and backend scheduling state, instruction queues, current state, and ready lists - essentially everything needed to restore the scheduler to a previous point in time.
