/* coverage_plugin.c - GCC plugin to trigger uncovered code in plugin.cc */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "ggc.h"
#include "cgraph.h"
#include <stdio.h>

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PART 1: Data structures for the three events
   ============================================ */

/* 1. For PLUGIN_PASS_MANAGER_SETUP: A simple dummy pass */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder pass */
    printf("Dummy pass executed\n");
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
        "dummy-pass",           /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        dummy_pass_gate,        /* gate */
        dummy_pass_execute,     /* execute */
        NULL,                   /* sub */
        NULL,                   /* next */
        0,                      /* static_pass_number */
        TV_NONE,                /* tv_id */
        0,                      /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    }
};

/* Register pass info structure */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass.pass,           /* Pointer to the pass */
    .reference_pass_name = "cfg",       /* Insert after the CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER     /* Insert after reference pass */
};

/* 2. For PLUGIN_INFO: Plugin information structure */
static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "Coverage test plugin for GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP,\n"
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* 3. For PLUGIN_REGISTER_GGC_ROOTS: GGC root table */
/* Create a dummy variable that GCC's garbage collector can track */
static GTY(()) tree dummy_tree_node = NULL_TREE;

/* Define the ggc_root_tab array (must be NULL-terminated) */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_tree_node,
        .nelt = 1,
        .stride = sizeof(dummy_tree_node),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator required */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   PART 2: Plugin initialization function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    printf("Coverage plugin initializing: %s\n", plugin_name);
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        printf("Plugin version check failed!\n");
        return 1;
    }
    
    /* ============================================
       Register callbacks for the three target events
       ============================================ */
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
    register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* No callback needed - infrastructure handles it */
        &dummy_pass_info
    );
    
    /* 2. Register PLUGIN_INFO event */
    register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* No callback needed - infrastructure handles it */
        &plugin_metadata
    );
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
    register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* No callback needed - infrastructure handles it */
        dummy_ggc_roots
    );
    
    printf("All three target events registered successfully\n");
    
    /* Additional: Register a simple callback to verify plugin works */
    register_callback(
        plugin_name,
        PLUGIN_START_PARSE_FUNCTION,
        [](void *gcc_data, void *user_data) {
            printf("Plugin active - parsing function\n");
            return 0;
        },
        NULL
    );
    
    return 0; /* Success */
}
