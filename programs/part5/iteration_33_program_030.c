/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc code */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP - Custom Pass Definition
   ============================================ */

/* Simple dummy pass structure */
struct dummy_pass_data {
    struct opt_pass pass;
};

/* Dummy pass execution function */
static unsigned int
execute_dummy_pass (void)
{
    /* Do nothing, just return */
    return 0;
}

/* Create a dummy pass instance */
static struct dummy_pass_data dummy_pass_instance = {
    {
        .type = GIMPLE_PASS,
        .name = "dummy-coverage-pass",
        .optinfo_flags = OPTGROUP_NONE,
        .tv_id = TV_NONE,
        .properties_required = 0,
        .properties_provided = 0,
        .properties_destroyed = 0,
        .todo_flags_start = 0,
        .todo_flags_finish = 0,
        .execute = execute_dummy_pass,
        .gate = NULL,  /* Always enabled */
    }
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass_instance.pass,
    .reference_pass_name = "cfg",  /* Insert after CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO - Plugin Information Structure
   ============================================ */

static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Coverage plugin for testing GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP,\n"
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS - GGC Root Tables
   ============================================ */

/* Dummy structure for GGC roots */
static GTY(()) tree dummy_tree_node = NULL_TREE;
static GTY(()) int dummy_int_array[10];

/* GGC root table array (must be NULL-terminated) */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_tree_node,
        .nelt = sizeof(dummy_tree_node),
        .stride = sizeof(dummy_tree_node),
        .cb = NULL,
        .pchw = NULL
    },
    {
        .base = (void *)&dummy_int_array,
        .nelt = sizeof(dummy_int_array) / sizeof(dummy_int_array[0]),
        .stride = sizeof(dummy_int_array[0]),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* NULL terminator */
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin version mismatch\n");
        return 1;
    }
    
    printf("Coverage plugin '%s' initializing...\n", plugin_name);
    
    /* ============================================
       Register callback for PLUGIN_PASS_MANAGER_SETUP
       This triggers: case PLUGIN_PASS_MANAGER_SETUP
       ============================================ */
    register_callback(plugin_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* No callback needed, infrastructure handles it */
                      &dummy_pass_info);
    
    /* ============================================
       Register callback for PLUGIN_INFO
       This triggers: case PLUGIN_INFO
       ============================================ */
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback needed */
                      &plugin_info_data);
    
    /* ============================================
       Register callback for PLUGIN_REGISTER_GGC_ROOTS
       This triggers: case PLUGIN_REGISTER_GGC_ROOTS
       ============================================ */
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback needed */
                      dummy_ggc_roots);
    
    printf("Coverage plugin '%s' registered all target events\n", plugin_name);
    
    return 0;  /* Success */
}
