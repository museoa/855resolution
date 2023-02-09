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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vbios.h"
#include "plugin.h"

extern struct plugin PLUGINS;
static struct plugin *plugins[] = { REF_PLUGINS };
#define nb_plugins ((int) (sizeof(plugins) / sizeof(struct plugin *)))

static struct vbios_mode *find_modes(int *bios_type) {
int i;
struct vbios_mode *modes;
unsigned char *p = bios;

    while(p < (bios+VBIOS_SIZE-3*sizeof(struct vbios_mode))) {
        modes = (struct vbios_mode *) p;

        if(((modes[0].mode & 0xf0) == 0x30) && ((modes[1].mode & 0xf0) == 0x30) && ((modes[2].mode & 0xf0) == 0x30) && ((modes[3].mode & 0xf0) == 0x30)) {
            if(*bios_type == -1) {
                for(i=0; i<nb_plugins; i++) {
                    if(plugins[i]->detect_vbios_type(modes)) {
                        *bios_type = i;
                        break;
                    }
                }
            }

            return modes;
        }

        p++;
    }

    return NULL;
}

static struct vbios_resolution *find_resolution(struct vbios_mode *modes, int mode) {
    while(modes->mode != 0xff) {
        if(modes->mode == mode) {
            return VBIOS_POINTER(modes->resolution);
        }

        modes++;
    }

    return NULL;
}

static void list_modes(int vbios_type, struct vbios_mode *modes) {
unsigned int x, y;

    while(modes->mode != 0xff) {
        plugins[vbios_type]->get_resolution(VBIOS_POINTER(modes->resolution), &x, &y);

        if((x != 0) && (y != 0)) {
            printf("Mode %02x : %dx%d, %d bits/pixel\n", modes->mode, x, y, modes->bits_per_pixel);
        }

        modes++;
    }
}

static int parse_args(int argc, char *argv[], int *list, int *bios_type, int *mode, int *x, int *y) {
int index = 1;

    *list = *mode = *x = *y = 0;
    *bios_type = -1;

    if((argc > index) && !strcmp(argv[index], "-l")) {
        *list = 1;
        index++;

        if(argc<=index) {
            return 0;
        }
    }

    if((argc > index) && !strcmp(argv[index], "-f")) {
        index++;
    *bios_type = atoi(argv[index])-1;
    index++;
        if((*bios_type)<0 || (*bios_type >= nb_plugins)) {
            fprintf(stderr, "Unknown forced VBIOS type (must be <= %d)\n", nb_plugins);
        return -1;
    }

    if(argc<=index) {
        return 0;
    }
    }

    if(argc-index != 3) {
        return -1;
    }

    *mode = (int) strtol(argv[index], NULL, 16);
    *x = atoi(argv[index+1]);
    *y = atoi(argv[index+2]);

    return 0;
}

static void usage(char *name) {
    printf("Usage: %s [-l] [-f bios_type] [mode X Y]\n", name);
    printf("  Set the resolution to XxY for mode\n");
    printf("  Options:\n");
    printf("    -l display the modes found into the vbios\n");
    printf("    -f skip the VBIOS detection by forcing a VBIOS type\n");
}

int main (int argc, char *argv[]) {
unsigned char *vbios_cfg;
int vbios_type;
struct vbios_mode *modes;
void *resolution;
int list, mode, x, y;

    printf("855resolution version %s, by Alain Poirier\n\n", VERSION);

    if(parse_args(argc, argv, &list, &vbios_type, &mode, &x, &y) == -1)
    {
        usage(argv[0]);
        return 2;
    }

    if(iopl(3) < 0) {
        perror("Unable to obtain the proper IO permissions");
        return 2;
    }

    display_chipset();

    open_bios();

    modes = find_modes(&vbios_type);
    if(vbios_type == -1)
    {
        fprintf(stderr, "Unknow VBIOS structure\n");
        close_bios();
        return 2;
    }

    printf("VBIOS type: %d\n", vbios_type+1);

    vbios_cfg = get_vbios_cfg();
    if(vbios_cfg == NULL) {
        fprintf(stderr, "Couldn't find the configuration area in the VBIOS!\n");
        close_bios();
        return 2;
    }

    printf("VBIOS Version: %.4s\n", plugins[vbios_type]->get_vbios_version(vbios_cfg));

    if(modes == NULL)
    {
        fprintf(stderr, "Couldn't find the modes table in the VBIOS\n");
        close_bios();
        return 2;
    }

    putchar('\n');

    if(list) {
        list_modes(vbios_type, modes);
    }

    if(mode!=0 && x!=0 && y!=0)
    {
        resolution = find_resolution(modes, mode);
        if(resolution == NULL)
        {
            fprintf(stderr, "Couldn't find the mode %02x into the modes table\n", mode);
            close_bios();
            return 2;
        }

        plugins[vbios_type]->set_resolution(resolution, x, y);

        printf("** Patch mode %02x to resolution %dx%d complete\n", mode, x, y);

        if(list) {
            list_modes(vbios_type, modes);
        }
    }

    close_bios();

    return 0;
}
