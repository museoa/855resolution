/* 855resolution by Alain Poirier
 *
 * Currently only tested on a Dell 510m with BIOS A04
 * *VERY* likely that this won't work yet on any
 * other versions or chipsets!!!
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

typedef int (*detect_vbios_type_type)(struct vbios_mode *modes);
typedef unsigned char *(*get_vbios_version_type)(unsigned char *);
typedef void (*get_resolution_type)(void *, unsigned int *x, unsigned int *y);
typedef void (*set_resolution_type)(void *, unsigned int x, unsigned int y);

struct plugin {
    detect_vbios_type_type detect_vbios_type;
    get_vbios_version_type get_vbios_version;
    get_resolution_type get_resolution;
    set_resolution_type set_resolution;
};

#endif

