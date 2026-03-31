/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc code */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "plugin-version.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP - Custom Pass Definition
   ============================================ */

/* Simple dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder pass */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always enable this pass */
    return true;
}

static struct opt_pass dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "dummy-coverage-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
    .gate = dummy_pass_gate,
    .execute = dummy_pass_execute,
    .sub = NULL,
    .next = NULL,
    .static_pass_number = 0
};

/* Pass info structure for registration */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO - Plugin Information Structure
   ============================================ */

static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "Coverage plugin for testing GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS - GGC Root Table
   ============================================ */

/* Dummy structure for GGC root registration */
static struct dummy_ggc_struct {
    int dummy_field;
} dummy_ggc_data;

/* GGC root table with one dummy entry */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_ggc_data,
        .nelt = 1,
        .stride = sizeof(struct dummy_ggc_struct),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* NULL terminator */
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Coverage plugin %s initializing...\n", plugin_name);
    
    /* ============================================
       Register callback for PLUGIN_PASS_MANAGER_SETUP
       ============================================ */
    register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* No callback function needed for registration */
        &dummy_pass_info
    );
    
    /* ============================================
       Register callback for PLUGIN_INFO
       ============================================ */
    register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* No callback function needed for registration */
        &plugin_metadata
    );
    
    /* ============================================
       Register callback for PLUGIN_REGISTER_GGC_ROOTS
       ============================================ */
    register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* No callback function needed for registration */
        dummy_ggc_roots
    );
    
    printf("Coverage plugin %s registered all callbacks\n", plugin_name);
    
    return 0;  /* Success */
}
