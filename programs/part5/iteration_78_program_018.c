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

/* Mandatory plugin declarations */
int plugin_is_GPL_compatible = 1;
const char *plugin_name = "coverage_trigger_plugin";

/* Dummy variable for GGC root registration */
static int dummy_ggc_root = 0;

/* Minimal dummy pass structure */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = NULL,
    .execute = NULL,
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
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info structure for PLUGIN_INFO */
static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP, "
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_roots[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminating NULL entry */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    int result;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 0;
    }
    
    /* Set global plugin name */
    plugin_name = plugin_info->base_name;
    
    /* 
     * Register for PLUGIN_PASS_MANAGER_SETUP event
     * This triggers lines 458-460 in plugin.cc
     * callback = NULL as required by gcc_assert(!callback)
     */
    result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* NULL callback - crucial for coverage */
        (void *)&pass_info
    );
    
    if (result != 1) {
        /* Registration failed */
        return 0;
    }
    
    /* 
     * Register for PLUGIN_INFO event
     * This triggers lines 462-464 in plugin.cc
     * callback = NULL as required by gcc_assert(!callback)
     */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* NULL callback - crucial for coverage */
        (void *)&plugin_metadata
    );
    
    if (result != 1) {
        /* Registration failed */
        return 0;
    }
    
    /* 
     * Register for PLUGIN_REGISTER_GGC_ROOTS event
     * This triggers lines 466-468 in plugin.cc
     * callback = NULL as required by gcc_assert(!callback)
     */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* NULL callback - crucial for coverage */
        (void *)dummy_roots
    );
    
    if (result != 1) {
        /* Registration failed */
        return 0;
    }
    
    /* Optional: Register finish callback for debugging */
    result = register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );
    
    return 1; /* Success */
}
