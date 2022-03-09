//
// Created by 黄磊 on 27.12.21.
//

#ifndef UNTITLED_V_NEURONSWC_H
#define UNTITLED_V_NEURONSWC_H
#include "v3d_basicdatatype.h"
#include "basic_surf_objs.h"
#include <vector>
#include <string>
#include <map>
using namespace std;

struct V_NeuronSWC_coord    //for sort
{
    union {
        double data[3];
        struct { double x,y,z; };
    };
    bool equal(const V_NeuronSWC_coord & other) const {if (x==other.x && y==other.y && z==other.z) return true; else return false;}
    bool equal(double x1, double y1, double z1) const {if (x==x1 && y==y1 && z==z1) return true; else return false;}
    bool nonequal(const V_NeuronSWC_coord &other) const {return !equal(other);}
    bool nonequal(double x1, double y1, double z1) const {return !equal(x1, y1, z1);}
    void set(double x1, double y1, double z1) {x=x1; y=y1; z=z1;}
};

inline bool operator == (const V_NeuronSWC_coord a, const V_NeuronSWC_coord b)
{
    return (a.x==b.x && a.y==b.y && a.z==b.z);
}

inline bool operator < (const V_NeuronSWC_coord a, const V_NeuronSWC_coord b)
{
    return ((a.x<b.x) ||
            (a.x==b.x && a.y<b.y) ||
            (a.x==b.x && a.y==b.y && a.z<b.z)
    );
}

inline double distL2square(const V_NeuronSWC_coord & a, const V_NeuronSWC_coord & b)
{
    return ((a.x-b.x)*(a.x-b.x) +
            (a.y-b.y)*(a.y-b.y) +
            (a.z-b.z)*(a.z-b.z) );
}

struct V_BranchUnit // for ranking segment hierarchy, used in loop detection
{
    V_BranchUnit() { isBranch = false; x = 0; y = 0; z = 0; paID = 0; hierarchy = 0; }
    bool isBranch;
    float x, y, z;
    int ID, paID;
    int hierarchy;
    vector<int> childIDs;

    int segLoc, segPaLoc;
    vector<int> childSegLocs;
};

struct V_NeuronSWC_unit
{
    union {
        double data[10];
        struct {
            double n, type, x, y, z, r, parent,
                    nchild,
                    seg_id, nodeinseg_id,
                    level, creatmode, timestamp, tfresindex;
        };
    };
    V_NeuronSWC_unit() {for (V3DLONG i=0;i<V3DLONG(sizeof(data)/sizeof(double));i++) data[i]=0; r=0.5;}
    operator V_NeuronSWC_coord() {V_NeuronSWC_coord c; c.x=x; c.y=y; c.z=z; return c;}
    V_NeuronSWC_coord get_coord() {V_NeuronSWC_coord c; c.x=x; c.y=y; c.z=z; return c;}
    void set(double x1, double y1, double z1, double r1, double p1, double t1) {x=x1; y=y1; z=z1; r=r1;parent=p1;type=t1;}
    void set(double x1, double y1, double z1, double r1, double p1) {x=x1; y=y1;z=z1;r=r1;parent=p1;}
    void set(double x1, double y1, double z1, double r1) {x=x1; y=y1;z=z1;r=r1;}
    void set(double x1, double y1, double z1) {x=x1; y=y1;z=z1;}
    void setType(double t) {type = t;}

    operator XYZ()const  {return XYZ(x,y,z);}
};

inline double distL2square(const V_NeuronSWC_unit & a, const V_NeuronSWC_unit & b)
{
    return ((a.x-b.x)*(a.x-b.x) +
            (a.y-b.y)*(a.y-b.y) +
            (a.z-b.z)*(a.z-b.z) );
}

struct Node_Link  // for graph form of swc
{
    V3DLONG i;
    vector<V3DLONG> in_link;
    vector<V3DLONG> out_link;
    V3DLONG nlink;
};
typedef map<V3DLONG, Node_Link> Link_Map;
typedef map<V3DLONG, Node_Link>::iterator Link_Map_Iter;

struct V_SWCNodes
{
    vector <V3DLONG> nid_array; //the array of node id (node id is the 1st column value)
    vector <V_NeuronSWC_coord> ncoord_array; //following the same order of nid_array
    map <V3DLONG, V3DLONG> nid_ipos_lut; //a look up table to return the index of an nid, i.e. nid_ipos_lut[nid_array.at(i)] equals i;
};

struct V_NeuronSWC
{
    vector <V_NeuronSWC_unit> row;
    bool b_linegraph;
    string name;
    string comment;
    string file;
    unsigned char color_uc[4];
    bool b_jointed;
    bool to_be_deleted;   // @ADDED by Alessandro on 2015-05-08. Needed to support late delete of multiple neuron segments.
    bool to_be_broken;
    bool on; //Added by Y. Wang on 2016-05-25. For the segment-wise display of a SWC.
    long long segid;
    bool check_data_consistency() {/* to verify if unique node id have unique coord, and if parent are in the nid, except -1*/ return true;}

    V_NeuronSWC(string new_name="unset", bool is_linegraph=false)
    {
        name=new_name; b_linegraph=is_linegraph;  *(int*)color_uc=0; b_jointed=false;
        to_be_deleted = false;
        to_be_broken = false;
        on = true;
    }

    V_BranchUnit branchingProfile;

    void printInfo();

    V3DLONG nrows() {return row.size();}

    V_SWCNodes unique_nodes_info(); //determine the unique nodes

    vector<V3DLONG> unique_nid(); //unique node id (id is the first column value in SWC)
    V3DLONG n_unique_nid(); //number of unique node ids
    vector<V_NeuronSWC_coord> unique_ncoord(); //unique node coordinates (coordinates are the 3rd to 5th column)
    V3DLONG n_unique_ncoord(); //number of unique node coords

    V3DLONG maxnoden() //091029 change maxnoden from >=-1 to >=0 for base_n in set_simple_path...
    {
        V3DLONG maxn=0;	for (V3DLONG i=0;i<(V3DLONG)row.size();i++) if (row.at(i).n > maxn) maxn = row.at(i).n;		return maxn;
    }
    V3DLONG getIndexofParent(V3DLONG j)
    {
        V3DLONG res=-1; V3DLONG parent = row.at(j).parent;
        for (V3DLONG i=0;i<(V3DLONG)row.size();i++) if (row.at(i).n==parent)	{res=i; break;}
        return res;
    }
    vector<V3DLONG> getIndexofParent_nodeid(V3DLONG nid) //return the array of of node "nid"'s parents' nid
    {
        vector<V3DLONG> res;
        for (V3DLONG i=0;i<(V3DLONG)row.size();i++)
        {
            if (row.at(i).n==nid)
            {
                V3DLONG curparent = row.at(i).parent;
                bool b_exist=false;
                for (V3DLONG j=0;j<(V3DLONG)res.size();j++)
                    if (res.at(j)==curparent) {	b_exist=true; break;}
                if (!b_exist)
                    res.push_back(curparent);
            }
        }
        return res;
    }

    void append(V_NeuronSWC_unit & new_node) {row.push_back(new_node);}
    void clear() {row.clear();}
    vector <V_NeuronSWC> decompose();
    bool reverse();

    bool isLineGraph() {return b_linegraph;} //just return the "claimed" property is a line graph
    //check if a 3D location is contained in the swc
    V3DLONG getFirstIndexof3DPos(double x,double y,double z) //return -1 is no included, othwise return the first detected index
    {
        V3DLONG res=-1;
        for (V3DLONG i=0;i<(V3DLONG)row.size();i++) if (row.at(i).data[2]==x && row.at(i).data[3]==y && row.at(i).data[4]==z)	{res=i; break;}
        return res;
    }
    V3DLONG getFirstIndexof3DPos(const V_NeuronSWC_unit & subject_node) {return getFirstIndexof3DPos(subject_node.data[2], subject_node.data[3], subject_node.data[4]);}
    V3DLONG getFirstIndexof3DPos(const V_NeuronSWC_unit * subject_node) {return getFirstIndexof3DPos(subject_node->data[2], subject_node->data[3], subject_node->data[4]);}

    vector<V3DLONG> getAllIndexof3DPos(double x,double y,double z, V3DLONG noninclude_ind) //return all indexes except the one indicated as noninclude_ind
    {
        vector<V3DLONG> res;
        for (V3DLONG i=0;i<(V3DLONG)row.size();i++) if (row.at(i).data[2]==x && row.at(i).data[3]==y && row.at(i).data[4]==z)	{ if (i!=noninclude_ind) res.push_back(i); }
        return res;
    }
    vector <V3DLONG> getAllIndexof3DPos(const V_NeuronSWC_unit & subject_node, V3DLONG noninclude_ind) {return getAllIndexof3DPos(subject_node.data[2], subject_node.data[3], subject_node.data[4], noninclude_ind);}
    vector <V3DLONG> getAllIndexof3DPos(const V_NeuronSWC_unit * subject_node, V3DLONG noninclude_ind) {return getAllIndexof3DPos(subject_node->data[2], subject_node->data[3], subject_node->data[4], noninclude_ind);}
};

struct V_NeuronSWC_list
{
    vector <V_NeuronSWC> seg; //since each seg could be a complete neuron or multiple paths, thus I call it "seg", but not "path"
    V3DLONG last_seg_num; //?? for what purpose? seems only used once in v3d_core.cpp. Questioned by Hanchuan, 20100210
    string name;
    string comment;
    string file;
    unsigned char color_uc[4];
    bool b_traced;

    V_NeuronSWC_list() {last_seg_num=-1; *(int*)color_uc=0; b_traced=true;}

    V3DLONG nsegs() {return seg.size();}
    V3DLONG nrows() {V3DLONG n=0; for (V3DLONG i=0;i<(V3DLONG)seg.size();i++) n+=seg.at(i).nrows(); return n;}
    V3DLONG maxnoden()
    {
        V3DLONG maxn=0;	for (V3DLONG i=0;i<(V3DLONG)seg.size();i++) if (seg.at(i).maxnoden() > maxn) maxn = seg.at(i).maxnoden();	return maxn;
    }
    bool isJointed() {return nsegs()==1 && seg.at(0).b_jointed;}

    void append(V_NeuronSWC & new_seg) {seg.push_back(new_seg); last_seg_num=seg.size();}
    void append(vector <V_NeuronSWC> & new_segs) {for (int k=0; k<(int)new_segs.size(); k++) seg.push_back(new_segs.at(k)); last_seg_num=seg.size();}
    void clear() {last_seg_num=seg.size(); seg.clear();}
    void merge();
    void decompose();
    bool reverse();
    bool split(V3DLONG seg_id, V3DLONG nodeinseg_id);
    bool deleteSeg(V3DLONG seg_id);

    // @ADDED by Alessandro on 2015-05-08. Needed to support late delete of multiple neuron segments.
    void                                            // no value returned
    deleteMultiSeg(                             // by default, deletes neuron segments having 'to_be_deleted' field set to 'true'
            std::vector <V3DLONG> *seg_ids = 0);    // if provided, deletes the corresponding neuron segments.
};

bool verifyIsLineGraph(const V_NeuronSWC & in_swc); //this will use graph algorithm to verify if really a line graph as claimed

///////////////////////////////
//// NOTE: merge_xxx() and join_xxx() operations are different! merge() means just putting together, but join_xxx() means more, also including removing redundant nodes.
//////////////////////////////

Link_Map get_link_map(const V_NeuronSWC & in_swc);
vector <V_NeuronSWC> decompose_V_NeuronSWC(V_NeuronSWC & in_swc);
V_NeuronSWC join_V_NeuronSWC_vec(vector <V_NeuronSWC> & in_swc_vec);

bool reverse_V_NeuronSWC_inplace(V_NeuronSWC & in_swc);
bool change_type_in_seg_of_V_NeuronSWC_list(V_NeuronSWC_list & swc_list, V3DLONG seg_id, int type);
bool change_radius_in_seg_of_V_NeuronSWC_list(V_NeuronSWC_list & swc_list, V3DLONG seg_id, double radius);
bool change_level_in_seg_of_V_NeuronSWC_list(V_NeuronSWC_list & swc_list, V3DLONG seg_id, int level);

V_NeuronSWC merge_V_NeuronSWC_list(V_NeuronSWC_list & in_swc_list);
bool delete_seg_in_V_NeuronSWC_list(V_NeuronSWC_list & swc_list, V3DLONG seg_id); //delete a seg in the V_NeuronSWC_list
double length_seg_in_V_NeuronSWC_list(V_NeuronSWC_list & swc_list, V3DLONG seg_id); //find the length of a seg
double getLength_V_NeuronSWC(V_NeuronSWC & subject_swc); //compute the length of a swc neuron
V_NeuronSWC join_segs_in_V_NeuronSWC_list(V_NeuronSWC_list & swc_list, V3DLONG seg_id_array[], int n_segs); //merge several segs in V_NeuronSWC_list, indexed by seg_id_array (length is n_segs)
bool join_two_V_NeuronSWC_old(V_NeuronSWC & destination_swc, V_NeuronSWC & subject_swc); //add the subject_swc neuron to the destination_swc, and merge all overlaping nodes
bool join_two_V_NeuronSWC(V_NeuronSWC & destination_swc, V_NeuronSWC & subject_swc); //add the subject_swc neuron to the destination_swc, and merge all overlaping nodes

V3DLONG find_node_in_V_NeuronSWC(V_NeuronSWC & in_swc, double x, double y, double z); //find the id of a node
V3DLONG find_seg_in_V_NeuronSWC_list(V_NeuronSWC_list & swc_list, double x, double y, double z, V3DLONG & nodeinseg_id ); //find the id of a seg

V_NeuronSWC_list split_V_NeuronSWC_simplepath(V_NeuronSWC & in_swc, V3DLONG nodeinseg_id);
V_NeuronSWC_list split_V_NeuronSWC_simplepath(V_NeuronSWC & in_swc, double x, double y, double z);

map <V3DLONG,V3DLONG> unique_V_NeuronSWC_nodeindex(V_NeuronSWC & in_swc);
bool simplify_V_NeuronSWC_nodeindex(V_NeuronSWC & my_swc); //map the node index of a swc neuron to the range 1~N (N is the number of unique indexes)



template <class T> //should be a struct at least included members (x,y,z)
void set_simple_path_unit (V_NeuronSWC_unit & v, V3DLONG base_n, vector<T> & mUnit, V3DLONG i, bool link_order, double r=1, double default_type=3, double creatmode=0, double default_timestamp=10, double default_tfresindex = 0)
{
    V3DLONG N = mUnit.size();
    v.type	= default_type;
    v.x 	= mUnit[i].x;
    v.y 	= mUnit[i].y;
    v.z 	= mUnit[i].z;
    v.r 	= r;
    v.creatmode = creatmode;
    v.timestamp = default_timestamp; // LMG 11/10/2018
    v.tfresindex = default_tfresindex; // LMG 13/12/2018
    if (link_order) // same as index order
    {
        v.n		= base_n +1+i;
        v.parent = (i>=N-1)? -1 : (v.n +1);
    }
    else // reversed link order
    {
        v.n		= base_n +1+i;
        v.parent = (i<=0)? -1 : (v.n -1);
    }
}
template <class T> //should be a struct at least included members (x,y,z)
void set_simple_path (V_NeuronSWC & cur_seg, V3DLONG base_n, vector<T> & mUnit, bool link_order, double r=1, double default_type=3, double creatmode=0, double default_timestamp=0, double default_tfresindex = 0)
{
    cur_seg.clear();
    for (V3DLONG i=0;i<mUnit.size();i++)
    {
        V_NeuronSWC_unit v;
        set_simple_path_unit(v, base_n, mUnit, i, link_order, r, default_type, creatmode, default_timestamp, default_tfresindex); //LMG 11/10/2018
        cur_seg.append(v);
        //qDebug("%d ", cur_seg.nnodes());
    }
}
template <class T> //should be a struct at least included members (n, parent)
void reset_simple_path_index (V3DLONG base_n, vector<T> & mUnit)
{
    V3DLONG N = mUnit.size();
    for (V3DLONG i=0; i<mUnit.size(); i++)
    {
        if (mUnit[0].parent >=1) // same as index order
        {
            mUnit[i].n = base_n +1+i;
            mUnit[i].parent = (i>=N-1)? -1: (mUnit[i].n +1);
        }
        else                    // reversed link order
        {
            mUnit[i].n = base_n +1+i;
            mUnit[i].parent = (i<=0)? -1: (mUnit[i].n -1);
        }
    }
}
template <class T>
V3DLONG value_in_vector(vector<T> & vec, T & val) //a <0 return value indicating the value does not exist in vector
{
    if (vec.size()<=0) return -1;
    for (V3DLONG i=0;i<vec.size();i++)
        if (vec.at(i)==val)
            return i;
    return -1;
}
template <class T>
vector<T> unique_values_in_vector(vector<T> & vec) //return a new vector where value are unique value in vec
{
    if (vec.size()<=0) return vec;
    vector<T> vnew;
    vnew.push_back(vec.at(0));
    for (V3DLONG i=1;i<vec.size();i++)
    {
        if (value_in_vector(vnew, vec.at(i))<0) //not exist yet
            vnew.push_back(vec.at(i));
    }
    return vnew;
}
#endif //UNTITLED_V_NEURONSWC_H
