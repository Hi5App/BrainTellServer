//
// Created by 黄磊 on 27.12.21.
//

#ifndef UNTITLED_NEURON_FORMAT_CONVERTER_H
#define UNTITLED_NEURON_FORMAT_CONVERTER_H

#include "basic_surf_objs.h"
#include "v_neuronswc.h"

NeuronTree V_NeuronSWC__2__NeuronTree(V_NeuronSWC & tracedNeuronSeg);// convert V_NeuronSWC to Vaa3D's external neuron structure NeuronTree
NeuronTree V_NeuronSWC_list__2__NeuronTree(V_NeuronSWC_list & tracedNeuron); //convert to V3D's external neuron structure

V_NeuronSWC_list NeuronTree__2__V_NeuronSWC_list(NeuronTree * nt);           //convert to V3D's internal neuron structure
V_NeuronSWC_list NeuronTree__2__V_NeuronSWC_list(NeuronTree & nt);           //convert to V3D's internal neuron structure. overload for convenience

#endif //UNTITLED_NEURON_FORMAT_CONVERTER_H
