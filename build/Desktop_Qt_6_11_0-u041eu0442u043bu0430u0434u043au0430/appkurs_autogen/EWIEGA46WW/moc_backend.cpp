/****************************************************************************
** Meta object code from reading C++ file 'backend.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../backend.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'backend.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN7BackendE_t {};
} // unnamed namespace

template <> constexpr inline auto Backend::qt_create_metaobjectdata<qt_meta_tag_ZN7BackendE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Backend",
        "findingsChanged",
        "",
        "hasScanRunChanged",
        "scanFiles",
        "QVariantList",
        "paths",
        "clear",
        "copyToClipboard",
        "text",
        "findings",
        "hasScanRun"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'findingsChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'hasScanRunChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'scanFiles'
        QtMocHelpers::MethodData<void(const QVariantList &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Method 'clear'
        QtMocHelpers::MethodData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'copyToClipboard'
        QtMocHelpers::MethodData<void(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'findings'
        QtMocHelpers::PropertyData<QVariantList>(10, 0x80000000 | 5, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'hasScanRun'
        QtMocHelpers::PropertyData<bool>(11, QMetaType::Bool, QMC::DefaultPropertyFlags, 1),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<Backend, qt_meta_tag_ZN7BackendE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Backend::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN7BackendE_t>.metaTypes,
    nullptr
} };

void Backend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Backend *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->findingsChanged(); break;
        case 1: _t->hasScanRunChanged(); break;
        case 2: _t->scanFiles((*reinterpret_cast<std::add_pointer_t<QVariantList>>(_a[1]))); break;
        case 3: _t->clear(); break;
        case 4: _t->copyToClipboard((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Backend::*)()>(_a, &Backend::findingsChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Backend::*)()>(_a, &Backend::hasScanRunChanged, 1))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QVariantList*>(_v) = _t->findings(); break;
        case 1: *reinterpret_cast<bool*>(_v) = _t->hasScanRun(); break;
        default: break;
        }
    }
}

const QMetaObject *Backend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Backend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN7BackendE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Backend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Backend::findingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Backend::hasScanRunChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
