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

/* Mandatory plugin declarations */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC roots registration */
static int dummy_ggc_var = 0;

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static bool dummy_gate(void)
{
    /* Always return false so the pass doesn't actually run */
    return false;
}

static unsigned int dummy_execute(void)
{
    /* This should never be called since gate returns false */
    return 0;
}

static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_gate,
    .execute = dummy_execute,
    .next = NULL,
    .static_pass_number = 0,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0
};

/* Data structure for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Data structure for PLUGIN_INFO */
static struct plugin_info plugin_info_struct = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP, "
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* Data structure for PLUGIN_REGISTER_GGC_ROOTS */
static struct ggc_root_tab dummy_roots[] = {
    {
        .base = &dummy_ggc_var,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_var),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator required */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_arg,
                struct plugin_gcc_version *version)
{
    int result;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1; /* Return non-zero to indicate failure */
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info_arg->base_name;
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event
     * This triggers lines 458-460 in plugin.cc
     * callback is NULL as required by the uncovered code
     */
    result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* callback must be NULL for this event */
        &pass_info
    );
    
    if (result != 0) {
        /* Registration failed */
        return 1;
    }
    
    /* Register for PLUGIN_INFO event
     * This triggers lines 461-463 in plugin.cc
     * callback is NULL as required by the uncovered code
     */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* callback must be NULL for this event */
        &plugin_info_struct
    );
    
    if (result != 0) {
        /* Registration failed */
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event
     * This triggers lines 464-466 in plugin.cc
     * callback is NULL as required by the uncovered code
     */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* callback must be NULL for this event */
        dummy_roots
    );
    
    if (result != 0) {
        /* Registration failed */
        return 1;
    }
    
    /* Optional: Register for PLUGIN_FINISH to confirm execution */
    result = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,  /* No callback needed for finish */
        NULL
    );
    
    /* Return 0 to indicate successful initialization */
    return 0;
}
