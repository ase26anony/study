/*
 * GCC plugin designed to trigger specific uncovered lines in plugin.cc
 * Lines 458-470: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
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
static int dummy_ggc_root = 0;

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static struct opt_pass dummy_pass;

/* Gate function that returns false so the pass doesn't actually run */
static bool dummy_gate(void)
{
    return false;
}

/* Dummy pass definition - minimal implementation */
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
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP, "
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab ggc_root_table[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = sizeof(dummy_ggc_root),
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* Optional finish callback to verify plugin executed */
static void plugin_finish(void *gcc_data, void *user_data)
{
    fprintf(stderr, "%s: Successfully triggered target events\n", plugin_name);
}

/**
 * plugin_init - Main plugin initialization function
 * @plugin_info: Plugin information structure
 * @version: GCC version information
 *
 * Returns: 1 on success, 0 on failure
 *
 * This function registers callbacks for the three target events:
 * 1. PLUGIN_PASS_MANAGER_SETUP - with register_pass_info
 * 2. PLUGIN_INFO - with plugin_info
 * 3. PLUGIN_REGISTER_GGC_ROOTS - with ggc_root_tab
 *
 * All registrations use NULL callback as required by the uncovered code.
 */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    int ret;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: Incompatible GCC version\n", plugin_name);
        return 0;
    }
    
    /* Set plugin name from arguments */
    plugin_name = plugin_info_args->base_name;
    
    fprintf(stderr, "%s: Initializing to trigger uncovered lines\n", plugin_name);
    
    /* 
     * Register for PLUGIN_PASS_MANAGER_SETUP
     * This triggers line 459: register_pass((struct register_pass_info *) user_data)
     * callback is NULL as required by gcc_assert(!callback)
     */
    ret = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* NULL callback - crucial for coverage */
        (void *)&pass_info
    );
    
    if (ret != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_PASS_MANAGER_SETUP\n", plugin_name);
        return 0;
    }
    
    fprintf(stderr, "%s: Registered PLUGIN_PASS_MANAGER_SETUP\n", plugin_name);
    
    /* 
     * Register for PLUGIN_INFO
     * This triggers line 463: register_plugin_info(plugin_name, (struct plugin_info *) user_data)
     * callback is NULL as required by gcc_assert(!callback)
     */
    ret = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* NULL callback - crucial for coverage */
        (void *)&plugin_info_data
    );
    
    if (ret != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return 0;
    }
    
    fprintf(stderr, "%s: Registered PLUGIN_INFO\n", plugin_name);
    
    /* 
     * Register for PLUGIN_REGISTER_GGC_ROOTS
     * This triggers line 467: ggc_register_root_tab((const struct ggc_root_tab*) user_data)
     * callback is NULL as required by gcc_assert(!callback)
     */
    ret = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* NULL callback - crucial for coverage */
        (void *)ggc_root_table
    );
    
    if (ret != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", plugin_name);
        return 0;
    }
    
    fprintf(stderr, "%s: Registered PLUGIN_REGISTER_GGC_ROOTS\n", plugin_name);
    
    /* Optional: Register finish callback to verify execution */
    ret = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        plugin_finish,
        NULL
    );
    
    fprintf(stderr, "%s: Plugin initialization complete\n", plugin_name);
    
    return 1;  /* Success */
}
