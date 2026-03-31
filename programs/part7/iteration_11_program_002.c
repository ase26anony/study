fprintf(dump_file, "%11d %11d %5d %d --(T,%d,%d)--> %d\n",
    this_start, this_end, SCHED_TIME(move->def),
    INSN_UID(this_insn), this_latency, this_distance,
    INSN_UID(move->insn));
