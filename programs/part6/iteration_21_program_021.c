This script creates a comprehensive test that:

1. **Sets help/version flags** (`print_help_list`, `print_version`, `verbose_only_flag`)
2. **Manipulates target system roots** (`target_system_root`, `target_system_root_changed`)
3. **Uses dump file options** (`dumpdir`, `dumpbase`, `dumpbase_ext`)
4. **Sets output base and save-temps** (`outbase`, `save_temps_flag`, `save_temps_overrides_dumpdir`)
5. **Uses time reporting and specs** (`report_times_to_file`, spec processing)
6. **Triggers warnings and errors** (`greatest_status`)
7. **Exercises all compilation phases** (preprocess, compile, assemble, link)
8. **Changes environment variables** between runs
9. **Uses a Makefile** for sequential build testing
10. **Ends with simple compilation** to verify cleanup worked

The script should be run with:
