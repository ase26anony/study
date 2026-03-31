/* test_plugin.c - GCC plugin to trigger uncovered lines in plugin.cc */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "plugin.h"
#include "pass_manager.h"
#include "ggc.h"

/* Mandatory plugin metadata */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "test_coverage_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_var = 0;

/* Dummy pass gate function (returns false so pass doesn't run) */
static bool dummy_gate(void)
{
    return false;
}

/* Dummy pass structure */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_gate,
    .execute = NULL,  /* No execution needed for coverage */
    .sub = NULL,
    .next = NULL,
    .static_pass_number = 0,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",  /* Insert after SSA pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Test plugin for coverage analysis\n"
            "This plugin triggers specific events in plugin.cc\n"
};

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab ggc_root_table[] = {
    {
        .base = &dummy_ggc_var,
        .nelt = sizeof(dummy_ggc_var),
        .stride = sizeof(dummy_ggc_var),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* Terminator */
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    int ret;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info_args->base_name;
    
    printf("Test coverage plugin initializing: %s\n", plugin_name);
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event
     * This triggers lines 458-462 in plugin.cc */
    ret = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        (void *)&pass_info
    );
    
    if (ret != 0) {
        fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP\n");
        return 1;
    }
    
    /* Register for PLUGIN_INFO event
     * This triggers lines 463-466 in plugin.cc */
    ret = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        (void *)&plugin_info_data
    );
    
    if (ret != 0) {
        fprintf(stderr, "Failed to register PLUGIN_INFO\n");
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event
     * This triggers lines 467-470 in plugin.cc */
    ret = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* callback must be NULL as per uncovered code assertion */
        (void *)ggc_root_table
    );
    
    if (ret != 0) {
        fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS\n");
        return 1;
    }
    
    /* Optional: Register for finish event to confirm execution */
    ret = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    printf("Test coverage plugin successfully registered all target events\n");
    
    return 0;  /* Return 0 for success */
}
