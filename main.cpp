#include "chemfilter.h"
#include <QApplication>
#include <QSettings>
#include <QSysInfo>
#include <QDebug>

#include <openbabel/obconversion.h>

#ifdef WIN32
	#include <windows.h>
    #include <winnls.h>
#endif

#include "style/qplastiquestyle.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("BIGCHEM");
    QCoreApplication::setOrganizationDomain("bigchem.eu");
    QCoreApplication::setApplicationName("ChemFilter");

    a.setStyle(new QPlastiqueStyle());

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

    //init OpenBabel
    OpenBabel::OBConversion conv(NULL, NULL);

    ChemFilter w;
    w.show();

    return a.exec();
}
