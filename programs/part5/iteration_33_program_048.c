/* plugin_coverage.c - GCC plugin to trigger uncovered code in plugin.cc */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PART 1: Data structures for the three events
   ============================================ */

/* 1. For PLUGIN_PASS_MANAGER_SETUP: A simple dummy pass */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing, just return */
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
    .reference_pass_name = "cfg",       /* Insert after the 'cfg' pass */
    .ref_pass_instance_number = 1,      /* First instance */
    .pos_op = PASS_POS_INSERT_AFTER     /* Insert after reference pass */
};

/* 2. For PLUGIN_INFO: Plugin information structure */
static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage analysis\n"
            "This plugin triggers uncovered code in plugin.cc"
};

/* 3. For PLUGIN_REGISTER_GGC_ROOTS: GGC root table */
/* Create a dummy variable that GCC's garbage collector can track */
static GTY(()) tree dummy_ggc_tree = NULL_TREE;

/* Define the root table with our dummy variable */
static const struct ggc_root_tab my_ggc_roots[] = {
    {
        .base = (void *)&dummy_ggc_tree,  /* Address of our variable */
        .nelt = 1,                        /* One element */
        .stride = sizeof(dummy_ggc_tree), /* Size of each element */
        .cb = NULL,                       /* No callback */
        .pchw = NULL                      /* No PCH handling */
    },
    /* Required NULL terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   PART 2: Plugin initialization function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: Incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s initializing...\n", plugin_name);
    
    /* ============================================
       Register callbacks for the three target events
       ============================================ */
    
    /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* No callback function needed for registration */
                      &dummy_pass_info);
    
    /* 2. Register PLUGIN_INFO event */
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback function needed */
                      &my_plugin_info);
    
    /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback function needed */
                      my_ggc_roots);
    
    /* Additional callback to demonstrate plugin is working */
    register_callback(plugin_name,
                      PLUGIN_FINISH,
                      NULL,
                      NULL);
    
    printf("Plugin %s successfully registered all events\n", plugin_name);
    
    return 0; /* Success */
}
