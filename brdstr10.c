#include <u.h>
#include <libc.h>
#include <bio.h>

enum {
	Len = 256,
};

void
usage(void)
{
	fprint(2,
	       "usage: %s file\n",
	       argv0);
	fprint(2,
	       "keep reading lines 10 * 1024 * 1024 from the file. If any has a length < 10, write it out\n");
	exits("usage");
}

void
main(int argc, char **argv)
{
	uvlong iters;
	long linelen;
	Biobuf *bin;
	char *buf;

	ARGBEGIN {
default:
		usage();
	}
	ARGEND;

	if(argc != 1) {
		fprint(2, "%s: mandatory argument filename not provided\n",
		       argv0);
		usage();
	}

	if(strlen(argv[0]) >= Len)
		sysfatal
		    ("%s: file name length should be increased from %d to %ld",
		     argv0, Len, strlen(argv[0]) + 1);
	bin = Bopen(argv[0], OREAD);
	for (iters = 0; iters < 10 * 1024 * 1024;) {
		while((buf = Brdstr(bin, '\n', 1)) != nil) {
			iters++;
			linelen = Blinelen(bin);
			if (linelen != 10) fprint(2,"iters=%llud, Blinelen=%ld, strlen(buf)=%ld, buf=%s\n",iters,linelen, strlen(buf),buf);
			print("%s\n",buf);
			free(buf);
		}
		sleep(100);
	}

	Bterm(bin);
	exits(nil);
}
