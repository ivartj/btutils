#include <ivartj/bencoding.h>
#include <ivartj/crypto.h>
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
	char *hash;
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
	FILE *file;
	char *benstr;
	size_t benstrlen;
	bencode_val *metainfo;
	const char *announce;
	char *urlinfohash;

	if(argc != 2) {
		fprintf(stderr, "Usage: btget <torrent-file>\n");
		exit(EXIT_FAILURE);
	}

	file = fopen(argv[1], "rb");
	if(file == NULL) {
		fprintf(stderr, "Failed  to open file.\n");
		exit(EXIT_FAILURE);
	}

	benstr = freadall(file, &benstrlen);

	fclose(file);

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
