#include "include\vm.h"

static void usage(int argc, char* argv[])
{
	printf("Usage: %s <image_name.exe>\n", argv[0]);
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		usage(argc, argv);
		return 1;
	}

	signal(SIGINT, handle_interrupt);
	disable_input_buffering();

	vm virtual_machine;

	vm_init(&virtual_machine);
	vm_load_image(&virtual_machine, argv[1]);

	for (;;)
	{
		vm_run(&virtual_machine);
	}

	restore_input_buffering();
	return 0;
}