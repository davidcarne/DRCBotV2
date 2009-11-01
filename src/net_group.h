
/********************* net_group *********************/
net_group * new_net_group(Vector_Outp * f);
void group_together(Vector_Outp * f, GerbObj * a, GerbObj * b);

class net_group {
	friend void add_to_group(Vector_Outp * f, net_group * n, GerbObj * o);
	friend net_group * new_net_group(Vector_Outp * f);
	
public:
	
	typedef std::set<GerbObj*> gObjSet_t;
	typedef gObjSet_t::iterator i_gObjSet_t;
	
	i_gObjSet_t start(void);
	i_gObjSet_t end(void);
	
	float r,g,b;
	int getSize() {return members.size();};
	int getID() {return set_id;};
protected:
	net_group(int id);
private:
	int set_id;
	std::set <GerbObj*> members;
};
