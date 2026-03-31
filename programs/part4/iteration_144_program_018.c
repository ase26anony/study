This code appears to be from the GCC compiler's scheduler module, specifically cleaning up saved scheduling state. Here's what each part does:

1. **`if (targetm.sched.free_sched_context)`** - Checks if the target machine has a function to free scheduler context
   - **`targetm.sched.free_sched_context (save->be_saved_data)`** - Calls the target-specific function to free backend saved data

2. **`if (current_sched_info->restore_state)`** - Checks if the current scheduler info has a restore_state function
   - **`free (save->fe_saved_data)`** - Frees frontend saved data

3. **`for (i = 0; i <= max_insn_queue_index; i++)`** - Loops through instruction queue
   - **`free_INSN_LIST_list (&save->insn_queue[i])`** - Frees each instruction list in the queue

4. **`free (save->insn_queue)`** - Frees the instruction queue array itself

5. **`free (save->curr_state)`** - Frees the current state data

6. **`free (save->ready.vec)`** - Frees the ready list vector

7. **`free (save)`** - Finally frees the main save structure itself

This is a classic cleanup pattern where resources are freed in reverse order of allocation (from deepest nested structures to the main structure). The code handles both target-specific and generic scheduler data cleanup.
