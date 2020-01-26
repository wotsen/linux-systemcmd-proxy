#include <stdio.h>
#include "../include/systemcmd_proxy_interface.h"

using namespace wotsen;

int main(int argc, char **argv)
{
	if (2 != argc)
	{
		printf("use age:\n");
		printf("\t%s cmd\n", argv[0]);
		return 0;
	}

	printf("%s ret = %d\n", get_systemcmd_proxy_version(), systemcmd_proxy(argv[1]));

	return 0;
}
