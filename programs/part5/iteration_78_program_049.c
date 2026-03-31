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

/* Dummy variable for GGC root registration */
static int dummy_ggc_root = 0;

/* Dummy GGC root table - terminated with NULL entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = sizeof(dummy_ggc_root),
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* NULL terminator */
};

/* Plugin info structure for PLUGIN_INFO event */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS"
};

/* Dummy gate function that always returns false (pass won't execute) */
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

/* Pass info structure for PLUGIN_PASS_MANAGER_SETUP event */
static struct register_pass_info pass_info_data = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",  /* Insert after SSA pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;  /* Return error */
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info_args->base_name;
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event
     * This triggers the case at line 458-462 in plugin.cc
     * callback is NULL as required by the assertion
     */
    register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* callback must be NULL as per uncovered code */
        &pass_info_data
    );
    
    /* Register for PLUGIN_INFO event
     * This triggers the case at line 463-466 in plugin.cc
     * callback is NULL as required by the assertion
     */
    register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* callback must be NULL as per uncovered code */
        &plugin_info_data
    );
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event
     * This triggers the case at line 467-470 in plugin.cc
     * callback is NULL as required by the assertion
     */
    register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* callback must be NULL as per uncovered code */
        dummy_ggc_roots
    );
    
    /* Optional: Register for PLUGIN_FINISH to confirm execution */
    register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    return 0;  /* Success */
}
