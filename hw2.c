#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <string.h>

#include <libfdt.h>
#include <nfb/nfb.h>
#include <nfb/ndp.h>

#include <netcope/nccommon.h>
#include <netcope/rxqueue.h>
#include <netcope/txqueue.h>

#define ARGUMENTS "d:i:f:l:s:"
#define REG_ERR  1
#define NFB_ERR  2
#define FDT_ERR  3
#define COMP_ERR 4

int check_hex(char* optarg){
    int length = strlen (optarg);
    for (int i=0;i<length; i++){
        if (!isdigit(optarg[i]))
        {
            return 0x1ffffff;
        }
    }

    int num = atoi(optarg);
    if(num >=0 && num <256){
        return num;
    }
    
    return 0x1ffffff;;
}

int adjust_reg(int reg, char* opt, int offset){
    int num = check_hex(opt);
    if(num>0xffffff){
        return 0x1ffffff;
    }
    num=num<<offset;
    reg += num;
    return reg;
}

int getopt(int argc, char *const argv[],const char *optstring);

int main(int argc, char *argv[]){
    struct nfb_device *dev;
    char c;
    int ret = 0;
    struct list_range index_range;
    list_range_init(&index_range);

    extern char *optarg;
    extern int optopt;

    char *file = NFB_PATH_DEV(0);
    int reg = 0x000000;

    while ((c = getopt(argc, argv, ARGUMENTS)) != -1) {
	switch (c) {
	case 'd':
		file = optarg;
		break;
	case 'i':
		if (list_range_parse(&index_range, optarg) < 0)
			errx(EXIT_FAILURE, "Cannot parse interface number.");
		break;
        case 'f':
            reg = adjust_reg(reg, optarg, 16);
            break;
        case 'l':
            reg = adjust_reg(reg, optarg, 8);
            break;
        case 's':
            reg = adjust_reg(reg, optarg, 0);
            break;
		default:
			err(-EINVAL, "Unknown argument -%c", optopt);
		}
    }

	if(reg>0xffffff){
		ret = -REG_ERR;
		goto err_reg;
	}	

    	dev = nfb_open(file);
        if (dev == NULL) {
        	ret = -NFB_ERR;
		goto err_list_range;
	}

	const void* fdt = nfb_get_fdt(dev);
	if(fdt == NULL){
		ret = -FDT_ERR;
		goto err_nfb_open;
	}
	
	struct nfb_comp* comp = nfb_comp_open(dev, fdt_path_offset(fdt, "/firmware/mi_bus0/nic_application/core0"));
	if(comp == NULL){
		ret = -COMP_ERR;
		goto err_nfb_open;
	}

	uint32_t read_reg = nfb_comp_read32(comp,0);

	uint32_t write_reg = reg|read_reg;

	reg = reg + 0xff000000;
	write_reg = write_reg & reg;

	printf(" _________________________  \n");
	printf("|  read_reg  | write_reg  | \n");
	printf("|============|============| \n");			
    	printf("| 0x%08x | 0x%08x |\n", read_reg, write_reg);
	printf(" ------------------------- \n\n");
	
	nfb_comp_write32(comp, 0, write_reg);	
	
	nfb_comp_close(comp);
	
	err_nfb_open:
		nfb_close(dev);
	err_list_range:
		list_range_destroy(&index_range);
	err_reg:
		
	return ret;
}
