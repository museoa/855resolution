/* 855resolution by Alain Poirier
 *
 * This code is based on the techniques used in :
 *
 *   - 855patch.  Many thanks to Christian Zietz (czietz gmx net)
 *     for demonstrating how to shadow the VBIOS into system RAM
 *     and then modify it.
 *
 *   - 1280patch by Andrew Tipton (andrewtipton null li).
 *
 * This source code is into the public domain.
 */

#include <stdio.h>
#include <stdlib.h>

#include "plugin.h"

extern struct plugin PLUGINS;
static struct plugin *plugins[] = { REF_PLUGINS };
#define nb_plugins ((int) (sizeof(plugins) / sizeof(struct plugin *)))

static int compare_plugins(const void *p1, const void *p2) {
struct plugin *pi1, *pi2;
int priority1, priority2;

	pi1 = *(struct plugin **) p1;
	pi2 = *(struct plugin **) p2;
	
	priority1 = pi1->get_plugin_info()->priority;
	priority2 = pi2->get_plugin_info()->priority;
	
	if(priority1 == priority2) {
		return 0;
	}
	
	if(priority1 < priority2) {
		return -1;
	}
	
	return 1;
}

void initialize_plugins(void) {
	qsort(plugins, get_nb_plugins(), sizeof(struct plugin *), compare_plugins);
}

int get_nb_plugins(void) {
	return nb_plugins;
}

struct plugin *get_plugin(int n) {
	return plugins[n];
}

void display_plugins(void) {
int i;
struct plugin_info *info;

	for(i=0; i<get_nb_plugins(); i++) {
		info = get_plugin(i)->get_plugin_info();
		
		printf("Plugin #%d\n", i+1);
		printf("  Name        : %s\n", info->label);
		printf("  Description : %s\n", info->description);
		printf("  Priority    : %d\n", info->priority);
		putchar('\n');
	}
}

struct plugin *detect_vbios_type(struct vbios_mode *modes) {
int i;
struct plugin *pi;

	for(i=0; i<get_nb_plugins(); i++) {
    	pi = get_plugin(i);
        if(pi->detect_vbios_type(modes)) {
        	return pi;
        }
	}
	
	return NULL;
}

int check_vbios_type(struct vbios_mode *modes, int expected_vbios_modeline_size) {
short int r1, r2;
float f;

    r1 = r2 = 32000;
    while(modes->mode != 0xff) {
		if(modes->resolution <= r1) {
        	r1 = modes->resolution;
    	} else {
        	if(modes->resolution <= r2) {
            	r2 = modes->resolution;
        	}
    	}

        modes++;
    }

    f = ((float) (r2-r1-6)) / expected_vbios_modeline_size;

    return f == (int) f;
}
