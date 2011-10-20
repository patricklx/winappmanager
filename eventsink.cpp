#include "eventsink.h"
#include <QTime>
#include <QDebug>

//copied from http://207.46.172.252/en-us/library/aa390425(VS.85).aspx

/*
bool EventSink::connectWMI()
{
    HRESULT hres;

    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------

    hres =  CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
             << hex << hres << endl;
        return false;                  // Program has failed.
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

    hres =  CoInitializeSecurity(
        NULL,
        -1,                          // COM negotiates service
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
        );


    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x"
             << hex << hres << endl;
        CoUninitialize();
        return false;                      // Program has failed.
    }

    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *) &pLoc);

    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object. "
             << "Err code = 0x"
             << hex << hres << endl;
        CoUninitialize();
        return false;                 // Program has failed.
    }

    // Step 4: ---------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    pSvc = NULL;

    // Connect to the local root\cimv2 namespace
    // and obtain pointer pSvc to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );

    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x"
             << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return false;                // Program has failed.
    }

    cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x"
             << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return false;               // Program has failed.
    }

    // Step 6: -------------------------------------------------
    // Receive event notifications -----------------------------

    // Use an unsecured apartment for security
    pUnsecApp = NULL;

    hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
        CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
        (void**)&pUnsecApp);

    return true;
}

EventSink::processChild EventSink::waitForProcessChild(int pid,int timeout)
{
    connectWMI();

    IEnumWbemClassObject* pEnumerator = NULL;
    // The ExecNotificationQueryAsync method will call
    // The EventQuery::Indicate method when an event occurs
    HRESULT hres = pSvc->ExecNotificationQuery(
        _bstr_t("WQL"),
        _bstr_t("SELECT * "
            "FROM __InstanceCreationEvent WITHIN 1 "
            "WHERE TargetInstance ISA 'Win32_Process'"),
        WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    // Check for errors.
    if (FAILED(hres))
    {
        qDebug("ExecNotificationQueryAsync failed "
            "with = 0x%X\n", hres);
        pSvc->Release();
        pLoc->Release();
        pUnsecApp->Release();
        CoUninitialize();
        return processChild();
    }

    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;

    QTime watch;
    watch.start();
    processChild pchild;
    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(1000, 1,
            &pclsObj, &uReturn);

        if(watch.elapsed()>timeout)
        {
            pclsObj->Release();
            break;
        }

        if(hr == WBEM_S_TIMEDOUT)
            continue;

        if(0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;
        VariantClear(&vtProp);

        // Get the value of the Name property
        hr = pclsObj->Get(L"ParentProcessId", 0, &vtProp, 0, 0);
        int ppid = (int)vtProp.intVal;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"ProcessId", 0, &vtProp, 0, 0);
        int pid = (int)vtProp.intVal;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        QString name = (char*)vtProp.bstrVal;
        VariantClear(&vtProp);

        pclsObj->Release();

        if(ppid==pid)
        {
            pchild.pid = pid;
            pchild.name = name;
            break;
        }
    }


    // Cleanup
    // ========

    pSvc->Release();
    pLoc->Release();
    pUnsecApp->Release();
    CoUninitialize();

    return pchild;   // Program successfully completed.

}
*/
