/****************************************************************************
** Meta object code from reading C++ file 'qtDLGWindowView.h'
**
** Created: Sat 19. Jan 02:04:18 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtDLGWindowView.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qtDLGWindowView.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_qtDLGWindowView[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_qtDLGWindowView[] = {
    "qtDLGWindowView\0"
};

void qtDLGWindowView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData qtDLGWindowView::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject qtDLGWindowView::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_qtDLGWindowView,
      qt_meta_data_qtDLGWindowView, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &qtDLGWindowView::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *qtDLGWindowView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *qtDLGWindowView::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_qtDLGWindowView))
        return static_cast<void*>(const_cast< qtDLGWindowView*>(this));
    if (!strcmp(_clname, "Ui_qtDLGWindowViewClass"))
        return static_cast< Ui_qtDLGWindowViewClass*>(const_cast< qtDLGWindowView*>(this));
    return QWidget::qt_metacast(_clname);
}

int qtDLGWindowView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
