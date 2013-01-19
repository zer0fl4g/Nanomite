/****************************************************************************
** Meta object code from reading C++ file 'qtDLGNanomite.h'
**
** Created: Sat 19. Jan 02:05:32 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtDLGNanomite.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtDLGNanomite.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qtDLGNanomite[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      40,   14,   14,   14, 0x08,
      62,   14,   14,   14, 0x08,
      82,   14,   14,   14, 0x08,
     108,   14,   14,   14, 0x08,
     133,  128,   14,   14, 0x08,
     162,   14,   14,   14, 0x08,
     181,   14,   14,   14, 0x08,
     203,   14,   14,   14, 0x08,
     225,   14,   14,   14, 0x08,
     246,   14,   14,   14, 0x08,
     269,   14,   14,   14, 0x08,
     291,   14,   14,   14, 0x08,
     315,   14,   14,   14, 0x08,
     348,   14,   14,   14, 0x08,
     381,   14,   14,   14, 0x08,
     407,   14,   14,   14, 0x08,
     431,   14,   14,   14, 0x08,
     458,   14,   14,   14, 0x08,
     489,   14,   14,   14, 0x08,
     516,   14,   14,   14, 0x08,
     550,  543,   14,   14, 0x08,
     569,  543,   14,   14, 0x08,
     588,   14,   14,   14, 0x08,
     606,   14,   14,   14, 0x08,
     637,  629,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_qtDLGNanomite[] = {
    "qtDLGNanomite\0\0action_FileOpenNewFile()\0"
    "action_FileAttachTo()\0action_FileDetach()\0"
    "action_FileTerminateGUI()\0action_DebugStart()\0"
    "iPID\0action_DebugAttachStart(int)\0"
    "action_DebugStop()\0action_DebugRestart()\0"
    "action_DebugSuspend()\0action_DebugStepIn()\0"
    "action_DebugStepOver()\0action_OptionsAbout()\0"
    "action_OptionsOptions()\0"
    "action_WindowDetailInformation()\0"
    "action_WindowBreakpointManager()\0"
    "action_WindowShowMemory()\0"
    "action_WindowShowHeap()\0"
    "action_WindowShowStrings()\0"
    "action_WindowShowDebugOutput()\0"
    "action_WindowShowHandles()\0"
    "action_WindowShowWindows()\0iValue\0"
    "OnDisAsScroll(int)\0OnStackScroll(int)\0"
    "OnDebuggerBreak()\0OnDebuggerTerminated()\0"
    "qAction\0GenerateMenuCallback(QAction*)\0"
};

void qtDLGNanomite::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        qtDLGNanomite *_t = static_cast<qtDLGNanomite *>(_o);
        switch (_id) {
        case 0: _t->action_FileOpenNewFile(); break;
        case 1: _t->action_FileAttachTo(); break;
        case 2: _t->action_FileDetach(); break;
        case 3: _t->action_FileTerminateGUI(); break;
        case 4: _t->action_DebugStart(); break;
        case 5: _t->action_DebugAttachStart((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->action_DebugStop(); break;
        case 7: _t->action_DebugRestart(); break;
        case 8: _t->action_DebugSuspend(); break;
        case 9: _t->action_DebugStepIn(); break;
        case 10: _t->action_DebugStepOver(); break;
        case 11: _t->action_OptionsAbout(); break;
        case 12: _t->action_OptionsOptions(); break;
        case 13: _t->action_WindowDetailInformation(); break;
        case 14: _t->action_WindowBreakpointManager(); break;
        case 15: _t->action_WindowShowMemory(); break;
        case 16: _t->action_WindowShowHeap(); break;
        case 17: _t->action_WindowShowStrings(); break;
        case 18: _t->action_WindowShowDebugOutput(); break;
        case 19: _t->action_WindowShowHandles(); break;
        case 20: _t->action_WindowShowWindows(); break;
        case 21: _t->OnDisAsScroll((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 22: _t->OnStackScroll((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->OnDebuggerBreak(); break;
        case 24: _t->OnDebuggerTerminated(); break;
        case 25: _t->GenerateMenuCallback((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData qtDLGNanomite::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject qtDLGNanomite::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_qtDLGNanomite,
      qt_meta_data_qtDLGNanomite, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qtDLGNanomite::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qtDLGNanomite::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qtDLGNanomite::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qtDLGNanomite))
        return static_cast<void*>(const_cast< qtDLGNanomite*>(this));
    if (!strcmp(_clname, "Ui_qtDLGNanomiteClass"))
        return static_cast< Ui_qtDLGNanomiteClass*>(const_cast< qtDLGNanomite*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int qtDLGNanomite::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
