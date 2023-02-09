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
#include <string.h>
#include <unistd.h>
#include <sys/io.h>

#include "vbios.h"
#include "plugin.h"

static struct vbios_mode *find_modes(struct plugin **plugin) {
struct vbios_mode *modes;
unsigned char *p = bios;

    while(p < (bios+VBIOS_SIZE-3*sizeof(struct vbios_mode))) {
        modes = (struct vbios_mode *) p;

        if(((modes[0].mode & 0xf0) == 0x30) && ((modes[1].mode & 0xf0) == 0x30) && ((modes[2].mode & 0xf0) == 0x30) && ((modes[3].mode & 0xf0) == 0x30)) {
            if(*plugin == NULL) {
            	*plugin = detect_vbios_type(modes);
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

static void list_modes(struct plugin *plugin, struct vbios_mode *modes) {
unsigned int x, y;

    while(modes->mode != 0xff) {
        plugin->get_resolution(VBIOS_POINTER(modes->resolution), &x, &y);

        if((x != 0) && (y != 0)) {
            printf("Mode %02x : %dx%d, %d bits/pixel\n", modes->mode, x, y, modes->bits_per_pixel);
        }

        modes++;
    }
}

static int parse_args(
						int argc, char *argv[],
						int *list, int *plugins, struct plugin **plugin,
						int *mode, int *x, int *y
					 ) {
int plugin_type;
int index = 1;

    *list = *plugins = *mode = *x = *y = 0;
    *plugin = NULL;

    while((argc > index) && (strlen(argv[index]) == 2) && (argv[index][0] == '-')) {
    	index++;
    	
    	switch(argv[index-1][1]) {
    		case 'l':
    			*list = 1;
    		break;
    		
    		case 'p':
    			*plugins = 1;
    		break;
    		
    		case 'f':
    			if(argc <= index)
    			{
    				return -1;
    			}
    			
				plugin_type = atoi(argv[index])-1;
		        if((plugin_type<0) || (plugin_type >= get_nb_plugins())) {
		            fprintf(stderr, "Unknown forced VBIOS type (must be <= %d)\n", get_nb_plugins());
		        	return -1;
		    	}
		    	
		    	*plugin = get_plugin(plugin_type);
		    	index++;
		    break;
		    
    		default:
    			return -1;
    	}
    }

	if((argc-index) == 0) {
		return 0;
	}
	
    if((argc-index) != 3) {
        return -1;
    }

    *mode = (int) strtol(argv[index], NULL, 16);
    *x = atoi(argv[index+1]);
    *y = atoi(argv[index+2]);

    return 0;
}

static void usage(char *name) {
    printf("Usage: %s [-p] [-l] [-f bios_type] [mode X Y]\n", name);
    printf("  Set the resolution to XxY for mode\n");
    printf("  Options:\n");
    printf("    -l display the modes found into the vbios\n");
    printf("    -f skip the VBIOS detection by forcing a VBIOS type\n");
    printf("    -p display the registered plugins\n");
}

int main (int argc, char *argv[]) {
unsigned char *vbios_cfg;
struct plugin *plugin = NULL;
struct vbios_mode *modes;
void *resolution;
int list, plugins, mode, x, y;

	initialize_plugins();
		
    printf("855resolution version %s, by Alain Poirier\n\n", VERSION);

    if(parse_args(argc, argv, &list, &plugins, &plugin, &mode, &x, &y) == -1)
    {
        usage(argv[0]);
        return 2;
    }

	if(plugins)
	{
		display_plugins();
	}
	
    if(iopl(3) < 0) {
        perror("Unable to obtain the proper IO permissions");
        return 2;
    }

    display_chipset();

    open_bios();

    modes = find_modes(&plugin);
    if(plugin == NULL)
    {
        fprintf(stderr, "Unknow VBIOS structure\n");
        close_bios();
        return 2;
    }

    printf("VBIOS type: %s\n", plugin->get_plugin_info()->label);

    vbios_cfg = get_vbios_cfg();
    if(vbios_cfg == NULL) {
        fprintf(stderr, "Couldn't find the configuration area in the VBIOS!\n");
        close_bios();
        return 2;
    }

    printf("VBIOS Version: %.4s\n", plugin->get_vbios_version(vbios_cfg));

    if(modes == NULL)
    {
        fprintf(stderr, "Couldn't find the modes table in the VBIOS\n");
        close_bios();
        return 2;
    }

    putchar('\n');

    if(list) {
        list_modes(plugin, modes);
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

		unlock_bios();
    	plugin->set_resolution(resolution, x, y);
		relock_bios();

		printf("** Patch mode %02x to resolution %dx%d complete\n", mode, x, y);

		if(list) {
			list_modes(plugin, modes);
    	}
    }

    close_bios();

    return 0;
}
