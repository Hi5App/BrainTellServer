//
// Created by 黄磊 on 27.12.21.
//
#include "basic_surf_objs.h"
#include "neuron_format_converter.h"
NeuronTree V_NeuronSWC__2__NeuronTree(V_NeuronSWC & seg) // convert V_NeuronSWC to Vaa3D's external neuron structure NeuronTree
{
    NeuronTree SS;

    std::list <NeuronSWC> listNeuron;
    std::map <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    {
        int count = 0;
        for (int k=0;k<seg.row.size();k++)
        {
            count++;
            NeuronSWC S;

            S.n 	= seg.row.at(k).data[0];
            S.type 	= seg.row.at(k).data[1];
            S.x 	= seg.row.at(k).data[2];
            S.y 	= seg.row.at(k).data[3];
            S.z 	= seg.row.at(k).data[4];
            S.r 	= seg.row.at(k).data[5];
            S.pn 	= seg.row.at(k).data[6];

            //for hit & editing
            S.seg_id       = seg.row.at(k).seg_id;
            S.nodeinseg_id = seg.row.at(k).nodeinseg_id;

            S.level = seg.row.at(k).level;

            //for timestamping and quality control LMG 8/10/2018
            S.creatmode = seg.row.at(k).creatmode;
            S.timestamp = seg.row.at(k).timestamp;
            S.tfresindex = seg.row.at(k).tfresindex;

            //qDebug("%s  ///  %d %d (%g %g %g) %g %d", buf, S.n, S.type, S.x, S.y, S.z, S.r, S.pn);

            //if (! listNeuron.contains(S)) // 081024
            {
                listNeuron.insert(listNeuron.end(),S);
                hashNeuron[S.n]= listNeuron.size()-1;
            }
        }
//		printf("---------------------read %d lines, %d remained lines", count, listNeuron.size());

        SS.n = -1;
        SS.color = XYZW(seg.color_uc[0],seg.color_uc[1],seg.color_uc[2],seg.color_uc[3]);
        SS.on = true;
        SS.listNeuron = listNeuron;
        SS.hashNeuron = hashNeuron;

        SS.name = seg.name.c_str();
        SS.file = seg.file.c_str();
    }

    return SS;
}

NeuronTree V_NeuronSWC_list__2__NeuronTree(V_NeuronSWC_list & tracedNeuron) //convert to Vaa3D's external neuron structure
{
    //first conversion
    V_NeuronSWC seg = merge_V_NeuronSWC_list(tracedNeuron);
    seg.name = tracedNeuron.name;
    seg.file = tracedNeuron.file;

    //second conversion
    return V_NeuronSWC__2__NeuronTree(seg);
}

V_NeuronSWC_list NeuronTree__2__V_NeuronSWC_list(NeuronTree * nt)           //convert to V3D's internal neuron structure
{
    if (!nt)
        return V_NeuronSWC_list();

    V_NeuronSWC cur_seg;	cur_seg.clear();

    for(auto &node:nt->listNeuron)
    {
        V_NeuronSWC_unit v;
        v.n=node.n;
        v.type=node.type;
        v.x=node.x;
        v.y=node.y;
        v.z=node.z;
        v.r=node.r;
        v.parent=node.parent;
        v.level=node.level;
        v.creatmode=node.creatmode;
        v.timestamp=node.timestamp;
        v.tfresindex=node.tfresindex;
        cur_seg.append(v);
    }
    cur_seg.name = "1";
    cur_seg.b_linegraph=false; //do not forget to do this

    V_NeuronSWC_list editableNeuron;

    editableNeuron.seg = cur_seg.decompose(); //////////////

    editableNeuron.name = nt->name;
    editableNeuron.file = nt->file;
    editableNeuron.b_traced = false;
    editableNeuron.color_uc[0] = nt->color.r;
    editableNeuron.color_uc[1] = nt->color.g;
    editableNeuron.color_uc[2] = nt->color.b;
    editableNeuron.color_uc[3] = nt->color.a;

    return editableNeuron;
}



V_NeuronSWC_list NeuronTree__2__V_NeuronSWC_list(NeuronTree & nt)           //convert to V3D's internal neuron structure. overload for convenience
{
    return NeuronTree__2__V_NeuronSWC_list( &nt );
}

