#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static void
print_usage_and_die(const char *progname)
{
	fprintf(stderr,
		"usage:\n\t%s length [ characters ]\n\n"
		"provide a positive, nonzero length of bytes to display\n"
		"optionally, provide a character set to print from\n"
		, progname);

	exit(EXIT_FAILURE);
}

static int
parse_zonzero_ull(unsigned long long *dest, const char *input)
{
	char *end;

	/* we do not want the ability of strtoull wherein
	 * negative inputs give the result of the negation from ULL_MAX */
	if (strchr(input, '-'))
		return 0;

	errno = 0;
	*dest = strtoull(input, &end, 10);

	/* in range, valid input, fully parsed, nonzero result */
	return errno != ERANGE && *input && !*end && *dest;
}

static unsigned int
seed(void) {
	unsigned int s = 0;
	struct timespec now;
	FILE *urandom = fopen("/dev/urandom", "rb");

	if (urandom) {
		if (fread(&s, sizeof s, 1, urandom) != 1)
			s = 0;

		fclose(urandom);
	}
#if _POSIX_C_SOURCE >= 200809L
	if (clock_gettime(CLOCK_REALTIME, &now) != -1)
		s += 19937U * now.tv_sec + 3 * now.tv_nsec;

	s += 269U * getpid();
#else
	s += time(NULL);
#endif
	return s;
}

static void
display_n_bytes(unsigned long long bytes, const char *set)
{
	static const char *defset =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		" `~!@#$%^&*()_+-={}[]:\";'<>?,./|\\"
		;

	size_t length;

	if (!set)
		set = defset;

	length = strlen(set);

	while (bytes--)
		putchar(set[rand() % length]);
}

int
main(int argc, char **argv)
{
	unsigned long long bytes;

	if (argc < 2 || !parse_zonzero_ull(&bytes, argv[1]))
		print_usage_and_die(argv[0]);

	srand(seed());
	display_n_bytes(bytes, argv[2]);
}
