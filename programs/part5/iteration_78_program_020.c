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

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static struct opt_pass dummy_pass = {
    .type = SIMPLE_IPA_PASS,
    .name = "dummy-coverage-pass",
    .gate = NULL,  /* Always returns false - pass won't execute */
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

/* Pass info structure for registration */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",  /* Insert after SSA pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info structure for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Plugin to trigger uncovered lines in plugin.cc (lines 458-470)"
};

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = &dummy_ggc_root,
        .nelt = 1,
        .stride = sizeof(dummy_ggc_root),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL-terminated array as required */
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
 * 1. PLUGIN_PASS_MANAGER_SETUP - with register_pass_info struct
 * 2. PLUGIN_INFO - with plugin_info struct  
 * 3. PLUGIN_REGISTER_GGC_ROOTS - with ggc_root_tab array
 *
 * All registrations use NULL callback as required by the uncovered code.
 */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "%s: incompatible GCC version\n", plugin_name);
        return 0;
    }

    /* Set global plugin name */
    plugin_name = plugin_info_args->base_name;

    /* 
     * Register for PLUGIN_PASS_MANAGER_SETUP event
     * This triggers line 459-461 in plugin.cc
     * callback is NULL as required by gcc_assert(!callback)
     */
    register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* NULL callback - event handled via user_data */
        &pass_info
    );

    /* 
     * Register for PLUGIN_INFO event  
     * This triggers line 462-464 in plugin.cc
     * callback is NULL as required by gcc_assert(!callback)
     */
    register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* NULL callback - event handled via user_data */
        &plugin_info_data
    );

    /* 
     * Register for PLUGIN_REGISTER_GGC_ROOTS event
     * This triggers line 465-467 in plugin.cc
     * callback is NULL as required by gcc_assert(!callback)
     */
    register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* NULL callback - event handled via user_data */
        dummy_ggc_roots
    );

    /* Optional: Register finish callback for debugging */
    register_callback(
        plugin_name,
        PLUGIN_FINISH,
        NULL,
        NULL
    );

    return 1;  /* Success */
}
