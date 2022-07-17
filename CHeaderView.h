#ifndef CHEADERVIEW_H
#define CHEADERVIEW_H

// CHeaderView  - компонент реализующий функционал сложного заголовка таблицы
// CHeaderModel - класс описание модели, используемой компонентом CHeaderView
// Версия 22.01.2020

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
    // Метод возвращает число уровней заголовка
    virtual int headerCount(Qt::Orientation orientation) const = 0;
    // Метод возвращает данные заголовка по его модельному индексу
    // Запрещается переопределение метода в наследниках, используйте headerDataInternal
    QVariant headerData(const QModelIndex &index, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    // Метод возвращает модельный индекс ячейки заголовка
    QModelIndex headerIndex(Qt::Orientation orientation, int row, int column, const QModelIndex &parent = QModelIndex()) const;
    // Метод создает объединение ячеек заголовка
    void headerSpan(Qt::Orientation orientation, int row, int column, int rowSpanCount, int columnSpanCount);
    // Метод очщает все объединения в заголовке с заданной ориентацией
    void headerClear(Qt::Orientation orientation);
protected:
    // Метод возвращает данные заголовка по его модельному индексу
    // Метод предназначен для переопределения в наследниках
    virtual QVariant headerDataInternal(const QModelIndex &index, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const = 0;
private:
    // Внутренняя структура для хранения данных объединения
    struct Span
    {
        QModelIndex parent;
        int rowSpanCount;
        int columnSpanCount;
    };
    // Списки соответствия объединенной ячейки и её родителя (верхней левой ячейки объединения)
    QMap<QModelIndex,Span> horizontalSpan, verticalSpan;
    // Метод проверки наличия в заголовке ячейки с заданным положением
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
    //Нижняя граница ряда ячеек
    mutable QVector <int> levelBottom;
    // Число строк в заголовке
    mutable int levelCount;

    QModelIndex IndexAt(const QPoint &pos) const;
    QModelIndex IndexAt(int ax, int ay) const;

    void paintSection(QPainter *painter, const QRect &rect, int col) const;
};


#endif // CHEADERVIEW_H
