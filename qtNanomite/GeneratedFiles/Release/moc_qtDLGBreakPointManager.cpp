/****************************************************************************
** Meta object code from reading C++ file 'qtDLGBreakPointManager.h'
**
** Created: Sat 19. Jan 02:12:58 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtDLGBreakPointManager.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtDLGBreakPointManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qtDLGBreakPointManager[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      24,   23,   23,   23, 0x08,
      34,   23,   23,   23, 0x08,
      58,   48,   23,   23, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_qtDLGBreakPointManager[] = {
    "qtDLGBreakPointManager\0\0OnClose()\0"
    "OnAddUpdate()\0iRow,iCol\0"
    "OnSelectedBPChanged(int,int)\0"
};

void qtDLGBreakPointManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        qtDLGBreakPointManager *_t = static_cast<qtDLGBreakPointManager *>(_o);
        switch (_id) {
        case 0: _t->OnClose(); break;
        case 1: _t->OnAddUpdate(); break;
        case 2: _t->OnSelectedBPChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData qtDLGBreakPointManager::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject qtDLGBreakPointManager::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_qtDLGBreakPointManager,
      qt_meta_data_qtDLGBreakPointManager, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qtDLGBreakPointManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qtDLGBreakPointManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qtDLGBreakPointManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qtDLGBreakPointManager))
        return static_cast<void*>(const_cast< qtDLGBreakPointManager*>(this));
    if (!strcmp(_clname, "Ui_qtDLGBreakPointManagerClass"))
        return static_cast< Ui_qtDLGBreakPointManagerClass*>(const_cast< qtDLGBreakPointManager*>(this));
    return QWidget::qt_metacast(_clname);
}

int qtDLGBreakPointManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
