/*
 * GCC plugin to trigger uncovered lines in plugin.cc
 * Lines 458-470: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "plugin.h"
#include "tree-pass.h"
#include "context.h"

/* Mandatory plugin metadata */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC root registration */
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

/* Dummy pass gate function - returns false so pass doesn't run */
static bool dummy_pass_gate(void)
{
    return false;
}

/* Dummy pass structure */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_pass_gate,
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

/* Pass info structure for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info structure for PLUGIN_INFO */
static struct plugin_info plugin_info_struct = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP, "
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    int result;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info->base_name;
    
    printf("Coverage Trigger Plugin: Initializing for plugin '%s'\n", plugin_name);
    
    /* 
     * Register for PLUGIN_PASS_MANAGER_SETUP event
     * This triggers line 458-462 in plugin.cc
     * callback is NULL as required by the assertion
     */
    result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* callback must be NULL for this event */
        (void *)&dummy_pass_info
    );
    
    if (result != 0) {
        fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP callback\n");
        return 1;
    }
    
    printf("Registered PLUGIN_PASS_MANAGER_SETUP event\n");
    
    /* 
     * Register for PLUGIN_INFO event
     * This triggers line 463-466 in plugin.cc
     * callback is NULL as required by the assertion
     */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* callback must be NULL for this event */
        (void *)&plugin_info_struct
    );
    
    if (result != 0) {
        fprintf(stderr, "Failed to register PLUGIN_INFO callback\n");
        return 1;
    }
    
    printf("Registered PLUGIN_INFO event\n");
    
    /* 
     * Register for PLUGIN_REGISTER_GGC_ROOTS event
     * This triggers line 467-470 in plugin.cc
     * callback is NULL as required by the assertion
     */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* callback must be NULL for this event */
        (void *)dummy_ggc_roots
    );
    
    if (result != 0) {
        fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS callback\n");
        return 1;
    }
    
    printf("Registered PLUGIN_REGISTER_GGC_ROOTS event\n");
    
    /* Optional: Register for finish event to confirm execution */
    result = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    printf("Coverage Trigger Plugin: All target events registered successfully\n");
    
    return 0;  /* Return 0 for success */
}
