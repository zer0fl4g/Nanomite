/****************************************************************************
** Meta object code from reading C++ file 'qtDLGOption.h'
**
** Created: Sat 19. Jan 02:04:18 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtDLGOption.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtDLGOption.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qtDLGOption[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x08,
      49,   12,   12,   12, 0x08,
      59,   12,   12,   12, 0x08,
      70,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_qtDLGOption[] = {
    "qtDLGOption\0\0OnRightClickCustomException(QPoint)\0"
    "OnClose()\0OnReload()\0OnSave()\0"
};

void qtDLGOption::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        qtDLGOption *_t = static_cast<qtDLGOption *>(_o);
        switch (_id) {
        case 0: _t->OnRightClickCustomException((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 1: _t->OnClose(); break;
        case 2: _t->OnReload(); break;
        case 3: _t->OnSave(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData qtDLGOption::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject qtDLGOption::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_qtDLGOption,
      qt_meta_data_qtDLGOption, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qtDLGOption::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qtDLGOption::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qtDLGOption::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qtDLGOption))
        return static_cast<void*>(const_cast< qtDLGOption*>(this));
    if (!strcmp(_clname, "Ui_qtDLGOptionClass"))
        return static_cast< Ui_qtDLGOptionClass*>(const_cast< qtDLGOption*>(this));
    return QDialog::qt_metacast(_clname);
}

int qtDLGOption::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
