/*
 * GCC Plugin to trigger uncovered lines in plugin.cc
 * Specifically targets:
 * - PLUGIN_PASS_MANAGER_SETUP
 * - PLUGIN_INFO
 * - PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "plugin-version.h"
#include "ggc.h"

/* Required for GCC plugin compatibility */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PART 1: PLUGIN_PASS_MANAGER_SETUP
   ============================================ */

/* Define a simple dummy pass for registration */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder */
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

/* Create the pass info structure for registration */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PART 2: PLUGIN_INFO
   ============================================ */

static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "GCC Coverage Plugin - Triggers uncovered plugin.cc lines\n"
            "This plugin registers dummy components to exercise plugin infrastructure."
};

/* ============================================
   PART 3: PLUGIN_REGISTER_GGC_ROOTS
   ============================================ */

/* Define a dummy GGC root table entry */
static GTY(()) tree dummy_ggc_tree = NULL_TREE;

static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_ggc_tree,
        .nelt = 1,
        .stride = sizeof(tree),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator - required by GCC */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   PLUGIN INITIALIZATION
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin version mismatch\n");
        return 1;
    }
    
    printf("Coverage Plugin: Initializing plugin '%s'\n", plugin_name);
    
    /* ============================================
       Register PLUGIN_PASS_MANAGER_SETUP event
       ============================================ */
    printf("Coverage Plugin: Registering PLUGIN_PASS_MANAGER_SETUP\n");
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP, 
                      NULL,  /* No callback function needed */
                      &dummy_pass_info);
    
    /* ============================================
       Register PLUGIN_INFO event
       ============================================ */
    printf("Coverage Plugin: Registering PLUGIN_INFO\n");
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback function needed */
                      &plugin_metadata);
    
    /* ============================================
       Register PLUGIN_REGISTER_GGC_ROOTS event
       ============================================ */
    printf("Coverage Plugin: Registering PLUGIN_REGISTER_GGC_ROOTS\n");
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback function needed */
                      dummy_ggc_roots);
    
    printf("Coverage Plugin: All events registered successfully\n");
    
    return 0; /* Success */
}
