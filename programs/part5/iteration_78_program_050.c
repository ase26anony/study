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
#include "pass_manager.h"
#include "ggc.h"

/* Mandatory plugin metadata */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_root = 0;

/* Minimal pass structure for PLUGIN_PASS_MANAGER_SETUP */
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
static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc (lines 458-470)"
};

/* Data structure for PLUGIN_REGISTER_GGC_ROOTS */
static struct ggc_root_tab dummy_root_tab[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator required by the API */
    { NULL, 0, 0, NULL, NULL }
};

/* Optional finish callback for debugging */
static void plugin_finish(void *gcc_data, void *user_data)
{
    fprintf(stderr, "%s: All target events triggered successfully\n", plugin_name);
}

/**
 * Plugin initialization function - registers callbacks for the target events
 */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    int ret;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: Incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info->base_name;
    
    fprintf(stderr, "%s: Initializing to trigger uncovered lines 458-470\n", plugin_name);
    
    /*
     * Register for PLUGIN_PASS_MANAGER_SETUP
     * This triggers line 459: register_pass((struct register_pass_info *) user_data)
     * Note: callback is NULL as required by the uncovered code
     */
    ret = register_callback(plugin_name, 
                           PLUGIN_PASS_MANAGER_SETUP,
                           NULL,  /* NULL callback as required */
                           &pass_info);
    
    if (ret) {
        fprintf(stderr, "%s: Failed to register PLUGIN_PASS_MANAGER_SETUP\n", plugin_name);
        return 1;
    }
    
    fprintf(stderr, "%s: Registered PLUGIN_PASS_MANAGER_SETUP\n", plugin_name);
    
    /*
     * Register for PLUGIN_INFO
     * This triggers line 463: register_plugin_info(plugin_name, (struct plugin_info *) user_data)
     * Note: callback is NULL as required by the uncovered code
     */
    ret = register_callback(plugin_name,
                           PLUGIN_INFO,
                           NULL,  /* NULL callback as required */
                           &plugin_metadata);
    
    if (ret) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return 1;
    }
    
    fprintf(stderr, "%s: Registered PLUGIN_INFO\n", plugin_name);
    
    /*
     * Register for PLUGIN_REGISTER_GGC_ROOTS
     * This triggers line 467: ggc_register_root_tab((const struct ggc_root_tab*) user_data)
     * Note: callback is NULL as required by the uncovered code
     */
    ret = register_callback(plugin_name,
                           PLUGIN_REGISTER_GGC_ROOTS,
                           NULL,  /* NULL callback as required */
                           dummy_root_tab);
    
    if (ret) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", plugin_name);
        return 1;
    }
    
    fprintf(stderr, "%s: Registered PLUGIN_REGISTER_GGC_ROOTS\n", plugin_name);
    
    /* Optional: Register finish callback for debugging */
    ret = register_callback(plugin_name,
                           PLUGIN_FINISH,
                           plugin_finish,
                           NULL);
    
    if (ret) {
        fprintf(stderr, "%s: Failed to register PLUGIN_FINISH\n", plugin_name);
    }
    
    fprintf(stderr, "%s: All target events registered successfully\n", plugin_name);
    
    return 0;  /* Return 0 for success */
}
