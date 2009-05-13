#include <vector>
#include <list>

#include <stdlib.h>
#include <stdio.h>

#include "drill_parse.h"

struct drill_parse_mode {
	bool mode_cfg;
	bool parse_more;
	bool units_metric;
	
	enum {
		P_LZ,
		P_TZ, 
		P_FLOAT
	} coord_mode;
	
	int tool
};
static bool parse_m_cmd(struct drill_file * rep, struct drill_parse_mode *m, char * l)
{
	if (strlen(l) < 3)
		return false;
	
	int mcmd = atoi(l+m);
	switch (mcmd)
	{
		case 72:
		case 48:
		case 30:
			
		default:
			printf("Unsupported drill command, ignoring\n");
			return false;
	}

}
static bool parse_drill_line(struct drill_file * rep, struct drill_parse_mode *m, char * l)
{
	// Check if we're going from config to mode
	if (strcmp(l,"%") == 0)
	{
		m->mode_cfg = false;
		return true;
	}
	
	switch (l[0])
	{
		case 'M':
			return parse_m_cmd(rep,m,l);
		case 'T':
			if (
		case 'X':
	}

}

struct drill_file * create_drill_file_rep_from_filename(char * filename)
{
	struct drill_parse_mode m;
	m.mode_cfg = true;
	m.parse_more = true;
	FILE * f = fopen(filename,"r");
	if (!f)
	{
		perror("File");
		return NULL;
	}
	
	
	struct drill_file * rep = new drill_file;

	// initialize
	rep->drill_aps = std::vector<double>();
	rep->list = std::list<struct drill_hit>();

	
	while (!feof(f) && m.parse_more)
	{	
		char buf[1024];
		fgets(f,buf,1023);
		parse_drill_line(rep,m,buf);
	}
	fclose(f);
	return rep;
}
void free_drill_file_rep(struct drill_file * rep)
{
	delete rep;
}