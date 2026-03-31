/*
 * GCC plugin designed to trigger specific uncovered lines in plugin.cc
 * Lines 458-470: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS
 * with NULL callback as required by the assertions.
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
        .base = &dummy_ggc_root,
        .nelt = sizeof(dummy_ggc_root),
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_arg,
                struct plugin_gcc_version *version)
{
    int result;
    
    /* Set global plugin name */
    plugin_name = plugin_info_arg->base_name;
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP with NULL callback */
    result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* NULL callback as required by gcc_assert(!callback) */
        &pass_info
    );
    
    if (result != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_PASS_MANAGER_SETUP\n", 
                plugin_name);
        return 0;
    }
    
    /* Register for PLUGIN_INFO with NULL callback */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* NULL callback as required by gcc_assert(!callback) */
        &plugin_info_struct
    );
    
    if (result != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_INFO\n", plugin_name);
        return 0;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS with NULL callback */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* NULL callback as required by gcc_assert(!callback) */
        dummy_roots
    );
    
    if (result != 1) {
        fprintf(stderr, "%s: Failed to register PLUGIN_REGISTER_GGC_ROOTS\n", 
                plugin_name);
        return 0;
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
    
    return 1; /* Success */
}
