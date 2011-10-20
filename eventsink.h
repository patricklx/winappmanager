/*#ifndef EVENTSINK_H
#define EVENTSINK_H
#define _WIN32_DCOM

#include <iostream>
using namespace std;
#include <comdef.h>
#include <Wbemidl.h>
#include <QString>
//#pragma comment(lib, "wbemuuid.lib")

class EventSink
{
    IWbemServices *pSvc;
    IWbemLocator *pLoc;
    IUnsecuredApartment *pUnsecApp;
    bool connectWMI();


public:
    struct processChild{
        int pid;
        QString name;
    };

    processChild waitForProcessChild(int pid,int timeout = 3000);


};
#endif // EVENTSINK_H

*/
