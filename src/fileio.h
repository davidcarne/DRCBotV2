struct mapped_file {
	bool valid;
	unsigned long file_len;
	void * dataptr;
	int filedes;
	char * filename;
};

struct mapped_file map_file(char * filename);
void unmap_file(struct mapped_file * mfile);

