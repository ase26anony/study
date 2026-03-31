/*
 * GCC plugin designed to trigger specific uncovered lines in plugin.cc
 * Lines 458-470: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS
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
static int dummy_ggc_root = 42;

/* Dummy GGC root table - terminated with NULL entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
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

/* Dummy pass structure */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
    .gate = NULL,  /* Gate always returns false, so pass won't execute */
    .execute = NULL,  /* No execution function */
    .sub = NULL,
    .next = NULL,
    .static_pass_number = 0
};

/* Dummy pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "This plugin triggers uncovered lines in plugin.cc for coverage testing."
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;  /* Return error - version mismatch */
    }

    /* Set global plugin name */
    plugin_name = plugin_info->base_name;

    /*
     * Register for PLUGIN_PASS_MANAGER_SETUP
     * This triggers lines 458-460 in plugin.cc
     * callback is NULL as required by the assertion
     */
    register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* callback must be NULL for this event */
        (void *)&dummy_pass_info
    );

    /*
     * Register for PLUGIN_INFO
     * This triggers lines 461-463 in plugin.cc
     * callback is NULL as required by the assertion
     */
    register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* callback must be NULL for this event */
        (void *)&plugin_info_data
    );

    /*
     * Register for PLUGIN_REGISTER_GGC_ROOTS
     * This triggers lines 464-466 in plugin.cc
     * callback is NULL as required by the assertion
     */
    register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* callback must be NULL for this event */
        (void *)dummy_ggc_roots
    );

    /* Optional: Register for finish event to confirm plugin executed */
    register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );

    return 0;  /* Success */
}
