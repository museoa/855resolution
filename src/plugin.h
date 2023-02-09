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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include "vbios.h"

struct plugin_info {
	char *label;
	char *description;
	int priority;
};

typedef struct plugin_info *(*get_plugin_info_type)(void);
typedef int (*detect_vbios_type_type)(struct vbios_mode *modes);
typedef unsigned char *(*get_vbios_version_type)(unsigned char *);
typedef void (*get_resolution_type)(void *, unsigned int *x, unsigned int *y);
typedef void (*set_resolution_type)(void *, unsigned int x, unsigned int y);

struct plugin {
	get_plugin_info_type get_plugin_info;
    detect_vbios_type_type detect_vbios_type;
    get_vbios_version_type get_vbios_version;
    get_resolution_type get_resolution;
    set_resolution_type set_resolution;
};

extern void initialize_plugins(void);
extern int get_nb_plugins(void);
extern struct plugin *get_plugin(int n);
extern void display_plugins(void);
extern struct plugin *detect_vbios_type(struct vbios_mode *modes);
extern int check_vbios_type(struct vbios_mode *modes, int expected_vbios_modeline_size);

#endif

