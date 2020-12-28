
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <math.h>

#include "../transformermodel.h"

std::string to_string(float x)
{
    char buf[256];
    sprintf(buf, "%g", x);

    return std::string(buf);
}

void calcMeanAndError(const std::vector<float> &data,
                      float * avg,
                      float * err)
{
    float x2 = 0.0;
    *avg = 0.0;

    for(int i=0; i< data.size(); i++)
    {
        x2 += data[i] * data[i];
        *avg += data[i];
    }

    const int N = data.size();
    *avg /= N;

    float s2 = (x2 - (*avg) * (*avg) * N) / (N - 1);
    if(s2 < 0)
        s2 = 0.0;

    float ci = student(N - 1) * sqrt( s2 ) / sqrt(N);
    *err = fabs(round(200.0 * ci / (*avg + 1e-3)));
}

int main()
{
    if(chdir("models"))
    {
        std::cerr << "Directory models is inaccessible: errno "
                  << errno << ": " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<class TransformerModel *> vm;
    std::map<std::string, std::vector<float> > R;

    int n_prop = 0;
    int n_mdl = 0;

    DIR * dir = opendir (".");
    struct dirent * dir_models = NULL;
    while ( (dir_models = readdir (dir)) )
    {
        const char * mdl = dir_models->d_name;
        if(!strncmp(mdl, "..", 2) || !strncmp(mdl, ".", 1))
            continue;

        if(chdir(mdl))
            continue;

        bool is_counted = false;

        DIR * model = opendir(".");
        struct dirent * dir_model = NULL;
        while((dir_model = readdir( model )))
        {
            const char * r_mdl = dir_model->d_name;
            if(!strncmp(r_mdl, "..", 2) || !strncmp(r_mdl, ".", 1) || !strstr(r_mdl, ".trm"))
                continue;

            TransformerModel * tm = new TransformerModel(r_mdl, mdl);
            if(tm->isGood())
            {
                vm.push_back(tm);
                n_mdl++;

                if(!is_counted)
                {
                    is_counted = true;
                    n_prop++;

                    R[mdl] = std::vector<float>();
                }
            }
            else
                delete tm;
        }

        closedir(model);
        chdir("..");
    }

    closedir(dir);


    if(vm.size() == 0)
    {
        std::cerr << "No models to compute." << std::endl;
        return EXIT_FAILURE;
    }

    char line [ 1024];
    while( fgets(line, 1024, stdin))
    {
        int len = strlen(line);
        if(line[len-1] == '\n')
            line[len-1] = '\0';

        for(std::map<std::string, std::vector<float> >::iterator it = R.begin(); it!= R.end(); ++it)
            it->second.clear();

        int n = 0;
        std::set<std::string> mols;
        if(GetRandomSmiles(line, mols, n))
        {
            TransformerModel::ResultValue res = vm[0]->predict(mols, n);
            if(res.valid)
            {
                std::vector<float> & d = R[vm[0]->getProp()];
                for(int i=0; i< res.size; i++)
                    d.push_back(res.value[i]);

                float * embeddings = vm[0]->getSmilesEmbeddings();
                for(int m=1; m< vm.size(); m++)
                {
                    TransformerModel::ResultValue res = vm[m]->predict(mols, n, embeddings);
                    if(res.valid)
                    {
                        std::vector<float> & d = R[vm[m]->getProp()];
                        for(int i=0; i< res.size; i++)
                            d.push_back(res.value[i]);
                    }
                }
            }

            bool first = true;
            std::string json("{");
            for(std::map<std::string, std::vector<float> >::iterator it = R.begin(); it!= R.end(); ++it)
            {
                float err, avg;
                calcMeanAndError(it->second, &avg, &err);

                if(!first) json += ",";

                json += "\"" + it->first + "\": ";
                json += to_string(avg) + ",";
                json += "\"" + it->first + "_d\": ";
                json += to_string(err);


                first = false;
            }

            json += "}";
            std::cout << json << std::endl;
        }
        else
            std::cout << "error conversion" << std::endl;

    }

    for(int i=0; i< vm.size(); i++)
       delete vm[i];

    vm.clear();

    return EXIT_SUCCESS;
}
