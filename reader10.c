#include <u.h>
#include <libc.h>
#include <bio.h>

/*
mk clean; mk all ; rm /tmp/test.file /tmp/test.readcontents; touch /tmp/test.file; ./6.reader10 /tmp/test.file /tmp/test.readcontents & echo 'echo kill>/proc/'$apid'/note'; ./6.pwriter10 /tmp/test.file

sed -n '/iters=53806/,$p' /tmp/test.readcontents | xd -c
*/
enum {
	Len = 256,
	BufSize = 1 * 1024 * 1024,
};

void
usage(void)
{
	fprint(2,
	       "usage: %s inputfile outputfile\n",
	       argv0);
	fprint(2,
	       "keep reading lines 10 * 1024 * 1024 from the file. If any has a length < 10, write it to outputfile\n");
	exits("usage");
}

void
main(int argc, char **argv)
{
	uvlong iters, inoffset=0;
	long linelen;
	int fd, n, w,nhbuf,wfd;
	char *buf = mallocz(BufSize + 1,1);
	char hbuf[256], mbuf[] = ".";

	ARGBEGIN {
default:
		usage();
	}
	ARGEND;

	if(argc != 2) {
		fprint(2, "%s: mandatory argument filenames not provided\n",
		       argv0);
		usage();
	}

	if(strlen(argv[0]) >= Len)
		sysfatal
		    ("%s: file name length should be increased from %d to %ld",
		     argv0, Len, strlen(argv[0]) + 1);
	if(strlen(argv[1]) >= Len)
		sysfatal
		    ("%s: file name length should be increased from %d to %ld",
		     argv0, Len, strlen(argv[1]) + 1);
	fd = open(argv[0],OREAD);
	wfd = create(argv[1],OWRITE,0664);
	if (wfd < 0) sysfatal("cannot create file: %s %r\n",argv[1]);
	for (iters = 0; iters < 10 * 1024 * 1024;) {
		while((n = read(fd, buf, BufSize)) > 0) {
			iters++;
			inoffset += n;
			linelen=strlen(buf);
			if (n != linelen) fprint(2, "iters=%llud n=%d strlen(buf)=%ld\n",iters,n,linelen);
			nhbuf = snprint(hbuf,256,"iters=%llud input offset=%llud output offset=%llud, n=%d strlen(buf)=%ld\n",iters,inoffset,seek(wfd,0,1),n,linelen);
			write(wfd,hbuf,nhbuf);
			write(wfd,mbuf,1);
			if((w = write(wfd, buf, n)) != n) {
				sysfatal
				    ("%s: could only write %d of %d bytes: %r\n",
				     argv0, w, n);
			}
			write(wfd,mbuf,1);
			memset(buf,0,BufSize);
		}
		sleep(100);
	}

	free(buf);
	close(fd);
	close(wfd);
	exits(nil);
}
