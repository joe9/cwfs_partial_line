#include <u.h>
#include <libc.h>
#include <thread.h>

#include <autism.h>

/* multiple pwrites in parallel */

enum {
	Nwriters = 50,          /* 100 fails */
	Stack = 8 * KiB,
	Rdiounit = 8 * KiB,
	Wriounit = 8 * KiB,
};

int debug = 0;
char teststring[] = "0123456789\n";

struct WriteUnit {
	vlong offset;
	int sz;
	uchar *buf;
};
typedef struct WriteUnit WriteUnit;

struct Parg {
	Channel *donec, *sendc;
	int fd;
	int i;                  /* index */
};
typedef struct Parg Parg;

void usage(void);
void writer(void *a);
int min(int a, int b);

void
threadmain(int argc, char **argv)
{
	int fd;
	uvlong offset = 0, iters;
	int n, i, j, nsend;
	char *buf;
	Parg arg[Nwriters];
	WriteUnit wrunit[Nwriters];
	Alt alts[Nwriters + 1];

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
	memset(arg, 0, sizeof arg);
	memset(wrunit, 0, sizeof wrunit);
	memset(alts, 0, sizeof alts);
	for(i = 0; i < Nwriters; i++) {
		arg[i].i = i;
		arg[i].fd = fd;
		wrunit[i].buf = emalloc(Wriounit);
		arg[i].sendc = chancreate(sizeof(WriteUnit *), 0);
		alts[i].c = arg[i].donec = chancreate(sizeof(int *), 0);
		alts[i].op = CHANRCV;
		proccreate(writer, arg + i, Stack);
	}
	alts[i].c = alts[i].v = nil;
	alts[i].op = CHANEND;
	n = strlen(teststring);
	buf = teststring;
	for (iters = 0; iters < 10 * 1024 * 1024; iters++) {
		for(j = 0; j < n; offset += nsend, j += nsend) {
			i = alt(alts);
			nsend = min(Wriounit, n - j);
			wrunit[i].offset = offset;
			wrunit[i].sz = nsend;
			memcpy(wrunit[i].buf, buf + j, nsend);
			if(nsend != n)
				fprint(2,
				       "something is off, not sending 11 bytes"
				       " n=%d j=%d nsend=%d buf=%s, buf+j=%s\n",
				       n, j, nsend, (char *) buf,
				       (char *) buf + j);
			D(2, "%s channel=%d offset=%llud sz=%d buf=%s\n",
			   argv0, i, offset, nsend, buf+j);
			if(sendp(arg[i].sendc, wrunit + i) != 1)
				sysfatal("%s: sendp failure i=%d, %r\n",
					 argv0, i);
		}
	}
	D(2, "%s exiting\n", argv0);
	for(i = 0; i < Nwriters; i++) {
		recvp(arg[i].donec);
		sendp(arg[i].sendc, nil);
		chanclose(arg[i].sendc);
		chanclose(arg[i].donec);
		chanfree(arg[i].sendc);
		chanfree(arg[i].donec);
	}
	close(fd);              /* why bother? could just exit */
	threadexitsall(nil);
}

void
usage(void)
{
	fprint(2, "usage: %s [-d] file\n", argv0);
	fprint(2, "-d: debug\n");
	exits("usage");
}

void
writer(void *a)
{
	Parg *arg = a;
	WriteUnit *w;
	long n;

	threadsetname("writer %d", arg->i);
	if(send(arg->donec, nil) != 1)
		sysfatal("could not send initial loading value: %r\n");;
	while((w = recvp(arg->sendc)) != nil) {
		/* D(2, ",%d,", w->sz); */
		/* D(2, "writer %d offset=%llud sz=%d\n", arg->i,
		   w->offset, w->sz); */
		if((n =
		    pwrite(arg->fd, w->buf, w->sz, w->offset)) != w->sz)
			sysfatal
			    ("%s: could only write %ld of %d bytes: %r\n",
			     argv0, n, w->sz);
		/* D(2, ".%d.", w->sz); */
		if(send(arg->donec, nil) != 1)
			break;
	}
	D(2, "writer %d exiting on receiving nil\n", arg->i);
	threadexits(nil);
}

int
min(int a, int b)
{
	return (a < b) ? a : b;
}
