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

 /* VBIOS as found on the Dell 510m */

#include <stdio.h>
#include <sys/io.h>

#include "../plugin.h"

struct vbios_resolution {
    unsigned char unknow1[2];
    unsigned char x1;
    unsigned char unknow2;
    unsigned char x2;
    unsigned char y1;
    unsigned char unknow3;
    unsigned char y2;
} __attribute__((packed));

static struct plugin_info *_get_plugin_info(void) {
static struct plugin_info pi = { "2", ".", 200 };
    return &pi;
}

static int _detect_vbios_type(struct vbios_mode *modes) {
	return check_vbios_type(modes, sizeof(struct vbios_resolution));
}

static unsigned char *_get_vbios_version(unsigned char *vbios_cfg) {
    return vbios_cfg+31;
}

static void _get_resolution(struct vbios_resolution *resolution, unsigned int *x, unsigned int *y) {
    *x = ((((unsigned int) resolution->x2) & 0xf0) << 4) | resolution->x1;
    *y = ((((unsigned int) resolution->y2) & 0xf0) << 4) | resolution->y1;
}

static void _set_resolution(struct vbios_resolution *resolution, unsigned int x, unsigned int y) {
    resolution->x2 = (resolution->x2 & 0x0f) | ((x >> 4) & 0xf0);
    resolution->x1 = (x & 0xff);

    resolution->y2 = (resolution->y2 & 0x0f) | ((y >> 4) & 0xf0);
    resolution->y1 = (y & 0xff);
}

struct plugin plugin2 = {
	_get_plugin_info,
    _detect_vbios_type,
    _get_vbios_version,
    (get_resolution_type) _get_resolution,
    (set_resolution_type) _set_resolution
};
