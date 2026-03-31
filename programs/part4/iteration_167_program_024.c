is_cpp_driver = 0;                    // Flag indicating if this is a C++ compiler driver
at_file_supplied = 0;                 // Whether an @file (response file) was provided
print_help_list = 0;                  // Flag to print help list
print_version = 0;                    // Flag to print version information
verbose_only_flag = 0;                // Verbose output flag
print_subprocess_help = 0;            // Flag to print subprocess help
use_ld = NULL;                        // Linker to use (NULL = default)
report_times_to_file = NULL;          // File to report timing information to
target_system_root = DEFAULT_TARGET_SYSTEM_ROOT;  // Default system root path
target_system_root_changed = 0;       // Flag if system root was changed
target_sysroot_suffix = 0;            // System root suffix
target_sysroot_hdrs_suffix = 0;       // System root headers suffix
save_temps_flag = SAVE_TEMPS_NONE;    // Don't save temporary files
save_temps_overrides_dumpdir = false; // Flag for save-temps behavior
dumpdir_trailing_dash_added = false;  // Flag for dump directory formatting
