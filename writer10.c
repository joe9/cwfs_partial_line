#include <u.h>
#include <libc.h>

int debug = 0;
char teststring[] = "0123456789\n";

void usage(void);

void
main(int argc, char **argv)
{
	int fd;
	uvlong iters;
	int n, w;

	ARGBEGIN {
case 'd':
		debug = 1;
		break;
default:
		usage();
	}
	ARGEND;

	if(argc < 1)
		usage();
	if((fd = create(argv[0], OWRITE, 0664)) < 0)
		sysfatal("%s: can't open %s: %r\n", argv0, argv[0]);
	n = strlen(teststring);
	for (iters = 0; iters < 10 * 1024 * 1024; iters++) {
		if ((w = write(fd,teststring,n)) != n)
			sysfatal("write failed: w=%d, n=%d %r\n",w,n);
	}
	close(fd);            
	exits(nil);
}

void
usage(void)
{
	fprint(2, "usage: %s [-d] file\n", argv0);
	fprint(2, "-d: debug\n");
	exits("usage");
}
