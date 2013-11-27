#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "LAKNode.h"
#include "MAKNode.h"

using namespace std;

int main(int argc, char*argv [])
{

    if(argc < 3){
        cout << "argument: [node_id] [algorithm]" << endl;
        cout << "[algorithm]: 0=Maekawa 1=LAK" << endl;
        return 0;
    }
    unsigned int nodeid = atoi(argv[1]);
    int algorithm = atoi(argv[2]);



    if(algorithm==1){
        cout << "LAK" << endl;
        LAKNode node(nodeid);
        node.init();
        node.start();
        //node.close();
    }else{
        cout << "MAK" << endl;
        MAKNode node(nodeid);
        node.init();
        node.start();
    }

    return 0;
}
