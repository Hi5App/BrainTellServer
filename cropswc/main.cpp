#include <iostream>

using namespace std;
#include <iostream>
#include "basic_surf_objs.h"
#include "neuron_format_converter.h"
int main(int argc,char *argv[])
{
    auto swcname=argv[1];
    auto savepath=argv[8];
    auto x1=atof(argv[2]);
    auto x2=atof(argv[3]);
    auto y1=atof(argv[4]);
    auto y2=atof(argv[5]);
    auto z1=atof(argv[6]);
    auto z2=atof(argv[7]);

    std::cout<<swcname<<std::endl;
    std::cout<<savepath<<std::endl;
    std::cout<<x1<<" "<<x2<<" "<<y1<<" "<<y2<<" "<<z1<<" "<<z2<<endl;
    std::cout<<"1";
    NeuronTree nt=readSWC_file(swcname);
    V_NeuronSWC_list segments=NeuronTree__2__V_NeuronSWC_list(nt);
std::cout<<"1";
    V_NeuronSWC_list tosave;
    for(std::vector<V_NeuronSWC_unit>::size_type i=0;i<segments.seg.size();i++)
    {
        NeuronTree SS;
        const V_NeuronSWC &seg_temp =  segments.seg.at(i);
        for(std::vector<V_NeuronSWC_unit>::size_type j=0;j<seg_temp.row.size();j++)
        {
            if(seg_temp.row.at(j).x>=x1&&seg_temp.row.at(j).x<=x2
               &&seg_temp.row.at(j).y>=y1&&seg_temp.row.at(j).y<=y2
               &&seg_temp.row.at(j).z>=z1&&seg_temp.row.at(j).z<=z2)
            {
                tosave.seg.push_back(seg_temp);
                break;
            }
        }
    }

    NeuronTree savent=V_NeuronSWC_list__2__NeuronTree(tosave);
        writeESWC_file(savepath,savent);
        return 0;
}
