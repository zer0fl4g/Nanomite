/****************************************************************************
** Meta object code from reading C++ file 'clsCallbacks.h'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../clsCallbacks.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'clsCallbacks.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_clsCallbacks[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      32,   18,   14,   13, 0x0a,
     102,   56,   14,   13, 0x0a,
     183,  148,   14,   13, 0x0a,
     284,  228,   14,   13, 0x0a,
     366,  351,   14,   13, 0x0a,
     426,  398,   14,   13, 0x0a,
     573,  465,   14,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_clsCallbacks[] = {
    "clsCallbacks\0\0int\0timeInfo,sLog\0"
    "OnLog(tm*,std::wstring)\0"
    "dwPID,dwTID,dwEP,bSuspended,dwExitCode,bFound\0"
    "OnThread(DWORD,DWORD,DWORD64,bool,DWORD,bool)\0"
    "dwPID,sFile,dwExitCode,dwEP,bFound\0"
    "OnPID(DWORD,std::wstring,DWORD,DWORD64,bool)\0"
    "sFuncName,sModName,dwOffset,dwExceptionCode,dwPID,dwTID\0"
    "OnException(std::wstring,std::wstring,DWORD64,DWORD64,DWORD,DWORD)\0"
    "sMessage,dwPID\0OnDbgString(std::wstring,DWORD)\0"
    "sDLLPath,dwPID,dwEP,bLoaded\0"
    "OnDll(std::wstring,DWORD,DWORD64,bool)\0"
    "dwStackAddr,dwReturnTo,sReturnToFunc,sModuleName,dwEIP,sFuncName,sFunc"
    "Module,sSourceFilePath,iSourceLineNum\0"
    "OnCallStack(DWORD64,DWORD64,std::wstring,std::wstring,DWORD64,std::wst"
    "ring,std::wstring,std::wstring,int)\0"
};

void clsCallbacks::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        clsCallbacks *_t = static_cast<clsCallbacks *>(_o);
        switch (_id) {
        case 0: { int _r = _t->OnLog((*reinterpret_cast< tm*(*)>(_a[1])),(*reinterpret_cast< std::wstring(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 1: { int _r = _t->OnThread((*reinterpret_cast< DWORD(*)>(_a[1])),(*reinterpret_cast< DWORD(*)>(_a[2])),(*reinterpret_cast< DWORD64(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< DWORD(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 2: { int _r = _t->OnPID((*reinterpret_cast< DWORD(*)>(_a[1])),(*reinterpret_cast< std::wstring(*)>(_a[2])),(*reinterpret_cast< DWORD(*)>(_a[3])),(*reinterpret_cast< DWORD64(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 3: { int _r = _t->OnException((*reinterpret_cast< std::wstring(*)>(_a[1])),(*reinterpret_cast< std::wstring(*)>(_a[2])),(*reinterpret_cast< DWORD64(*)>(_a[3])),(*reinterpret_cast< DWORD64(*)>(_a[4])),(*reinterpret_cast< DWORD(*)>(_a[5])),(*reinterpret_cast< DWORD(*)>(_a[6])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 4: { int _r = _t->OnDbgString((*reinterpret_cast< std::wstring(*)>(_a[1])),(*reinterpret_cast< DWORD(*)>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 5: { int _r = _t->OnDll((*reinterpret_cast< std::wstring(*)>(_a[1])),(*reinterpret_cast< DWORD(*)>(_a[2])),(*reinterpret_cast< DWORD64(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        case 6: { int _r = _t->OnCallStack((*reinterpret_cast< DWORD64(*)>(_a[1])),(*reinterpret_cast< DWORD64(*)>(_a[2])),(*reinterpret_cast< std::wstring(*)>(_a[3])),(*reinterpret_cast< std::wstring(*)>(_a[4])),(*reinterpret_cast< DWORD64(*)>(_a[5])),(*reinterpret_cast< std::wstring(*)>(_a[6])),(*reinterpret_cast< std::wstring(*)>(_a[7])),(*reinterpret_cast< std::wstring(*)>(_a[8])),(*reinterpret_cast< int(*)>(_a[9])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = _r; }  break;
        default: ;
        }
    }
}

const QMetaObjectExtraData clsCallbacks::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject clsCallbacks::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_clsCallbacks,
      qt_meta_data_clsCallbacks, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &clsCallbacks::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *clsCallbacks::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *clsCallbacks::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_clsCallbacks))
        return static_cast<void*>(const_cast< clsCallbacks*>(this));
    return QObject::qt_metacast(_clname);
}

int clsCallbacks::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
