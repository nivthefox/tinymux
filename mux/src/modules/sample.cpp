/*! \file sample.cpp
 * \brief Sample Module
 *
 * $Id$
 *
 */

#include "../autoconf.h"
#include "../config.h"
#include "../libmux.h"
#include "../modules.h"
#include "sample.h"

static INT32 g_cComponents  = 0;
static INT32 g_cServerLocks = 0;

static ISample *g_pISample = NULL;

#define NUM_CLASSES 2
static CLASS_INFO sample_classes[NUM_CLASSES] =
{
    { CID_Sample },
    { CID_SumProxy }
};

// The following four functions are for access by dlopen.
//
extern "C" MUX_RESULT DCL_EXPORT DCL_API mux_CanUnloadNow(void)
{
    if (  0 == g_cComponents
       && 0 == g_cServerLocks)
    {
        return MUX_S_OK;
    }
    else
    {
        return MUX_S_FALSE;
    }
}

extern "C" MUX_RESULT DCL_EXPORT DCL_API mux_GetClassObject(MUX_CID cid, MUX_IID iid, void **ppv)
{
    MUX_RESULT mr = MUX_E_CLASSNOTAVAILABLE;

    if (CID_Sample == cid)
    {
        CSampleFactory *pSampleFactory = NULL;
        try
        {
            pSampleFactory = new CSampleFactory;
        }
        catch (...)
        {
            ; // Nothing.
        }

        if (NULL == pSampleFactory)
        {
            return MUX_E_OUTOFMEMORY;
        }

        mr = pSampleFactory->QueryInterface(iid, ppv);
        pSampleFactory->Release();
    }
    else if (CID_SumProxy == cid)
    {
        CSumProxyFactory *pSumProxyFactory = NULL;
        try
        {
            pSumProxyFactory = new CSumProxyFactory;
        }
        catch (...)
        {
            ; // Nothing.
        }

        if (NULL == pSumProxyFactory)
        {
            return MUX_E_OUTOFMEMORY;
        }

        mr = pSumProxyFactory->QueryInterface(iid, ppv);
        pSumProxyFactory->Release();
    }
    return mr;
}

extern "C" MUX_RESULT DCL_EXPORT DCL_API mux_Register(void)
{
    MUX_RESULT mr = MUX_E_UNEXPECTED;

    if (NULL == g_pISample)
    {
        // Advertise our components.
        //
        mr = mux_RegisterClassObjects(NUM_CLASSES, sample_classes, NULL);
        if (MUX_FAILED(mr))
        {
            return mr;
        }

        // Create an instance of our CSample component.
        //
        ISample *pISample = NULL;
        mr = mux_CreateInstance(CID_Sample, NULL, UseSameProcess, IID_ISample, (void **)&pISample);
        if (MUX_SUCCEEDED(mr))
        {
            g_pISample = pISample;
            pISample = NULL;
        }
        else
        {
            (void)mux_RevokeClassObjects(NUM_CLASSES, sample_classes);
            mr = MUX_E_OUTOFMEMORY;
        }
    }
    return mr;
}

extern "C" MUX_RESULT DCL_EXPORT DCL_API mux_Unregister(void)
{
    // Destroy our CSample component.
    //
    if (NULL != g_pISample)
    {
        g_pISample->Release();
        g_pISample = NULL;
    }

    return mux_RevokeClassObjects(NUM_CLASSES, sample_classes);
}

// Sample component which is not directly accessible.
//
CSample::CSample(void) : m_cRef(1)
{
    g_cComponents++;
    m_pILog = NULL;
    m_pIServerEventsControl = NULL;
}

#define LOG_ALWAYS      0x80000000  /* Always log it */

MUX_RESULT CSample::FinalConstruct(void)
{
    MUX_RESULT mr;

    // Use CLog provided by netmux.
    //
    mr = mux_CreateInstance(CID_Log, NULL, UseSameProcess, IID_ILog, (void **)&m_pILog);
    if (MUX_SUCCEEDED(mr))
    {
        if (m_pILog->start_log(LOG_ALWAYS, T("INI"), T("INFO")))
        {
            m_pILog->log_printf("CSample::CSample().");
            m_pILog->end_log();
        }
    }

    // Use CServerEventsSource to hook up our IServerEventsSink callback interface.
    //
    mux_IServerEventsSink *pIServerEventsSink = NULL;
    mr = QueryInterface(IID_IServerEventsSink, (void **)&pIServerEventsSink);
    if (MUX_SUCCEEDED(mr))
    {
        mr = mux_CreateInstance(CID_ServerEventsSource, NULL, UseSameProcess, IID_IServerEventsControl, (void **)&m_pIServerEventsControl);
        if (MUX_SUCCEEDED(mr))
        {
            m_pIServerEventsControl->Advise(pIServerEventsSink);
        }
        pIServerEventsSink->Release();
    }
    return mr;
}

CSample::~CSample()
{
    if (NULL != m_pILog)
    {
        if (m_pILog->start_log(LOG_ALWAYS, T("INI"), T("INFO")))
        {
            m_pILog->log_printf("CSample::~CSample().");
            m_pILog->end_log();
        }

        m_pILog->Release();
        m_pILog = NULL;
    }

    if (NULL != m_pIServerEventsControl)
    {
        m_pIServerEventsControl->Release();
        m_pIServerEventsControl = NULL;
    }

    g_cComponents--;
}

MUX_RESULT CSample::QueryInterface(MUX_IID iid, void **ppv)
{
    if (mux_IID_IUnknown == iid)
    {
        *ppv = static_cast<ISample *>(this);
    }
    else if (IID_ISample == iid)
    {
        *ppv = static_cast<ISample *>(this);
    }
    else if (IID_IServerEventsSink == iid)
    {
        *ppv = static_cast<mux_IServerEventsSink *>(this);
    }
    else
    {
        *ppv = NULL;
        return MUX_E_NOINTERFACE;
    }
    reinterpret_cast<mux_IUnknown *>(*ppv)->AddRef();
    return MUX_S_OK;
}

UINT32 CSample::AddRef(void)
{
    m_cRef++;
    return m_cRef;
}

UINT32 CSample::Release(void)
{
    m_cRef--;
    if (0 == m_cRef)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

int CSample::Add(int a, int b)
{
    return a + b;
}

// Factory for Sample component which is not directly accessible.
//
CSampleFactory::CSampleFactory(void) : m_cRef(1)
{
}

CSampleFactory::~CSampleFactory()
{
}

MUX_RESULT CSampleFactory::QueryInterface(MUX_IID iid, void **ppv)
{
    if (mux_IID_IUnknown == iid)
    {
        *ppv = static_cast<mux_IClassFactory *>(this);
    }
    else if (mux_IID_IClassFactory == iid)
    {
        *ppv = static_cast<mux_IClassFactory *>(this);
    }
    else
    {
        *ppv = NULL;
        return MUX_E_NOINTERFACE;
    }
    reinterpret_cast<mux_IUnknown *>(*ppv)->AddRef();
    return MUX_S_OK;
}

UINT32 CSampleFactory::AddRef(void)
{
    m_cRef++;
    return m_cRef;
}

UINT32 CSampleFactory::Release(void)
{
    m_cRef--;
    if (0 == m_cRef)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

MUX_RESULT CSampleFactory::CreateInstance(mux_IUnknown *pUnknownOuter, MUX_IID iid, void **ppv)
{
    // Disallow attempts to aggregate this component.
    //
    if (NULL != pUnknownOuter)
    {
        return MUX_E_NOAGGREGATION;
    }

    CSample *pSample = NULL;
    try
    {
        pSample = new CSample;
    }
    catch (...)
    {
        ; // Nothing.
    }

    MUX_RESULT mr;
    if (NULL == pSample)
    {
        return MUX_E_OUTOFMEMORY;
    }
    else
    {
        mr = pSample->FinalConstruct();
        if (MUX_FAILED(mr))
        {
            pSample->Release();
            return mr;
        }
    }

    mr = pSample->QueryInterface(iid, ppv);
    pSample->Release();
    return mr;
}

MUX_RESULT CSampleFactory::LockServer(bool bLock)
{
    if (bLock)
    {
        g_cServerLocks++;
    }
    else
    {
        g_cServerLocks--;
    }
    return MUX_S_OK;
}

// Called after all normal MUX initialization is complete.
//
void CSample::startup(void)
{
    m_pILog->log_printf("Sample module sees CSample::startup event." ENDLINE);
}

// This is called prior to the game syncronizing its own state to its own
// database.  If you depend on the the core database to store your data,
// you need to checkpoint your changes here. The write-protection
// mechanism in MUX is not turned on at this point.  You are guaranteed
// to not be a fork()-ed dumping process.
//
void CSample::presync_database(void)
{
    m_pILog->log_printf("Sample module sees CSample::presync_database event." ENDLINE);
}

// Like the above routine except that it called from the SIGSEGV handler.
// At this point, your choices are limited. You can attempt to use the core
// database. The core won't stop you, but it is risky.
//
void CSample::presync_database_sigsegv(void)
{
    m_pILog->log_printf("Sample module sees CSample::presync_database_sigsegv event." ENDLINE);
}

// This is called prior to the game database writing out it's own
// database.  This is typically only called from the fork()-ed process so
// write-protection is in force and you will be unable to modify the
// game's database for you own needs.  You can however, use this point to
// maintain your own dump file.
//
// The caveat is that it is possible the game will crash while you are
// doing this, or it is already in the process of crashing.  You may be
// called reentrantly.  Therefore, it is recommended that you follow the
// pattern in dump_database_internal() and write your database to a
// temporary file, and then if completed successfully, move your temporary
// over the top of your old database.
//
// The argument dump_type is one of the 5 DUMP_I_x defines declared in
// externs.h
//
void CSample::dump_database(int dump_type)
{
    m_pILog->log_printf("Sample module sees CSample::dump_database event." ENDLINE);
}

// The function is called when the dumping process has completed.
// Typically, this will be called from within a signal handler. Your
// ability to do anything interesting from within a signal handler is
// severly limited.  This is also called at the end of the dumping process
// if either no dumping child was created or if the child finished
// quickly. In fact, this may be called twice at the end of the same dump.
//
void CSample::dump_complete_signal(void)
{
    m_pILog->log_printf("Sample module sees CSample::dump_complete_signal event." ENDLINE);
}

// Called when the game is shutting down, after the game database has
// been saved but prior to the logfiles being closed.
//
void CSample::shutdown(void)
{
    m_pILog->log_printf("Sample module sees CSample::shutdown event." ENDLINE);
}

// Called after the database consistency check is completed.   Add
// checks for local data consistency here.
//
void CSample::dbck(void)
{
    m_pILog->log_printf("Sample module sees CSample::dbck event." ENDLINE);
}

// Called when a player connects or creates at the connection screen.
// isnew of 1 indicates it was a creation, 0 is for a connection.
// num indicates the number of current connections for player.
//
void CSample::connect(dbref player, int isnew, int num)
{
    m_pILog->log_printf("Sample module sees CSample::connect event." ENDLINE);
}

// Called when player disconnects from the game.  The parameter 'num' is
// the number of connections the player had upon being disconnected.
// Any value greater than 1 indicates multiple connections.
//
void CSample::disconnect(dbref player, int num)
{
    m_pILog->log_printf("Sample module sees CSample::disconnect event." ENDLINE);
}

// Called after any object type is created.
//
void CSample::data_create(dbref object)
{
    m_pILog->log_printf("Sample module sees CSample::data_create event." ENDLINE);
}

// Called when an object is cloned.  clone is the new object created
// from source.
//
void CSample::data_clone(dbref clone, dbref source)
{
    m_pILog->log_printf("Sample module sees CSample::data_clone event." ENDLINE);
}

// Called when the object is truly destroyed, not just set GOING
//
void CSample::data_free(dbref object)
{
    m_pILog->log_printf("Sample module sees CSample::data_free event." ENDLINE);
}

// SumProxy component which is not directly accessible.
//
CSumProxy::CSumProxy(void) : m_cRef(1)
{
    g_cComponents++;
}

MUX_RESULT CSumProxy::FinalConstruct(void)
{
    return MUX_S_OK;
}

CSumProxy::~CSumProxy()
{
    g_cComponents--;
}

MUX_RESULT CSumProxy::QueryInterface(MUX_IID iid, void **ppv)
{
    if (mux_IID_IUnknown == iid)
    {
        *ppv = static_cast<ISum *>(this);
    }
    else if (IID_ISum == iid)
    {
        *ppv = static_cast<ISum *>(this);
    }
    else if (mux_IID_IMarshal == iid)
    {
        *ppv = static_cast<mux_IMarshal *>(this);
    }
    else
    {
        *ppv = NULL;
        return MUX_E_NOINTERFACE;
    }
    reinterpret_cast<mux_IUnknown *>(*ppv)->AddRef();
    return MUX_S_OK;
}

UINT32 CSumProxy::AddRef(void)
{
    m_cRef++;
    return m_cRef;
}

UINT32 CSumProxy::Release(void)
{
    m_cRef--;
    if (0 == m_cRef)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

MUX_RESULT CSumProxy::GetUnmarshalClass(MUX_IID riid, marshal_context ctx, MUX_CID *pcid)
{
    return MUX_E_NOTIMPLEMENTED;
}

MUX_RESULT CSumProxy::MarshalInterface(size_t *pnBuffer, char **pBuffer, MUX_IID riid, marshal_context ctx)
{
    return MUX_E_NOTIMPLEMENTED;
}

MUX_RESULT CSumProxy::UnmarshalInterface(size_t nBuffer, char *pBuffer, MUX_IID riid, void **ppv)
{
    // TODO: Given a marshal packet from the remote component, we should be able to connect and provide a proxy ISum.
    //
    return QueryInterface(riid, ppv);
}

MUX_RESULT CSumProxy::ReleaseMarshalData(char *pBuffer)
{
    return MUX_S_OK;
}

MUX_RESULT CSumProxy::DisconnectObject(void)
{
    return MUX_E_NOTIMPLEMENTED;
}

MUX_RESULT CSumProxy::Add(int a, int b, int *sum)
{
    // TODO: Communicate with the remote component to service this request.
    //
    return MUX_E_NOTIMPLEMENTED;
}

// Factory for SumProxy component which is not directly accessible.
//
CSumProxyFactory::CSumProxyFactory(void) : m_cRef(1)
{
}

CSumProxyFactory::~CSumProxyFactory()
{
}

MUX_RESULT CSumProxyFactory::QueryInterface(MUX_IID iid, void **ppv)
{
    if (mux_IID_IUnknown == iid)
    {
        *ppv = static_cast<mux_IClassFactory *>(this);
    }
    else if (mux_IID_IClassFactory == iid)
    {
        *ppv = static_cast<mux_IClassFactory *>(this);
    }
    else
    {
        *ppv = NULL;
        return MUX_E_NOINTERFACE;
    }
    reinterpret_cast<mux_IUnknown *>(*ppv)->AddRef();
    return MUX_S_OK;
}

UINT32 CSumProxyFactory::AddRef(void)
{
    m_cRef++;
    return m_cRef;
}

UINT32 CSumProxyFactory::Release(void)
{
    m_cRef--;
    if (0 == m_cRef)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

MUX_RESULT CSumProxyFactory::CreateInstance(mux_IUnknown *pUnknownOuter, MUX_IID iid, void **ppv)
{
    // Disallow attempts to aggregate this component.
    //
    if (NULL != pUnknownOuter)
    {
        return MUX_E_NOAGGREGATION;
    }

    CSumProxy *pSumProxy = NULL;
    try
    {
        pSumProxy = new CSumProxy;
    }
    catch (...)
    {
        ; // Nothing.
    }

    MUX_RESULT mr;
    if (NULL == pSumProxy)
    {
        return MUX_E_OUTOFMEMORY;
    }
    else
    {
        mr = pSumProxy->FinalConstruct();
        if (MUX_FAILED(mr))
        {
            pSumProxy->Release();
            return mr;
        }
    }

    mr = pSumProxy->QueryInterface(iid, ppv);
    pSumProxy->Release();
    return mr;
}

MUX_RESULT CSumProxyFactory::LockServer(bool bLock)
{
    if (bLock)
    {
        g_cServerLocks++;
    }
    else
    {
        g_cServerLocks--;
    }
    return MUX_S_OK;
}