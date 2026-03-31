/*
 * GCC plugin to trigger uncovered lines in plugin.cc (lines 458-470)
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and 
 * PLUGIN_REGISTER_GGC_ROOTS events with NULL callbacks.
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

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static bool dummy_pass_gate(void)
{
    /* Always return false so the pass doesn't actually run */
    return false;
}

static unsigned int dummy_pass_execute(void)
{
    /* This should never be called since gate returns false */
    return 0;
}

static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = dummy_pass_gate,
    .execute = dummy_pass_execute,
    .next = NULL,
    .static_pass_number = 0,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0
};

/* Structure for PLUGIN_PASS_MANAGER_SETUP registration */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Structure for PLUGIN_INFO registration */
static struct plugin_info plugin_info_struct = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP, "
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* Structure for PLUGIN_REGISTER_GGC_ROOTS registration */
static struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL-terminated array as required by GCC */
    { NULL, 0, 0, NULL, NULL }
};

/**
 * plugin_init - Main plugin initialization function
 * @plugin_info: Plugin information from GCC
 * @version: GCC version information
 *
 * Returns: 1 on success, 0 on failure
 *
 * This function registers callbacks for the three target events:
 * 1. PLUGIN_PASS_MANAGER_SETUP - registers a dummy pass
 * 2. PLUGIN_INFO - registers plugin metadata
 * 3. PLUGIN_REGISTER_GGC_ROOTS - registers dummy GGC roots
 *
 * All registrations use NULL callbacks as required by the uncovered code.
 */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    int ret;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: incompatible GCC version\n", plugin_name);
        return 0;
    }
    
    /* Set plugin name from arguments */
    plugin_name = plugin_info_args->base_name;
    
    /* 
     * Register for PLUGIN_PASS_MANAGER_SETUP event with NULL callback.
     * This triggers line 458-462 in plugin.cc.
     */
    ret = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* NULL callback as required by uncovered code */
        (void *)&pass_info
    );
    
    if (ret != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_PASS_MANAGER_SETUP\n", 
                plugin_name);
        return 0;
    }
    
    /* 
     * Register for PLUGIN_INFO event with NULL callback.
     * This triggers line 463-466 in plugin.cc.
     */
    ret = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* NULL callback as required by uncovered code */
        (void *)&plugin_info_struct
    );
    
    if (ret != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return 0;
    }
    
    /* 
     * Register for PLUGIN_REGISTER_GGC_ROOTS event with NULL callback.
     * This triggers line 467-469 in plugin.cc.
     */
    ret = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* NULL callback as required by uncovered code */
        (void *)dummy_ggc_roots
    );
    
    if (ret != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", 
                plugin_name);
        return 0;
    }
    
    /* Optional: Register for PLUGIN_FINISH to confirm execution */
    ret = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    fprintf(stderr, "%s: Successfully registered all target events\n", 
            plugin_name);
    
    return 1;  /* Success */
}
