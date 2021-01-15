
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
#include <stdlib.h>
#include <time.h>

#ifdef WIN32
    #include <windows.h>
    #include <winnls.h>
#endif

#include "../transformermodel.h"

std::string to_string(float x)
{
    char buf[256];
    sprintf(buf, "%g", x);

    return std::string(buf);
}

int main()
{
    srand(time(NULL));

#ifdef WIN32

    const int MAX_MODULE_NAME = 1024;
    char modulename[MAX_MODULE_NAME];

    int len = GetModuleFileNameA(NULL, modulename, 1024);
    if(len == 0)
    {
        fprintf(stderr, "Cannot determine the module path.\n");
        return 0;
    }

    char * p = strrchr(modulename, '\\');
    if( p == NULL )
    {
        fprintf(stderr, "Cannot analyze module's name.\n");
        return 0;
    }

    *++p = '\0';

    len = p - modulename;

    char env_babel_lib[MAX_MODULE_NAME + 30];
    sprintf(env_babel_lib, "BABEL_LIBDIR=%s/obplugins",modulename);

    putenv(env_babel_lib);

    char env_babel_data[MAX_MODULE_NAME + 30];
    sprintf(env_babel_data, "BABEL_DATADIR=%s/obdata",modulename);
    putenv(env_babel_data);

#endif

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
