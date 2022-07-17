#ifndef CHEADERVIEW_H
#define CHEADERVIEW_H

// CHeaderView  - ��������� ����������� ���������� �������� ��������� �������
// CHeaderModel - ����� �������� ������, ������������ ����������� CHeaderView
// ������ 22.01.2020

#include <QHeaderView>
#include <QVector>
#include <QMap>


class CHeaderModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    enum
    {
        RotationRole = Qt::UserRole,
        SectionSpanRole,
        LevelSpanRole
    };

    explicit CHeaderModel(QObject *parent = 0) : QAbstractTableModel(parent){}
    ~CHeaderModel();
    // ����� ���������� ����� ������� ���������
    virtual int headerCount(Qt::Orientation orientation) const = 0;
    // ����� ���������� ������ ��������� �� ��� ���������� �������
    // ����������� ��������������� ������ � �����������, ����������� headerDataInternal
    QVariant headerData(const QModelIndex &index, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    // ����� ���������� ��������� ������ ������ ���������
    QModelIndex headerIndex(Qt::Orientation orientation, int row, int column, const QModelIndex &parent = QModelIndex()) const;
    // ����� ������� ����������� ����� ���������
    void headerSpan(Qt::Orientation orientation, int row, int column, int rowSpanCount, int columnSpanCount);
    // ����� ������ ��� ����������� � ��������� � �������� �����������
    void headerClear(Qt::Orientation orientation);
protected:
    // ����� ���������� ������ ��������� �� ��� ���������� �������
    // ����� ������������ ��� ��������������� � �����������
    virtual QVariant headerDataInternal(const QModelIndex &index, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const = 0;
private:
    // ���������� ��������� ��� �������� ������ �����������
    struct Span
    {
        QModelIndex parent;
        int rowSpanCount;
        int columnSpanCount;
    };
    // ������ ������������ ������������ ������ � �� �������� (������� ����� ������ �����������)
    QMap<QModelIndex,Span> horizontalSpan, verticalSpan;
    // ����� �������� ������� � ��������� ������ � �������� ����������
    bool headerHasIndex(Qt::Orientation orientation,int row, int column, const QModelIndex &parent = QModelIndex()) const;
};


class CHeaderView: public QHeaderView
{
    Q_OBJECT

public:
    explicit CHeaderView(Qt::Orientation orientation, QWidget *parent = 0);
    void setModel(QAbstractItemModel *model);
    void setSelectionModel(QItemSelectionModel *selectionModel);
    void doItemsLayout();
    void reset();
protected:
  void initializeSections();
    QSize sectionSizeFromContents(int section) const;
    QSize cellSizeFromContents(const QModelIndex &index) const;
    void mousePressEvent(QMouseEvent *e);
    bool viewportEvent(QEvent *e);
private:
    //������ ������� ���� �����
    mutable QVector <int> levelBottom;
    // ����� ����� � ���������
    mutable int levelCount;

    QModelIndex IndexAt(const QPoint &pos) const;
    QModelIndex IndexAt(int ax, int ay) const;

    void paintSection(QPainter *painter, const QRect &rect, int col) const;
};


#endif // CHEADERVIEW_H
