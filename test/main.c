#include "../src/netsystem-proxy-interface.h"

int main(int argc, char **argv)
{
	if (2 != argc)
	{
		printf("use age:\n");
		printf("\t%s cmd\n", argv[0]);
		return 0;
	}

	printf("ret = %d\n", net_system_cmd_proxy(argv[1]));
    printf("ret_code = %s\n", get_netsystem_cmd_error_code());

	return 0;
}
