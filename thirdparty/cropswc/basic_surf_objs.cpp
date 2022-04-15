//
// Created by 黄磊 on 27.12.21.
//

#include "basic_surf_objs.h"
#include "color_xyz.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <list>

std::vector<std::string> splitstring(const std::string &str, const char cs) {
    std::vector<std::string> res;
    std::istringstream iss(str);
    std::string token;
    while (getline(iss, token, cs)) {
        res.push_back(token);
    }
    return res;
}

std::list<CellAPO> readAPO_file(const std::string &filename) {
    std::list<CellAPO> mylist;

    std::ifstream qf;
    qf.open(filename, std::ios::in);
    if (!qf.is_open()) {
        return mylist;
    }

    int count = 0;
    mylist.clear();

    std::string buf;
    while (getline(qf, buf)) {
        auto it = buf.begin();
        for (; it < buf.end() && *it == ' '; it++);
        if (it == buf.end() || *it == '#') continue;

        count++;
        CellAPO S;

        auto qsl = splitstring(std::string(it, buf.end()), ' ');
        if (qsl.size() == 0) continue;
        for (std::string::size_type i = 0; i < qsl.size(); i++) {
            if (qsl[i].size() > 200) qsl[i].erase(200, qsl[i].size());

            if (i == 0) S.n = stoi(qsl[i]);
            else if (i == 1) S.orderinfo = qsl[i];
            else if (i == 2) S.name = qsl[i]; //strcpy(S.name, qsl[i].toStdString().c_str()); //by PHC, 090219
            else if (i == 3) S.comment = qsl[i]; //by PHC, added on 090220
            else if (i == 4) S.z = stof(qsl[i]);
            else if (i == 5) S.x = stof(qsl[i]);
            else if (i == 6) S.y = stof(qsl[i]);
            else if (i == 7) S.pixmax = stof(qsl[i]);
            else if (i == 8) S.intensity = stof(qsl[i]);
            else if (i == 9) S.sdev = stof(qsl[i]);
            else if (i == 10) S.volsize = stof(qsl[i]);
            else if (i == 11) S.mass = stof(qsl[i]);
            else if (i == 15) S.color.r = stoi(qsl[i]);
            else if (i == 16) S.color.g = stoi(qsl[i]);
            else if (i == 17) S.color.b = stoi(qsl[i]);
        }
        if (qsl.size() - 1 < 8)
            S.intensity = rand() % 255;
        if (qsl.size() - 1 < 10)
            S.volsize = 1;
        if (qsl.size() - 1 < 15)
            S.color = random_rgba8(255);
        S.on = true;

        mylist.push_back(S);

    }
    qf.close();

    return mylist;
}

bool writeAPO_file(const std::string &filename, const std::list<CellAPO> &listCell) {
    std::string curFile = filename;
    FILE *fp = fopen(curFile.c_str(), "wt");
    if (!fp) {
        int errNum = errno;
        printf("open fail errno = %d reason = %s \n", errNum, strerror(errNum));
        return false;
    }

    fprintf(fp, "##n,orderinfo,name,comment,z,x,y, pixmax,intensity,sdev,volsize,mass,,,, color_r,color_g,color_b\n");
    for (auto &cell: listCell) {
        fprintf(fp,
                "%ld, %s, %s,%s, %5.3f,%5.3f,%5.3f, %5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,,%d,%d,%d\n", //change from V3DLONG type to float, 20121212, by PHC
                cell.n, //i+1,

                cell.orderinfo.c_str(),
                cell.name.c_str(),
                cell.comment.c_str(),

                cell.z,
                cell.x,
                cell.y,

                cell.pixmax,
                cell.intensity,
                cell.sdev,
                cell.volsize,
                cell.mass,

                cell.color.r,
                cell.color.g,
                cell.color.b
        );
    }
    fclose(fp);
    return true;
}


NeuronTree readSWC_file(const std::string &filename) {
    NeuronTree nt;
    std::ifstream qf;
    qf.open(filename, std::ios::in);
    if (!qf.is_open()) {
        return nt;
    }

    int count = 0;
    std::list<NeuronSWC> listNeuron;
    std::map<int, int> hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    std::string name = "";
    std::string comment = "";
    nt.flag = true;

    std::string buf;
    while (getline(qf, buf)) {
        auto it = buf.begin();
        for (; it < buf.end() && *it == ' '; it++);
        if (it == buf.end()) continue;
        if (*it == '#') {
            if (std::string(it + 1, it + 6) == "name ") name = std::string(it + 6, buf.end());
            else if (std::string(it + 1, it + 9) == "comment ") comment = std::string(it + 9, buf.end());
            continue;
        }
        count++;
        NeuronSWC S;
        auto qsl = splitstring(std::string(it, buf.end()), ' ');
        if (qsl.size() == 0) continue;
        for (int i = 0; i < qsl.size(); i++) {
            if (qsl[i].size() > 99) qsl[i].erase(99, qsl[i].size());
            if (i == 0) S.n = stoi(qsl[i]);
            else if (i == 1) S.type = stoi(qsl[i]);
            else if (i == 2) S.x = stof(qsl[i]);
            else if (i == 3) S.y = stof(qsl[i]);
            else if (i == 4) S.z = stof(qsl[i]);
            else if (i == 5) S.r = stof(qsl[i]);
            else if (i == 6) S.pn = stoi(qsl[i]);
                //the ESWC extension, by PHC, 20120217
            else if (i == 7) S.seg_id = stol(qsl[i]);
            else if (i == 8) S.level = stol(qsl[i]);
            else if (i == 9) S.creatmode = stol(qsl[i]);
            else if (i == 10) S.timestamp = stod(qsl[i]);
            else if (i == 11) S.tfresindex = stod(qsl[i]);
                //change ESWC format to adapt to flexible feature number, by WYN, 20150602
            else
                S.fea_val.push_back(stof(qsl[i]));
        }

        //if (! listNeuron.contains(S)) // 081024
        {
            listNeuron.push_back(S);
            hashNeuron[S.n] = listNeuron.size() - 1;
        }
    }

    if (listNeuron.size() < 1)
        return nt;
    //now update other NeuronTree members

    nt.n = 1; //only one neuron if read from a file
    nt.listNeuron = listNeuron;
    nt.hashNeuron = hashNeuron;
    nt.color = XYZW(0, 0, 0, 0); /// alpha==0 means using default neuron color, 081115
    nt.on = true;
    if (*name.rbegin() == '\n') name.erase(name.size() - 1, 1);
    if (name.size() == 0) {
        auto pos = name.find_last_of('/');
        if (pos == std::string::npos)
            name = filename;
        else
            name = filename.substr(pos + 1);
    }
    nt.name = name;

    if (*comment.rbegin() == '\n') comment.erase(comment.size() - 1, 1);
    nt.comment = comment;
    return nt;
}

bool writeSWC_file(const std::string &filename, const NeuronTree &nt, const std::list<std::string> &infostring) {
    std::string curFile = filename;
    FILE *fp = fopen(curFile.c_str(), "wt");
    if (!fp) {

        return false;
    }

    fprintf(fp, "#name %s\n", nt.name.c_str());
    fprintf(fp, "#comment %s\n", nt.comment.c_str());

    if (infostring.size()) {
        for (auto &s: infostring)
            fprintf(fp, "#%s\n", s.c_str());
    }

    fprintf(fp, "##n,type,x,y,z,radius,parent\n");
    for (auto &p: nt.listNeuron) {
        fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld\n",
                p.n, p.type, p.x, p.y, p.z, p.r, p.pn);
    }
    fclose(fp);
    return true;
}

bool writeESWC_file(const std::string &filename, const NeuronTree &nt) {
    std::string curFile = filename;
    FILE *fp = fopen(curFile.c_str(), "wt");
    if (!fp) return false;

    fprintf(fp, "#name %s\n", nt.name.c_str());
    fprintf(fp, "#comment %s\n", nt.comment.c_str());

    fprintf(fp, "##n,type,x,y,z,radius,parent,seg_id,level,mode,timestamp,feature_value\n");
    for (auto &p: nt.listNeuron) {
        fprintf(fp, "%ld %d %5.3f %5.3f %5.3f %5.3f %ld %ld %ld %d %.0f",
                p.n, p.type, p.x, p.y, p.z, p.r, p.pn, p.seg_id, p.level, p.creatmode, p.timestamp);
        for (auto &v: p.fea_val) {
            fprintf(fp, " %.5f", v);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return true;
}


