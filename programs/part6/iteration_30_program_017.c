This code appears to be from the GCC compiler's scheduler, specifically cleaning up saved scheduling state. Here's what each part does:

1. **`if (targetm.sched.free_sched_context)`** - Calls a target-specific function to free scheduler context data if the target machine has this hook defined.

2. **`if (current_sched_info->restore_state)`** - Frees front-end saved data if the current scheduling info structure indicates state restoration is needed.

3. **`for (i = 0; i <= max_insn_queue_index; i++)`** - Iterates through the instruction queue array and frees each instruction list.

4. **`free (save->insn_queue)`** - Frees the array that held the instruction queues.

5. **`free (save->curr_state)`** - Frees the current state structure.

6. **`free (save->ready.vec)`** - Frees the ready list vector (instructions ready for scheduling).

7. **`free (save)`** - Finally frees the main save structure itself.

This is cleanup code that runs when discarding saved scheduler state, likely during:
- Scheduling region transitions
- Error recovery
- Or when abandoning speculative scheduling attempts

The code follows a careful order: first target-specific cleanup, then front-end data, then individual data structures from contained objects outward, finally the main structure.
