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

int check_hex(char* optarg){
    int length = strlen (optarg);
    for (int i=0;i<length; i++){
        if (!isdigit(optarg[i]))
        {
            return -1;
        }
    }

    int num = atoi(optarg);
    if(num >=0 && num <256){
        return num;
    }
    
    return -1;
}

int adjust_reg(int reg, char* opt, int offset){
    int num = check_hex(opt);
    if(num<0){
        fprintf( stderr, "Invalid number: %s\n", opt);
        exit(-1);
    }
    for(int i=0;i<offset;i++){
        num *= 16;

    }
    reg += num;
    printf("0x%06x\n", reg);
    return reg;
}

int getopt(int argc, char *const argv[],const char *optstring);

int main(int argc, char *argv[]){
    struct nfb_device *dev;
    char c;
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
            reg = adjust_reg(reg, optarg, 4);
            break;
        case 'l':
            reg = adjust_reg(reg, optarg, 2);
            break;
        case 's':
            reg = adjust_reg(reg, optarg, 0);
            break;
		default:
			err(-EINVAL, "Unknown argument -%c", optopt);
		}
	}

    dev = nfb_open(file);
        if (dev == NULL) {
            err(errno, "Can't open NFB device");
        }



    nfb_close(dev);
}