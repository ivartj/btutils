#include <ivartj/crypto.h>
#include <ivartj/bencoding.h>
#include <stdlib.h>
#include <stdio.h>

static char *freadall(FILE *in, size_t *rlen);

char *freadall(FILE *in, size_t *rlen)
{
	size_t len, cap;
	int inc;
	char *out;

	len = 0;
	cap = 256;
	out = malloc(cap);

	while((inc = fread(out + len, 1, cap - len, in)) > 0) {
		len += inc;
		if(len == cap) {
			cap *= 2;
			out = realloc(out, cap + 1);
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
	size_t benstringlen;
	bencode_val *benval;
	bencode_val *info;
	char *infostr;
	size_t infostrlen;
	char *hash;
	size_t hashlen;
	int i;
	FILE *file;

	if(argc != 2) {
		fprintf(stderr, "Usage: btih <torrent-file>\n");
		exit(EXIT_FAILURE);
	}

	file = fopen(argv[1], "r");
	if(file == NULL) {
		fprintf(stderr, "Failed to open file.\n");
		exit(EXIT_FAILURE);
	}

	benstring = freadall(file, &benstringlen);

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
