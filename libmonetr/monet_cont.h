#ifndef MONET_CONT_H
#define MONET_CONT_H

//#include <c:/intel/bak/prj/lib13/include/db13.h>
//TODO: donno why it doesn't work under dev-cpp
#include <c:/intel/sid/prj/lib13/include/db13.h>

typedef objid13_t mn_contid_t;

struct monet_container {

    char* name;
    struct db13* db;

};

struct monet_cont_list {
    mn_contid_t n;
    struct monet_container* first, *last, *curr;
};

#endif // MONET_CONT_H
