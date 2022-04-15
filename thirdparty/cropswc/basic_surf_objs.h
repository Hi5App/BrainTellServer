//
// Created by 黄磊 on 27.12.21.
//

#ifndef UNTITLED_BASIC_SURF_OBJS_H
#define UNTITLED_BASIC_SURF_OBJS_H
#include "color_xyz.h"
#include "v3d_basicdatatype.h"
#include <string>
#include <list>
#include <map>

struct BasicSurfObj
{
    V3DLONG n;				// index
    RGBA8 color;
    bool on;
    bool selected;
    std::string name;
    std::string comment;
    BasicSurfObj() {n=0; color.r=color.g=color.b=color.a=255; on=true;selected=false; name=comment="";}
};

// .marker marker files
//##########################################################################################
// 090617 RZC : image marker position is 1-based to consist with LocationSimple
// ATTENTION: it is easy to be chaos in 0/1-based coordinates!!!
//##########################################################################################

// .apo pointcloud files

struct CellAPO  : public BasicSurfObj
{
    float x, y, z;		// point coordinates
    float intensity;
    float sdev, pixmax, mass;
    float volsize;		// volume size
    std::string orderinfo;
    //char *timestamp;    // timestamp  LMG 26/9/2018
    std::string timestamp;		// timestamp  LMG 27/9/2018

    operator XYZ() const { return XYZ(x, y, z); }
    CellAPO() {x=y=z=intensity=volsize=sdev=pixmax=mass=0; timestamp=""; orderinfo="";}
};

std::list <CellAPO> readAPO_file(const std::string& filename);
bool writeAPO_file(const std::string& filename, const std::list <CellAPO> & listCell);

// .swc neurons and other graph-describing files

struct NeuronSWC : public BasicSurfObj
{
    int type;			// 0-Undefined, 1-Soma, 2-Axon, 3-Dendrite, 4-Apical_dendrite, 5-Fork_point, 6-End_point, 7-Custom
    float x, y, z;		// point coordinates

    union{
        float r;			// radius
        float radius;
    };

    union{
        V3DLONG pn;				// previous point index (-1 for the first point)
        V3DLONG parent;				// previous point index (-1 for the first point)
    };

    V3DLONG level; //20120217, by PHC. for ESWC format
    std::list<float> fea_val; //20120217, by PHC. for ESWC format

    V3DLONG seg_id; //this is reused for ESWC format, 20120217, by PHC
    V3DLONG nodeinseg_id; //090925, 091027: for segment editing

    V3DLONG creatmode;      // creation mode LMG 8/10/2018
    double timestamp;		// timestamp  LMG 27/9/2018

    double tfresindex;         // TeraFly resolution index LMG 13/12/2018

    operator XYZ() const { return XYZ(x, y, z); }
    NeuronSWC () {n=type=pn=0; x=y=z=r=0; seg_id=-1; nodeinseg_id=0; fea_val=std::list<float>(); level=-1; creatmode=0; timestamp=0; tfresindex=0;}
};



struct NeuronTree : public BasicSurfObj
{
    std::list <NeuronSWC> listNeuron;
    std::map <int, int>  hashNeuron;
    std::string file;
    bool editable;
    int linemode; //local control if a neuron will displayed as line or tube mode(s). by PHC 20130926
    bool flag;
    NeuronTree()
    {
        listNeuron.clear(); hashNeuron.clear(); file=""; editable=false;
        linemode=-1; //-1 is no defined. 0 is NO, and 1 is yes
        flag=false;
    }

    void deepCopy(const NeuronTree p)
    {
        n=p.n; color=p.color; on=p.on;
        selected=p.selected; name=p.name; comment=p.comment;

        file     = p.file;
        editable = p.editable;
        linemode = p.linemode;
        listNeuron.clear();
        hashNeuron.clear();

        for(auto &node:p.listNeuron)
        {
            NeuronSWC S;
            S.n = node.n;
            S.type = node.type;
            S.x = node.x;
            S.y= node.y;
            S.z = node.z;
            S.r = node.r;
            S.pn = node.pn;
            S.seg_id = node.seg_id;
            S.level = node.level;
            S.creatmode = node.creatmode;  // Creation Mode LMG 8/10/2018
            S.timestamp =node.timestamp;  // Timestamp LMG 27/9/2018
            S.tfresindex = node.tfresindex; // TeraFly resolution index LMG 13/12/2018
            S.fea_val = node.fea_val;
            listNeuron.insert(listNeuron.end(),node);

            hashNeuron[S.n]=listNeuron.size()-1;
        }

    }

    void copy(const NeuronTree & p)
    {
        n=p.n; color=p.color; on=p.on; selected=p.selected; name=p.name; comment=p.comment;
        listNeuron = p.listNeuron;
        hashNeuron = p.hashNeuron;
        file     = p.file;
        editable = p.editable;
        linemode = p.linemode;
    }
    void copyGeometry(const NeuronTree & p)
    {
        if (p.listNeuron.size()!=listNeuron.size()) return;
        auto it1=listNeuron.begin();
        auto it2=p.listNeuron.begin();
        for(;it1!=listNeuron.end();it1++,it2++)
        {
            it1->x=it2->x;
            it1->y=it2->y;
            it1->z=it2->z;
            it1->r=it2->r;
            it1->creatmode=it2->creatmode;
            it1->timestamp=it2->timestamp;
            it1->tfresindex=it2->tfresindex;
        }
    }
    bool projection(int axiscode=3) //axiscode, 1 -- x, 2 -- y, 3 -- z, 4 -- r
    {
        if (axiscode!=1 && axiscode!=2 && axiscode!=3 && axiscode!=4) return false;
        for(auto &node:listNeuron){
            if (axiscode==1) node.x = 0;
            else if (axiscode==2) node.y = 0;
            else if (axiscode==3) node.z = 0;
            else if (axiscode==4) node.r = 0.5;
        }
        return true;
    }
};

NeuronTree readSWC_file(const std::string& filename);
//bool writeSWC_file(const std::string& filename, const NeuronTree& nt);
bool writeESWC_file(const std::string& filename, const NeuronTree& nt);



inline bool operator==(CellAPO& a, CellAPO& b)
{
    return XYZ(a)==XYZ(b);
}

inline bool operator==(NeuronSWC& a, NeuronSWC& b)
{
    return XYZ(a)==XYZ(b);
}
inline bool operator==(NeuronTree& a, NeuronTree& b)
{
    return a.file==b.file;
}

#endif //UNTITLED_BASIC_SURF_OBJS_H
