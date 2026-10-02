#include <unistd.h>
#include <string.h>

#include "Models.h"
#include "Quaternions.h"
#include "Serialize.h"

/* ----------------------------------------------------------------------
   These all read big-endian data, one, two or four bytes at a time, and the
   models are read entirely through them - 364,000 reads to load the game.
   Each one used to be a read() straight at the filesystem: on a desktop that
   is a syscall and merely slow, but on a Wii it is a trip through libfat to
   the SD card and it is the whole of the loading time.

   So the file is pulled in 64 KB at a time and the small reads are served
   from that.  One file is read at a time, start to finish, which is all this
   has to cope with; a seek or a close drops the buffer (see Support.h).
   ---------------------------------------------------------------------- */

static int           buf_fd = -1;
static unsigned char buf_data[64 * 1024];
static size_t        buf_len = 0;
static size_t        buf_pos = 0;

void SerializeDropBuffer(int fd)
{
	(void)fd;
	buf_fd = -1;
	buf_len = buf_pos = 0;
}

static size_t BufferedRead(int fd, void *out, size_t want)
{
	unsigned char *p = (unsigned char *)out;
	size_t got = 0;

	if (fd != buf_fd) {          /* a different file: start again */
		buf_fd = fd;
		buf_len = buf_pos = 0;
	}

	while (got < want) {
		if (buf_pos == buf_len) {
			const ssize_t n = read(fd, buf_data, sizeof(buf_data));

			if (n <= 0) break;   /* end of the file, or a broken one */

			buf_len = (size_t)n;
			buf_pos = 0;
		}

		size_t take = buf_len - buf_pos;

		if (take > want - got) take = want - got;

		memcpy(p + got, buf_data + buf_pos, take);

		buf_pos += take;
		got     += take;
	}

	return got;
}

#define read(fd, buf, n) BufferedRead((fd), (buf), (size_t)(n))

int ReadBool(int fd, int count, bool *b)
{
	while (count--) {
		unsigned char buf[1];
		
		if (read(fd, buf, 1) != 1) {
			STUB_FUNCTION;
		}
		
		*b = (buf[0] != 0) ? true : false;
		
		b++;
	}
	
	return 1;
}

int ReadShort(int fd, int count, short *s)
{
	while (count--) {
		unsigned char buf[2];
		
		if (read(fd, buf, 2) != 2) {
			STUB_FUNCTION;
		}
		
		*s = (short)((buf[0] << 8) | buf[1]);
		
		s++;
	}
	
	return 1;
}

int ReadInt(int fd, int count, int *s)
{
	while (count--) {
		unsigned char buf[4];
		
		if (read(fd, buf, 4) != 4) {
			STUB_FUNCTION;
		}
		
		*s = (int)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
		
		s++;
	}
	
	return 1;
}

union intfloat {
	int i;
	float f;
} intfloat;

int ReadFloat(int fd, int count, float *f)
{
	union intfloat infl;
	
	while (count--) {
		ReadInt(fd, 1, &(infl.i));
		
		*f = infl.f;
		
		f++;
	}
	
	return 1;
}

int ReadXYZ(int fd, int count, XYZ *xyz)
{
	while (count--) {
		ReadFloat(fd, 1, &(xyz->x));
		ReadFloat(fd, 1, &(xyz->y));
		ReadFloat(fd, 1, &(xyz->z));
		
		xyz++;
	}
	
	return 1;
}

int ReadTexturedTriangle(int fd, int count, TexturedTriangle *tt)
{
	while (count--) {
		short pad;
		ReadShort(fd, 3, tt->vertex);
		ReadShort(fd, 1, &pad); /* crud */
		ReadFloat(fd, 1, &(tt->r));
		ReadFloat(fd, 1, &(tt->g));
		ReadFloat(fd, 1, &(tt->b));
		
		tt++;
	}
	
	return count;
}
