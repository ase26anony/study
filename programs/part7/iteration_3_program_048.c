This code appears to be from the GCC compiler's scheduler, specifically cleaning up saved scheduling state. Here's what each part does:

1. **`if (targetm.sched.free_sched_context)`** - If the target machine has a function to free scheduler context, call it with `save->be_saved_data` (backend saved data).

2. **`if (current_sched_info->restore_state)`** - If the current scheduler info has a restore_state function, free the `save->fe_saved_data` (frontend saved data).

3. **`for (i = 0; i <= max_insn_queue_index; i++)`** - Loop through all instruction queue indices and free the INSN_LIST linked lists at each position.

4. **`free (save->insn_queue)`** - Free the array that holds the instruction queues.

5. **`free (save->curr_state)`** - Free the current state data structure.

6. **`free (save->ready.vec)`** - Free the vector used for the ready list (instructions ready to be scheduled).

7. **`free (save)`** - Finally, free the main save structure itself.

This is typical cleanup code for a complex data structure with multiple nested allocations. The order of freeing generally goes from deepest nested structures outward to the main container.
