This code appears to be from a compiler's scheduler implementation (likely GCC), specifically cleaning up saved scheduling state. Here's what each part does:

1. **`if (targetm.sched.free_sched_context)`** - If the target machine has a function to free scheduler context, call it with the saved backend data.

2. **`if (current_sched_info->restore_state)`** - If the current scheduler info has a restore_state function, free the frontend saved data.

3. **`for (i = 0; i <= max_insn_queue_index; i++)`** - Loop through the instruction queue array and free each instruction list.

4. **`free (save->insn_queue)`** - Free the array that holds the instruction queues.

5. **`free (save->curr_state)`** - Free the current state data.

6. **`free (save->ready.vec)`** - Free the ready list vector.

7. **`free (save)`** - Finally, free the main save structure itself.

This is a classic cleanup pattern where resources are freed in reverse order of allocation (from deepest nested structures to the main container). The code handles both target-specific and generic scheduler data cleanup.
