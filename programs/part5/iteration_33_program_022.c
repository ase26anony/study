/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc code */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Required plugin metadata */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Simple dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder pass */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always run this pass */
    return true;
}

static struct gimple_opt_pass dummy_pass = {
    {
        GIMPLE_PASS,
        "dummy-coverage-pass",      /* name */
        OPTGROUP_NONE,              /* optinfo_flags */
        dummy_pass_gate,            /* gate */
        dummy_pass_execute,         /* execute */
        NULL,                       /* sub */
        NULL,                       /* next */
        0,                          /* static_pass_number */
        TV_NONE,                    /* tv_id */
        PROP_gimple_any,            /* properties_required */
        0,                          /* properties_provided */
        0,                          /* properties_destroyed */
        0,                          /* todo_flags_start */
        0                           /* todo_flags_finish */
    }
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass.pass,       /* Reference to the pass */
    .reference_pass_name = "cfg",   /* Insert after the 'cfg' pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "Coverage test plugin for GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP,\n"
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy GGC root table entry */
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
   Plugin Initialization Function
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
    
    printf("Coverage plugin '%s' initializing...\n", plugin_name);
    
    /* ============================================
       Register callback for PLUGIN_PASS_MANAGER_SETUP
       ============================================ */
    int result = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* No callback function needed - infrastructure handles it */
        &dummy_pass_info
    );
    
    if (result) {
        fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP\n");
        return 1;
    }
    printf("  Registered PLUGIN_PASS_MANAGER_SETUP\n");
    
    /* ============================================
       Register callback for PLUGIN_INFO
       ============================================ */
    result = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* No callback function needed */
        &plugin_metadata
    );
    
    if (result) {
        fprintf(stderr, "Failed to register PLUGIN_INFO\n");
        return 1;
    }
    printf("  Registered PLUGIN_INFO\n");
    
    /* ============================================
       Register callback for PLUGIN_REGISTER_GGC_ROOTS
       ============================================ */
    result = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* No callback function needed */
        dummy_ggc_roots
    );
    
    if (result) {
        fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS\n");
        return 1;
    }
    printf("  Registered PLUGIN_REGISTER_GGC_ROOTS\n");
    
    printf("Coverage plugin initialization complete\n");
    return 0;
}
