#ifndef SPELLWORK_H
#define SPELLWORK_H

#include <QtGui/QMainWindow>
#include <QtGui/QDialog>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtCore/QThread>
#include <QtCore/QEvent>
#include <QtGui/QShortcut>

#include "ui_SpellWorkUI.h"
#include "ui_AboutUI.h"

#include "DBC/DBCStores.h"
#include "ThreadObject.h"

class SpellWork : public QMainWindow, public Ui::SpellWorkUI
{
    Q_OBJECT

        friend class ObjThread;
    public:
        SpellWork(QWidget *parent = 0);
        ~SpellWork();

        void ShowInfo();

        void AppendSkillLine();
        void AppendCastTimeLine();
        void AppendDurationLine();
        void AppendRangeInfo();
        void AppendSpellEffectInfo();
        void AppendAuraInfo(quint8 index);
        void AppendRadiusInfo(quint8 index);
        void AppendTriggerInfo(quint8 index);
        void AppendProcInfo(SpellEntry const *spellInfo);

        QString PowerString();
        QString CompareAttributes(AttrType attr, quint8 index = 0);
        QString GetDescription(QString str, SpellEntry const *spellInfo);

        void BeginThread(quint8 id);
        bool event(QEvent* e);
        bool IsFilter() const { return useFilter; }

    signals:
        void SignalSearch(bool filter);

    private slots:
        void SlotAbout();
        void SlotFilterSearch();
        void SlotButtonSearch();
        void SlotSearch(bool filter);
        void SlotSearchFromList(const QModelIndex &index);

        void SlotRegExp();

    private:
        void LoadComboBoxes();

        Ui::SpellWorkUI ui;
        SpellEntry const* m_spellInfo;

        typedef QList<ObjThread*> ThreadList;
        ThreadList threads;

        bool useRegExp;
        bool useFilter;
};

class ObjectSearch
{
    friend class SpellWork;
public:
    ObjectSearch(SpellWork *obj = NULL);
    ~ObjectSearch();

    void Search();
    bool hasValue(quint8 index, QString str);

private:
    SpellWork *iFace;
    SpellEntry const *m_spellInfo;
};

class About : public QDialog, public Ui::AboutUI
{
    Q_OBJECT

public:
    About(QWidget *parent = 0);
    ~About();

private:
    Ui::AboutUI ui;
};

class _Event : public QEvent
{
public:
    enum 
    {
        TypeId = QEvent::User + 1
    };
    _Event(quint8 id = 0, QStandardItemModel* m = NULL, SpellEntry const* s = NULL);
    ~_Event();

    quint8 GetId() const { return op_id; }
    QStandardItemModel* GetmObj() const { return mObj; }
    SpellEntry const* GetsObj() const { return sObj; }
private:
    quint8 op_id;
    QStandardItemModel* mObj;
    SpellEntry const* sObj;
};

#endif
