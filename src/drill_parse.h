
struct drill_hit {
	int tool;
	double x;
	double y;
};

struct drill_file {
	std::vector<double> drill_aps;
	std::list<struct drill_hit> hits;
};
struct drill_file * create_drill_file_rep_from_filename(char * filename);
void free_drill_file_rep(struct drill_file * rep);