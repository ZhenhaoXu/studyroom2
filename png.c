#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

struct chunk_header {
	u_int size;
	char type[4];
}
#ifdef __GNUC__
__attribute__((__packed__))
#endif
;

struct ihdr_chunk {
	u_int breite;
	u_int hoehe;
	u_char bpp;
}
#ifdef __GNUC__
__attribute__((__packed__))
#endif
;

u_int big2littleEndianInt(u_int zahl);

int main(int argc, char* argv[])
{
	FILE *datei;
	struct chunk_header ch;
	char *dot;
	char pngsign[9] = "xPNG\r\nx\n";
	char sign[8];
	char type[5];
	char *chunk;
	struct ihdr_chunk *ihdr;

	pngsign[0] = 0x89;
	pngsign[6] = 0x1a;

	printf("sizeof chunk_header: %ld\n", sizeof(ch));
	printf("sizeof ihdr_chunk: %ld\n", sizeof(struct ihdr_chunk));

	if (argc < 2) {
		fprintf(stderr, "Bitte Dateiname übergeben\n");
		exit(1);
	}
	dot = strrchr(argv[1], '.');
	if (dot == NULL || strcasecmp(dot, ".png") != 0) {
		fprintf(stderr, "Das ist wohl keine PNG-datei!\n");
		exit(1);
	}

	/* Datei öffnen zum Lesen */
	datei = fopen(argv[1], "rb");
	if (datei == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if (fread(sign, 8, 1, datei) != 1) {
		fprintf(stderr, "PNG-Kopf konnte nicht gelesen werden\n");
		exit(1);
	}

	if (strncmp(sign, pngsign, 8) != 0) {
		fprintf(stderr, "Das ist keine PNG-datei\n");
		exit(1);
	}

	while(1) {
		if (fread(&ch, sizeof(ch), 1, datei) != 1) {
			fprintf(stderr, "PNG-Chunk-Header konnte nicht gelesen werden\n");
			exit(1);
		}
		strncpy(type, ch.type, 4);
		printf("Typ: %4s (%u)\n", type, big2littleEndianInt(ch.size));

		if (strcmp(type, "IEND") == 0)
			break;

		chunk = malloc(big2littleEndianInt(ch.size) + 4); /* plus 4 byte CRC */
		if (chunk == NULL) {
			perror("malloc");
			exit(1);
		}
		if (fread(chunk, big2littleEndianInt(ch.size)+4, 1, datei) != 1) {
			fprintf(stderr, "PNG-Chunk konnte nicht gelesen werden\n");
			exit(1);
		}
		if (strcmp(type, "IHDR") == 0) {
			ihdr = (struct ihdr_chunk *)chunk;

			printf("Breite: %u\n", ntohl(ihdr->breite));
			printf("Höhe: %u\n", ntohl(ihdr->hoehe));
			printf("BPP: %u\n", ihdr->bpp);
		}

		free(chunk);
	}

	fclose(datei);
	/*
	 */
	return 0;
}


u_int big2littleEndianInt(u_int zahl)
{
	u_int erg;
	char *von, *nach;
	int i;

	von = (char *)&zahl;
	nach = (char *) &erg + sizeof(zahl) - 1;

	for (i = 0; i < sizeof(zahl); i++) {
		*nach = *von;
		von++;
		nach--;
	}

	return erg;
}
