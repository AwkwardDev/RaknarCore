#include "ThreadObject.h"

ObjThread::ObjThread(quint8 id, SpellWork *obj)
    : t_id(id), _iFace(obj)
{
    moveToThread(this);
}

ObjThread::~ObjThread()
{
}

void ObjThread::run()
{
    switch (t_id)
    {
        case THREAD_SEARCH:
        {
            new ObjectSearch(_iFace);
        }
        break;
    }
}