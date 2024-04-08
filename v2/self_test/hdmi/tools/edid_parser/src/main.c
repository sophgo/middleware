
#include "edid_type.h"
#include "edid_parser.h"


void print_usage(char * program) {
    printf("Usage: %s [OPTION] \n", program);
    printf("\t-s,  --edid             EDID file.\n");
    printf("\t-h,  --help             this help.\n");

}

int open_edid(char * edid_file, u8 ** edid)
{
	FILE * fp = NULL;
	int size = 0;

	printf("Open File: %s\n", edid_file);
	fp = fopen(edid_file,"rb"); // read mode

	if( fp == NULL ) {
		perror("Error while opening the file.\n");
		exit(EXIT_FAILURE);
	}

	fseek(fp, 0, SEEK_END);

	size = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	*edid = (u8 *)malloc(size+1);
	if (size != fread(*edid, sizeof(char), size, fp))
	{
		free(*edid);
		return -2; // -2 means file reading fail
	}

	(*edid)[size] = 0;

	fclose(fp);
	return size;
}


int main(int argc, char **argv)
{
        int option;
        int option_index = 0;

	char * edid_file = NULL;
	int    edid_size = 0;
	u8 *   edid = NULL;
	edidCeaExt_t edidExt;
	hdmi_tx_dev_t dev = 1;

        static struct option long_options[] = {
                        {"help" , no_argument, 0, 'h'},
                        {"edid" , required_argument, 0, 'e'},
                        {NULL   , 0, NULL, 0}
        };

	
        if ((option = getopt_long(argc, argv, "he:",
                        long_options, &option_index)) != -1) {

                switch (option) {
                case  0 :
                        if (long_options[option_index].flag != 0)
                                break; // set flag on only
                        printf("option %s\n", long_options[option_index].name); break;
                case 'e':
                	if (optarg == NULL) {
                		printf ("File name parameters missing\n");
                                print_usage(argv[0]);
                                exit(0);
                	}

                	edid_file = optarg;
                        break;
                case '?':
                case 'h':
                        print_usage(argv[0]);
                        exit(0);
                default:
                        printf ("?? getopt returned character code 0%o ??\n", option);
                        print_usage(argv[0]);
                }
        }
        edid_CeaExtReset(&dev, &edidExt);
        edid_size = open_edid(edid_file, &edid);
        edid_parser(&dev, edid, &edidExt, edid_size);

        LOGGER(SNPS_NOTICE,"Is sink HDMI 2.0? %d\n", edidExt.edid_m20Sink);
        LOGGER(SNPS_NOTICE,"Number of SVDs parsed %d\n", edidExt.edid_mSvdIndex);
        LOGGER(SNPS_NOTICE,"First SVDs parsed VIC %d\n", edidExt.edid_mSvd[0].mCode);
        LOGGER(SNPS_NOTICE,"Second SVDs parsed VIC %d\n", edidExt.edid_mSvd[1].mCode);
        LOGGER(SNPS_NOTICE,"Third SVDs parsed VIC %d\n", edidExt.edid_mSvd[2].mCode);

	return 0;
}
