#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ivartj/io.h>
#include <ivartj/bencoding.h>

char *outfile = NULL;
FILE *out = NULL;
char *infile = NULL;
FILE *in = NULL;
bencode_val *doc;

void usage(FILE *out)
{
	fprintf(out, "usage: ben2js [ -o <output-file> ] [ <input-file> ]\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "out", required_argument, NULL, 'o' },
		{ "help", no_argument, NULL, 'h' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "ho:", longopts, NULL)) != -1)
	switch(c) {
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case 'o':
		outfile = optarg;
		break;
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		break;
	case 1:
		infile = argv[optind];
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}
}

void openfiles(void)
{
	if(infile != NULL) {
		in = fopen(infile, "r");
		if(in == NULL)
			perror("fopen");
	} else
		in = stdin;

	if(outfile != NULL) {
		out = fopen(outfile, "w");
		if(out == NULL)
			perror("fopen");
	} else
		out = stdout;
}

void parse(void)
{
	io_reader r;
	r.read = io_fread;
	r.data = in;

	doc = bencode_parse_reader(&r);
	if(doc == NULL) {
		fprintf(stderr, "Failed to parse bencode.\n");
		exit(EXIT_FAILURE);
	}
}

void print(void)
{
	char *js;
	size_t jslen, written;

	js = bencode_val_json(doc, &jslen);
	written = fwrite(js, 1, jslen, out);
	if(written != jslen) {
		fprintf(stderr, "Failed to write JSON.\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	parseargs(argc, argv);
	openfiles();
	parse();
	print();
	exit(EXIT_SUCCESS);
}
