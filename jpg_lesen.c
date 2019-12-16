#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

struct segment_header {
	u_char type[2];
	u_short size;
}
#ifdef __GNUC__
__attribute__((__packed__))
#endif
;

struct ffc0_segment {
	u_char bpp;
	u_short hoehe;
	u_short breite;
}
#ifdef __GNUC__
__attribute__((__packed__))
#endif
;

int main(int argc, char* argv[])
{
	FILE *datei;
	char *dot;
	struct segment_header sh;
	char *segment;
	struct ffc0_segment *ffc0;

	printf("sizeof segment: %ld\n", sizeof(struct segment_header));

	printf("Zeiger byte: %ld\n", sizeof(segment));
	if (argc < 2) {
		fprintf(stderr, "Bitte Dateiname übergeben\n");
		exit(1);
	}
	dot = strrchr(argv[1], '.');
	if (dot == NULL || strcasecmp(dot, ".jpg") != 0) {
		fprintf(stderr, "Das ist wohl keine JPG-datei!\n");
		exit(1);
	}

	/* Datei öffnen zum Lesen */
	datei = fopen(argv[1], "rb");
	if (datei == NULL) {
		perror(argv[1]);
		exit(1);
	}
	if (fgetc(datei) != 0xff || fgetc(datei) != 0xd8) {
		fprintf(stderr, "JPG-Signatur konnte nicht gelesen werden oder ist falsch\n");
		exit(1);
	}

	while (1) {
		if (fread(&sh, sizeof(sh), 1, datei) != 1) {
			fprintf(stderr, "Segment-Header joinnte nicht gelesen werden\n");
			exit(1);
		}
		printf("Segment: %02X%02X (%hu)\n", sh.type[0], sh.type[1], ntohs(sh.size));

		segment = malloc(ntohs(sh.size) - sizeof(sh.size));
		if (segment == NULL) {
			perror("malloc");
			exit(1);
		}
		if (fread(segment, ntohs(sh.size) - sizeof(sh.size), 1, datei) != 1) {
			fprintf(stderr, "Segment konnte nicht gelesen werden\n");
			exit(1);
		}
		/* Segment für Segment einlesen bis FFC0 */
		if (sh.type[0] == 0xff && sh.type[1] == 0xc0)
	 		break;
		free (segment);
	}

	fclose(datei);
	/* FFC0 ist gelesen worden. Ausgabe der Kenndaten */

	ffc0 = (struct ffc0_segment *) segment;
	printf("Breite: %hu\n", ntohs(ffc0->breite));
	printf("Höhe: %hu\n", ntohs(ffc0->hoehe));
	printf("BPP: %u\n", ffc0->bpp);

	free(segment);
	return 0;
}



