#include <ivartj/bencoding.h>
#include <ivartj/crypto.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

char *infile = NULL;
FILE *in = NULL;

void usage(FILE *out)
{
	fprintf(out, "usage: bturl <torrent-file>\n");
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
	case '1':
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
	if(in == NULL)
		perror("fopen");
}

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

const char *metainfo_announce(bencode_val *metainfo)
{
	bencode_val *val;
	val = bencode_dict_get((bencode_dict *)metainfo, "announce");
	if(val->type != BENCODE_STRING)
		return NULL;
	return val->string.val;
}

char *metainfo_urlinfohash(bencode_val *metainfo, size_t *rlen)
{
	bencode_val *info;
	char *infostr;
	size_t infostrlen;
	unsigned char *hash;
	size_t hashlen;
	char *out;
	size_t len;
	int i;

	info = bencode_dict_get((bencode_dict *)metainfo, "info");
	if(info == NULL)
		return NULL;

	infostr = bencode_val_string(info, &infostrlen);

	hash = sha1(infostr, infostrlen, &hashlen);

	free(infostr);

	len = hashlen * 31;
	out = malloc(len + 1);
	out[len] = '\0';

	for(i = 0; i < hashlen; i++) {
		sprintf(out + i * 3, "%%%.2hhX", hash[i]);
	}

	free(hash);

	if(rlen != NULL)
		*rlen = len;

	return out;
}

int main(int argc, char *argv[])
{
	char *benstr;
	size_t benstrlen;
	bencode_val *metainfo;
	const char *announce;
	char *urlinfohash;

	parseargs(argc, argv);
	openfile();

	benstr = freadall(in, &benstrlen);

	metainfo = bencode_parse(benstr, benstrlen);
	free(benstr);
	if(metainfo == NULL) {
		fprintf(stderr, "Failed to parse bencoding.\n");
		exit(EXIT_FAILURE);
	}

	announce = metainfo_announce(metainfo);
	if(announce == NULL) {
		fprintf(stderr, "Failed to retrieve announce URL from torrent file.\n");
		exit(EXIT_FAILURE);
	}

	urlinfohash = metainfo_urlinfohash(metainfo, NULL);
	if(urlinfohash == NULL) {
		fprintf(stderr, "Couldn't find info segment of file.\n");
		exit(EXIT_FAILURE);
	}

	printf("%s?info_hash=%s\n", announce, urlinfohash);

	free(urlinfohash);
	bencode_free_recursive(metainfo);

	exit(EXIT_SUCCESS);
}
