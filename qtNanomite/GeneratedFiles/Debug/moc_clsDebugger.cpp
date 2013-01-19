/****************************************************************************
** Meta object code from reading C++ file 'clsDebugger.h'
**
** Created: Sat 19. Jan 02:04:18 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../clsDebugger/clsDebugger.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'clsDebugger.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_clsDebugger[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      31,   12,   12,   12, 0x05,
     100,   54,   12,   12, 0x05,
     181,  146,   12,   12, 0x05,
     282,  226,   12,   12, 0x05,
     364,  349,   12,   12, 0x05,
     410,  396,   12,   12, 0x05,
     462,  434,   12,   12, 0x05,
     609,  501,   12,   12, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_clsDebugger[] = {
    "clsDebugger\0\0OnDebuggerBreak()\0"
    "OnDebuggerTerminated()\0"
    "dwPID,dwTID,dwEP,bSuspended,dwExitCode,bFound\0"
    "OnThread(DWORD,DWORD,DWORD64,bool,DWORD,bool)\0"
    "dwPID,sFile,dwExitCode,dwEP,bFound\0"
    "OnPID(DWORD,std::wstring,DWORD,DWORD64,bool)\0"
    "sFuncName,sModName,dwOffset,dwExceptionCode,dwPID,dwTID\0"
    "OnException(std::wstring,std::wstring,DWORD64,DWORD64,DWORD,DWORD)\0"
    "sMessage,dwPID\0OnDbgString(std::wstring,DWORD)\0"
    "timeInfo,sLog\0OnLog(tm*,std::wstring)\0"
    "sDLLPath,dwPID,dwEP,bLoaded\0"
    "OnDll(std::wstring,DWORD,DWORD64,bool)\0"
    "dwStackAddr,dwReturnTo,sReturnToFunc,sModuleName,dwEIP,sFuncName,sFunc"
    "Module,sSourceFilePath,iSourceLineNum\0"
    "OnCallStack(DWORD64,DWORD64,std::wstring,std::wstring,DWORD64,std::wst"
    "ring,std::wstring,std::wstring,int)\0"
};

void clsDebugger::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        clsDebugger *_t = static_cast<clsDebugger *>(_o);
        switch (_id) {
        case 0: _t->OnDebuggerBreak(); break;
        case 1: _t->OnDebuggerTerminated(); break;
        case 2: _t->OnThread((*reinterpret_cast< DWORD(*)>(_a[1])),(*reinterpret_cast< DWORD(*)>(_a[2])),(*reinterpret_cast< DWORD64(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< DWORD(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6]))); break;
        case 3: _t->OnPID((*reinterpret_cast< DWORD(*)>(_a[1])),(*reinterpret_cast< std::wstring(*)>(_a[2])),(*reinterpret_cast< DWORD(*)>(_a[3])),(*reinterpret_cast< DWORD64(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 4: _t->OnException((*reinterpret_cast< std::wstring(*)>(_a[1])),(*reinterpret_cast< std::wstring(*)>(_a[2])),(*reinterpret_cast< DWORD64(*)>(_a[3])),(*reinterpret_cast< DWORD64(*)>(_a[4])),(*reinterpret_cast< DWORD(*)>(_a[5])),(*reinterpret_cast< DWORD(*)>(_a[6]))); break;
        case 5: _t->OnDbgString((*reinterpret_cast< std::wstring(*)>(_a[1])),(*reinterpret_cast< DWORD(*)>(_a[2]))); break;
        case 6: _t->OnLog((*reinterpret_cast< tm*(*)>(_a[1])),(*reinterpret_cast< std::wstring(*)>(_a[2]))); break;
        case 7: _t->OnDll((*reinterpret_cast< std::wstring(*)>(_a[1])),(*reinterpret_cast< DWORD(*)>(_a[2])),(*reinterpret_cast< DWORD64(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 8: _t->OnCallStack((*reinterpret_cast< DWORD64(*)>(_a[1])),(*reinterpret_cast< DWORD64(*)>(_a[2])),(*reinterpret_cast< std::wstring(*)>(_a[3])),(*reinterpret_cast< std::wstring(*)>(_a[4])),(*reinterpret_cast< DWORD64(*)>(_a[5])),(*reinterpret_cast< std::wstring(*)>(_a[6])),(*reinterpret_cast< std::wstring(*)>(_a[7])),(*reinterpret_cast< std::wstring(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData clsDebugger::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject clsDebugger::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_clsDebugger,
      qt_meta_data_clsDebugger, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &clsDebugger::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *clsDebugger::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *clsDebugger::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_clsDebugger))
        return static_cast<void*>(const_cast< clsDebugger*>(this));
    return QThread::qt_metacast(_clname);
}

int clsDebugger::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void clsDebugger::OnDebuggerBreak()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void clsDebugger::OnDebuggerTerminated()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void clsDebugger::OnThread(DWORD _t1, DWORD _t2, DWORD64 _t3, bool _t4, DWORD _t5, bool _t6)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)), const_cast<void*>(reinterpret_cast<const void*>(&_t6)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void clsDebugger::OnPID(DWORD _t1, std::wstring _t2, DWORD _t3, DWORD64 _t4, bool _t5)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void clsDebugger::OnException(std::wstring _t1, std::wstring _t2, DWORD64 _t3, DWORD64 _t4, DWORD _t5, DWORD _t6)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)), const_cast<void*>(reinterpret_cast<const void*>(&_t6)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void clsDebugger::OnDbgString(std::wstring _t1, DWORD _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void clsDebugger::OnLog(tm * _t1, std::wstring _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void clsDebugger::OnDll(std::wstring _t1, DWORD _t2, DWORD64 _t3, bool _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void clsDebugger::OnCallStack(DWORD64 _t1, DWORD64 _t2, std::wstring _t3, std::wstring _t4, DWORD64 _t5, std::wstring _t6, std::wstring _t7, std::wstring _t8, int _t9)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)), const_cast<void*>(reinterpret_cast<const void*>(&_t6)), const_cast<void*>(reinterpret_cast<const void*>(&_t7)), const_cast<void*>(reinterpret_cast<const void*>(&_t8)), const_cast<void*>(reinterpret_cast<const void*>(&_t9)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_END_MOC_NAMESPACE
