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
#include "tree-pass.h"  /* For pass definitions */
#include "ggc.h"        /* For GGC root registration */

/* Mandatory plugin metadata */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_root = 42;

/* Dummy GGC root table - terminated with NULL entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = sizeof(dummy_ggc_root),
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL }  /* Terminator */
};

/* Dummy gate function for the pass (returns false so pass doesn't run) */
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

/* Plugin info structure */
static struct plugin_info plugin_info_struct = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Registers events with NULL callbacks for coverage testing."
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
    
    /* 
     * Register for PLUGIN_PASS_MANAGER_SETUP with NULL callback.
     * This triggers line 458-460 in plugin.cc.
     */
    register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* NULL callback as required by uncovered code */
        &pass_info
    );
    
    /* 
     * Register for PLUGIN_INFO with NULL callback.
     * This triggers line 462-464 in plugin.cc.
     */
    register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* NULL callback as required by uncovered code */
        &plugin_info_struct
    );
    
    /* 
     * Register for PLUGIN_REGISTER_GGC_ROOTS with NULL callback.
     * This triggers line 466-468 in plugin.cc.
     */
    register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* NULL callback as required by uncovered code */
        (void *)dummy_ggc_roots
    );
    
    /* Optional: Register finish callback for debugging */
    register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    return 0;  /* Success */
}
