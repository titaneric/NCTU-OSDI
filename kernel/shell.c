#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/shell.h>
#include <inc/timer.h>

struct Command
{
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char **argv);
};

static struct Command commands[] = {
	{"help", "Display this list of commands", mon_help},
	{"kerninfo", "Display information about the kernel", mon_kerninfo},
	{"print_tick", "Display system tick", print_tick},
	{"chgcolor", "Change shell color", chgcolor}};
#define NCOMMANDS (sizeof(commands) / sizeof(commands[0]))

int mon_help(int argc, char **argv)
{
	int i;

	for (i = 0; i < NCOMMANDS; i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int mon_kerninfo(int argc, char **argv)
{
	/* TODO: Print the kernel code and data section size 
   * NOTE: You can count only linker script (kernel/kern.ld) to
   *       provide you with those information.
   *       Use PROVIDE inside linker script and calculate the
   *       offset.
   */
	extern char kernel_load_addr[], etext[], data[], end[];
	cprintf("Kernel code base start=0x%x size=%d\n", kernel_load_addr, etext - kernel_load_addr);
	cprintf("Kernel data base start=0x%x size=%d\n", data, end - data);
	cprintf("Kernel executable memory footprint: %dKB\n", (end - kernel_load_addr) >> 10);
	return 0;
}
int print_tick(int argc, char **argv)
{
	cprintf("Now tick = %d\n", get_tick());
}
int chgcolor(int argc, char **argv)
{
	if (argc < 2)
	{
		cprintf("Not enough arguments!\n");
		return 1;
	}
	char forecolor = argv[1][0];
	if (forecolor >= '0' && forecolor <= '9')
	{
		forecolor -= '0';
	}
	else if (forecolor >= 'A' && forecolor <= 'F')
	{
		forecolor = forecolor - 'A' + 10;
	}
	else
	{
		forecolor = 15;
	}
	settextcolor(forecolor, 0);
	cprintf("Change color %d\n", forecolor);
	return 0;
}
#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int runcmd(char *buf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1)
	{
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS - 1)
		{
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < NCOMMANDS; i++)
	{
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}
void shell()
{
	char *buf;
	cprintf("Welcome to the OSDI course!\n");
	cprintf("Type 'help' for a list of commands.\n");

	while (1)
	{
		buf = readline("OSDI> ");
		if (buf != NULL)
		{
			if (runcmd(buf) < 0)
				break;
		}
	}
}
