/*
 * GCC plugin to trigger uncovered lines in plugin.cc (lines 458-470)
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "plugin.h"
#include "context.h"
#include "pass_manager.h"

/* Mandatory plugin declarations */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC roots registration */
static int dummy_ggc_var = 0;

/* Dummy GGC root table - terminated with NULL entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = &dummy_ggc_var,
        .nelt = sizeof(dummy_ggc_var),
        .stride = sizeof(dummy_ggc_var),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* Terminator */
};

/* Dummy plugin info structure */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS"
};

/* Dummy gate function that always returns false (so pass doesn't run) */
static bool dummy_gate(void)
{
    return false;
}

/* Dummy pass structure */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_gate,
    .execute = NULL,  /* No execution needed */
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

/* Pass registration info */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",  /* Insert after SSA pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/**
 * plugin_init - Main plugin initialization function
 * @plugin_info: Plugin information structure
 * @version: GCC version information
 *
 * Returns: 1 on success, 0 on failure
 *
 * This function registers callbacks for the three target events:
 * 1. PLUGIN_PASS_MANAGER_SETUP - with register_pass_info structure
 * 2. PLUGIN_INFO - with plugin_info structure  
 * 3. PLUGIN_REGISTER_GGC_ROOTS - with ggc_root_tab array
 *
 * All registrations use NULL callback as required by the uncovered code.
 */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    int result;
    
    /* Set global plugin name */
    plugin_name = plugin_info_args->base_name;
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* NULL callback as required by uncovered code */
        (void *)&pass_info
    );
    
    if (result != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_PASS_MANAGER_SETUP\n", plugin_name);
        return 0;
    }
    
    /* Register for PLUGIN_INFO event */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* NULL callback as required by uncovered code */
        (void *)&plugin_info_data
    );
    
    if (result != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return 0;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* NULL callback as required by uncovered code */
        (void *)dummy_ggc_roots
    );
    
    if (result != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", plugin_name);
        return 0;
    }
    
    /* Optional: Register finish callback for debugging */
    result = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    fprintf(stderr, "%s: Successfully registered all target events\n", plugin_name);
    
    return 1;  /* Success */
}
