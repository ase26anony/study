This code appears to be from a compiler's scheduler implementation (likely GCC), specifically cleaning up saved scheduling state. Here's what each part does:

1. **`if (targetm.sched.free_sched_context)`** - If the target machine has a function to free scheduler context, call it with the saved backend data.

2. **`if (current_sched_info->restore_state)`** - If the current scheduling info has a restore_state function, free the frontend saved data.

3. **`for (i = 0; i <= max_insn_queue_index; i++)`** - Loop through all instruction queue entries and free the INSN_LIST linked lists.

4. **`free (save->insn_queue)`** - Free the array that holds the instruction queues.

5. **`free (save->curr_state)`** - Free the current state data structure.

6. **`free (save->ready.vec)`** - Free the ready list vector (instructions ready for scheduling).

7. **`free (save)`** - Finally free the main save structure itself.

This is cleanup code that would be called when discarding saved scheduling state, perhaps during:
- Undoing speculative scheduling
- Cleaning up after failed scheduling attempts
- Restoring state after trying different scheduling decisions

The code follows a careful order of freeing nested structures before freeing the containers that hold them.
