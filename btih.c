#include <ivartj/crypto.h>
#include <ivartj/bencoding.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

char *infile = NULL;
FILE *in = NULL;

void usage(FILE *out)
{
	fprintf(out, "usage: btih <torrent-file>\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "h", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 1:
		infile = argv[optind];
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}
}

void openfile(void)
{
	in = fopen(infile, "rb");
	if(in == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
}

static char *freadall(FILE *in, size_t *rlen);

char *freadall(FILE *in, size_t *rlen)
{
	size_t len, cap;
	int inc;
	char *out;

	len = 0;
	cap = 256;
	out = malloc(cap);

	while((inc = fread(out + len, 1, cap - len -1, in)) > 0) {
		len += inc;
		if(len + 1 == cap) {
			cap *= 2;
			out = realloc(out, cap);
		}
	}

	if(rlen != NULL)
		*rlen = len;

	out[len] = '\0';

	return out;
}

int main(int argc, char *argv[])
{
	char *benstring;
	unsigned char *hash;
	char *infostr;
	bencode_val *benval;
	bencode_val *info;
	size_t benstringlen;
	size_t infostrlen;
	size_t hashlen;
	int i;

	parseargs(argc, argv);
	openfile();

	benstring = freadall(in, &benstringlen);

	benval = bencode_parse(benstring, benstringlen);
	if(benval == NULL) {
		fprintf(stderr, "Failed to parse bencoding.\n");
		exit(EXIT_FAILURE);
	}

	info = bencode_dict_get((bencode_dict *)benval, "info");
	if(info == NULL) {
		fprintf(stderr, "Failed to find info segment of file.\n");
		exit(EXIT_FAILURE);
	}

	infostr = bencode_val_string(info, &infostrlen);

	hash = sha1(infostr, infostrlen, &hashlen);

	for(i = 0; i < hashlen; i++)
		printf("%.2hhX", hash[i]);
	puts("");

	free(infostr);
	free(hash);
	free(benstring);
	bencode_free_recursive(benval);

	exit(EXIT_SUCCESS);
}
