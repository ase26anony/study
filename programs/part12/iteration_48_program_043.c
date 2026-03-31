This code appears to be from the GCC compiler's scheduler module, specifically cleaning up saved scheduling state. Here's what each part does:

1. **`targetm.sched.free_sched_context`** - Calls a target-specific function to free backend-specific scheduling context data if it exists.

2. **`current_sched_info->restore_state`** - If the current scheduling info has a `restore_state` function, it frees the frontend-specific saved data.

3. **`free_INSN_LIST_list (&save->insn_queue[i])`** - Frees individual instruction lists in the instruction queue array for each priority level (0 to `max_insn_queue_index`).

4. **`free (save->insn_queue)`** - Frees the array that holds the instruction queue lists.

5. **`free (save->curr_state)`** - Frees the current scheduling state structure.

6. **`free (save->ready.vec)`** - Frees the vector used for the ready list of instructions.

7. **`free (save)`** - Finally frees the main saved state structure itself.

This is a classic pattern of nested cleanup where a complex data structure with multiple dynamically allocated components is properly deallocated in reverse order of allocation (from deepest to shallowest).
