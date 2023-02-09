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
#include <sys/io.h>

#include "../plugin.h"

struct vbios_modeline {
    unsigned long clock;

    unsigned short x1;
    unsigned short htotal;
    unsigned short x2;
    unsigned short hblank;
    unsigned short hsyncstart;
    unsigned short hsyncend;

    unsigned short y1;
    unsigned short vtotal;
    unsigned short y2;
    unsigned short vblank;
    unsigned short vsyncstart;
    unsigned short vsyncend;

    unsigned short timing_h;
    unsigned short timing_v;

    unsigned char unknown[6];
} __attribute__((packed));

struct vbios_resolution {
    unsigned char unknown[6];

    struct vbios_modeline modelines[];
} __attribute__((packed));

#define TIMING_60 0
#define TIMING_75 1
#define TIMING_85 2

static int freqs[] = { 60, 75, 85 };

static struct plugin_info *_get_plugin_info(void) {
static struct plugin_info pi = { "3", ".", 300 };
    return &pi;
}

static int _detect_vbios_type(struct vbios_mode *modes) {
	return check_vbios_type(modes, sizeof(struct vbios_modeline));
}

static unsigned char *_get_vbios_version(unsigned char *vbios_cfg) {
    return vbios_cfg+29;
}

static void _get_resolution(struct vbios_resolution *resolution, unsigned int *x, unsigned int *y) {
    *x = resolution->modelines[0].x1+1;
    *y = resolution->modelines[0].y1+1;
}

static void _set_resolution(struct vbios_resolution *resolution, unsigned int x, unsigned int y) {
struct vbios_modeline *modeline;
int i;

    for(i=0; i<3; i++) {
        modeline = &resolution->modelines[i];

        if(modeline->x1 == resolution->modelines[0].x1) {
            modeline->x1 = modeline->x2 = x-1;
            modeline->y1 = modeline->y2 = y-1;

            /* WARNING: I don't know at all what are the formulas
            to calculate these values !
            The following lines are only an example.
            */
            modeline->clock = freqs[i] * x;

            modeline->hsyncstart = x+10;
            modeline->hsyncend   = x+20;
            modeline->htotal     = x+30;

            modeline->vsyncstart = y+10;
            modeline->vsyncend   = y+20;
            modeline->vtotal     = y+30;

            modeline->timing_h   = 0x2ff;
            modeline->timing_v   = 0x4ff;
        }
    }
}

struct plugin plugin3 = {
	_get_plugin_info,
    _detect_vbios_type,
    _get_vbios_version,
    (get_resolution_type) _get_resolution,
    (set_resolution_type) _set_resolution
};

