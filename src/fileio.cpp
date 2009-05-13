#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fileio.h"
#include "main.h"

struct mapped_file map_file(char * filename)
{
	struct stat file_stat;
	struct mapped_file mfile;
	mfile.valid = false;
	
	// We allocate our own copy of the filename - must free it ourselves
	mfile.filename = strdup(filename);

	// Collect vital information about the file
	int stat_err = stat(filename, &file_stat);
	
	if (stat_err)
	{
		if (debug_level >= DEBUG_ERROR)
			perror("Could not stat file:");
		return mfile;
	}

	// Ensure that its a regular file
	if (!S_ISREG(file_stat.st_mode))
	{
		if (debug_level >= DEBUG_ERROR)
			perror("ERROR! - not a regular file!");
		return mfile;
	}

	off_t file_size = file_stat.st_size;
	mfile.file_len = file_size;

	if (debug_level >= DEBUG_VERBOSE)
	{
		printf("Input file name = %s\n", filename);
		printf("Input file size = %u\n", file_size);
	}
	
	// Try to open the file
	int file_des = open(filename, O_RDONLY);
	mfile.filedes = file_des;
	if (file_des == -1)
	{
		// an error occurred in opening the file
		if (debug_level >= DEBUG_ERROR)
			perror("Error could not open file!");
		return mfile;
	}

	// now try to map it
	// MAP_NORESERVE is good for large files, but it doesn't work on windows, and if we've got multi
	// gig gerbers - we've got more to worry about
 	void * file_data = mmap(0, file_size, PROT_READ, MAP_PRIVATE/* | MAP_NORESERVE*/, file_des, 0);
	mfile.dataptr = file_data;

	if (file_data == MAP_FAILED)
	{
		if (debug_level >= DEBUG_ERROR)
			perror("Couldn't map file");
		// Clean up by closing the file
		close(file_des);
		return mfile;
	}
	
	// Everything suceeded - now mark the mapped file as valid
	mfile.valid = true;

	return mfile;
}

void unmap_file(struct mapped_file * mfile)
{
	// make sure its valid
	if (!mfile->valid)
		return;
	
	mfile->valid = false;

	munmap(mfile->dataptr, mfile->file_len);
	close(mfile->filedes);
	free(mfile->filename);
}

