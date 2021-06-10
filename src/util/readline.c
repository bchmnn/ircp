#include "util/readline.h"

#include <stdio.h>
#include <stdlib.h>

// https://stackoverflow.com/a/314422/14661040

char * readline(void) {
	char * line = malloc(100), * linep = line;
	size_t lenmax = 100, len = lenmax;
	int c;

	if(line == NULL)
		return NULL;

	for(;;) {
		c = fgetc(stdin);
		if(c == EOF)
			break;

		if(--len == 0) {
			len = lenmax;
			char * linen = realloc(linep, lenmax *= 2);

			if(linen == NULL) {
				free(linep);
				return NULL;
			}
			line = linen + (line - linep);
			linep = linen;
		}

		if((*line++ = c) == '\n')
			break;
	}
	*--line = '\0';
	return linep;
}
