This code appears to be from the GCC compiler's scheduler, specifically cleaning up saved scheduling state. Here's what each part does:

1. **`targetm.sched.free_sched_context(save->be_saved_data)`**
   - Calls a target-specific function to free backend-specific scheduling context data
   - Only executed if the target machine has this function defined

2. **`free(save->fe_saved_data)`**
   - Frees frontend-specific saved scheduling state
   - Only executed if the current scheduling info has a `restore_state` function

3. **`free_INSN_LIST_list(&save->insn_queue[i])`**
   - Frees individual instruction lists in the instruction queue
   - Loops through all queue indices (0 to `max_insn_queue_index`)

4. **`free(save->insn_queue)`**
   - Frees the array that holds the instruction queue lists

5. **`free(save->curr_state)`**
   - Frees the current scheduling state

6. **`free(save->ready.vec)`**
   - Frees the vector used for the ready list of instructions

7. **`free(save)`**
   - Finally frees the main save structure itself

This is a classic pattern of nested cleanup where:
- Individual elements are freed first
- Then the containers/arrays that hold them
- Finally the main structure

The code handles both target-specific (backend) and common (frontend) scheduling data cleanup.
