/*
 * GCC plugin to trigger uncovered lines in plugin.cc
 * Specifically targets lines 458-470 covering:
 * - PLUGIN_PASS_MANAGER_SETUP
 * - PLUGIN_INFO  
 * - PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "intl.h"
#include "plugin.h"
#include "tree-pass.h"
#include "ggc.h"

/* Mandatory plugin metadata */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_root = 0;

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
    .static_pass_number = 0
};

/* Data structure for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Data structure for PLUGIN_INFO */
static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP, "
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* Data structure for PLUGIN_REGISTER_GGC_ROOTS */
static struct ggc_root_tab dummy_roots[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = sizeof(dummy_ggc_root),
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminating NULL entry as required */
    { NULL, 0, 0, NULL, NULL }
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
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP event
     * This triggers the uncovered code at line 458-462 in plugin.cc
     * Note: callback is NULL as required by the uncovered code assertion
     */
    result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* NULL callback as required */
        (void *)&pass_info
    );
    
    if (result != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_PASS_MANAGER_SETUP\n", 
                plugin_name);
        return 1;
    }
    
    /* Register for PLUGIN_INFO event
     * This triggers the uncovered code at line 463-466 in plugin.cc
     * Note: callback is NULL as required by the uncovered code assertion
     */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* NULL callback as required */
        (void *)&plugin_metadata
    );
    
    if (result != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return 1;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS event
     * This triggers the uncovered code at line 467-470 in plugin.cc
     * Note: callback is NULL as required by the uncovered code assertion
     */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* NULL callback as required */
        (void *)dummy_roots
    );
    
    if (result != 0) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", 
                plugin_name);
        return 1;
    }
    
    /* Optional: Register for PLUGIN_FINISH to confirm execution */
    result = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    fprintf(stderr, "%s: Successfully registered all target events\n", 
            plugin_name);
    
    return 0;  /* Return 0 for success */
}
