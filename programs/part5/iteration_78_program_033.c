/*
 * GCC plugin to trigger uncovered lines in plugin.cc (lines 458-470)
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, 
 * and PLUGIN_REGISTER_GGC_ROOTS events with NULL callbacks.
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

/* Mandatory plugin metadata */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_root = 0;

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static struct opt_pass dummy_pass;

/* Gate function that returns false so the pass doesn't actually run */
static bool dummy_gate(void)
{
    return false;
}

/* Execute function (not called since gate returns false) */
static unsigned int dummy_execute(void)
{
    return 0;
}

/* The actual pass definition */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_gate,
    .execute = dummy_execute,
    .next = NULL,
    .static_pass_number = 0
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
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Registers events: PASS_MANAGER_SETUP, INFO, REGISTER_GGC_ROOTS"
};

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab ggc_root_table[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator */
    { NULL, 0, 0, NULL, NULL }
};

/**
 * plugin_init - Main plugin initialization function
 * @plugin_info: Plugin information structure
 * @version: GCC version information
 *
 * Returns: 1 on success, 0 on failure
 *
 * This function registers callbacks for the three target events
 * with NULL callbacks as required by the uncovered code.
 */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: incompatible GCC version\n", plugin_name);
        return 0;
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info->base_name;
    
    printf("%s: Initializing coverage trigger plugin\n", plugin_name);
    
    /* 
     * Register for PLUGIN_PASS_MANAGER_SETUP with NULL callback
     * This triggers lines 458-460 in plugin.cc
     */
    if (!register_callback(plugin_name, 
                          PLUGIN_PASS_MANAGER_SETUP,
                          NULL,  /* NULL callback as required */
                          &pass_info)) {
        fprintf(stderr, "%s: Failed to register PASS_MANAGER_SETUP\n", plugin_name);
        return 0;
    }
    printf("%s: Registered PLUGIN_PASS_MANAGER_SETUP\n", plugin_name);
    
    /* 
     * Register for PLUGIN_INFO with NULL callback
     * This triggers lines 461-464 in plugin.cc
     */
    if (!register_callback(plugin_name,
                          PLUGIN_INFO,
                          NULL,  /* NULL callback as required */
                          &plugin_info_data)) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return 0;
    }
    printf("%s: Registered PLUGIN_INFO\n", plugin_name);
    
    /* 
     * Register for PLUGIN_REGISTER_GGC_ROOTS with NULL callback
     * This triggers lines 465-468 in plugin.cc
     */
    if (!register_callback(plugin_name,
                          PLUGIN_REGISTER_GGC_ROOTS,
                          NULL,  /* NULL callback as required */
                          ggc_root_table)) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", plugin_name);
        return 0;
    }
    printf("%s: Registered PLUGIN_REGISTER_GGC_ROOTS\n", plugin_name);
    
    /* Optional: Register finish callback for debugging */
    register_callback(plugin_name, PLUGIN_FINISH, NULL, NULL);
    
    printf("%s: Successfully registered all target events\n", plugin_name);
    return 1;  /* Success */
}
