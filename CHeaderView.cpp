// CHeaderView  - компонент реализующий функционал сложного заголовка таблицы
// CHeaderModel - класс описание модели, используемой компонентом CHeaderView
// Версия 22.01.2020

// TODO реакция на изменения числа уровней и секций заголовка
// TODO отображение и поддержка индикатора сортировки

#include "CHeaderView.h"
#include <qevent.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>

// CHeaderModel - класс описание модели, используемой компонентом CHeaderView

// Деструктор. Очистка списков объединений
CHeaderModel::~CHeaderModel()
{
   horizontalSpan.clear();
   verticalSpan.clear();
}
// Метод проверки наличия в заголовке ячейки с заданным положением
bool CHeaderModel::headerHasIndex(Qt::Orientation orientation,int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0)
        return false;
    if (orientation == Qt::Horizontal)
        return row < headerCount(orientation) && column < columnCount(parent);
    else
        return row < headerCount(orientation) && column < rowCount(parent);
}
// Метод возвращает модельный индекс ячейки заголовка
QModelIndex CHeaderModel::headerIndex(Qt::Orientation orientation, int row, int column, const QModelIndex &parent) const
{
    if  (headerHasIndex(orientation, row, column, parent))
    {
        QModelIndex index = createIndex(row, column);
        if (orientation == Qt::Horizontal && horizontalSpan.contains(index))
            return horizontalSpan.value(index).parent;
        if (orientation == Qt::Vertical && verticalSpan.contains(index))
            return verticalSpan.value(index).parent;
        return index;
    }
   return QModelIndex();
}
// Метод создает объединение ячеек заголовка
void CHeaderModel::headerSpan(Qt::Orientation orientation, int row, int column, int rowSpanCount, int columnSpanCount)
{
    if (headerHasIndex(orientation, row, column) && rowSpanCount > 0 && columnSpanCount > 0)
    {
        Span item;
        item.parent          = createIndex(row, column);
        item.rowSpanCount    = qBound(1,rowSpanCount,headerCount(orientation)-row);
        item.columnSpanCount = orientation == Qt::Horizontal? qBound(1,columnSpanCount,columnCount()-column) :
                                                              qBound(1,columnSpanCount,rowCount()-column);

        for (int i=row; i<row+item.rowSpanCount; ++i)
           for (int j=column; j<column+item.columnSpanCount; ++j)
               if (headerHasIndex(orientation, i, j))
               {
                   orientation == Qt::Horizontal? horizontalSpan.insert(createIndex(i,j),item):
                                                  verticalSpan.insert(createIndex(i,j),item);
               }
    }
}
// Метод очищает все объединения в заголовке с заданной ориентацией
void CHeaderModel::headerClear(Qt::Orientation orientation)
{
    if (orientation == Qt::Horizontal)
        horizontalSpan.clear();
    else
    if (orientation == Qt::Vertical)
        verticalSpan.clear();
}
// Метод возвращает данные заголовка по его модельному индексу
// Запрещается переопределение метода в наследниках, используйте headerDataInternal
QVariant CHeaderModel::headerData(const QModelIndex &index, Qt::Orientation orientation, int role) const
{
  if (index.isValid())
  {
    switch (role)
    {
        case CHeaderModel::SectionSpanRole:
        {
            if (orientation == Qt::Horizontal)
                return horizontalSpan.contains(index)? horizontalSpan.value(index).columnSpanCount : 1;
            if (orientation == Qt::Vertical)
                return verticalSpan.contains(index)? verticalSpan.value(index).columnSpanCount : 1;
            return QVariant();
        }
        case CHeaderModel::LevelSpanRole:
        {
            if (orientation == Qt::Horizontal)
                return horizontalSpan.contains(index)? horizontalSpan.value(index).rowSpanCount : 1;
            if (orientation == Qt::Vertical)
                return verticalSpan.contains(index)? verticalSpan.value(index).rowSpanCount : 1;
            return QVariant();
        }
        default:
            return headerDataInternal(index, orientation, role);
    }
  }
  return QVariant();
}



// CHeaderView  - компонент реализующий функционал сложного заголовка таблицы

// В конструкторе запрещаем перемещения ячеек
CHeaderView::CHeaderView(Qt::Orientation orientation, QWidget *parent):
    QHeaderView(orientation,parent), levelCount(0)
{
    QHeaderView::setMovable(false);
}

void CHeaderView::setModel(QAbstractItemModel *model)
{
    QHeaderView::setModel(model);
    initializeSections();
}

// При изменении выделения ячеек будет перерисовываться весь заголовок
// (сделано для корректного отображения объединенных ячеек)
void CHeaderView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    if (this->selectionModel()) {
        disconnect(this->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                   viewport(), SLOT(update()));
    }

    QHeaderView::setSelectionModel(selectionModel);

    if (selectionModel) {
        connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                viewport(), SLOT(update()));
    }
}

void CHeaderView::doItemsLayout()
{
    initializeSections();
    QHeaderView::doItemsLayout();
}

void CHeaderView::reset()
{
    initializeSections();
    QHeaderView::reset();
}


// Инициализация вспомогательного массива высот ячеек (учитываются объединения)
// и вызов метода предка QHeaderView::initializeSections()
void CHeaderView::initializeSections()
{
    CHeaderModel *cmodel = dynamic_cast<CHeaderModel *>(model());
    if (cmodel)
    {
        levelCount = cmodel->headerCount(orientation());

        int size,bottom = 0;
        levelBottom.clear();

        for (int row=0; row<levelCount; ++row)
        {
            size = 0;
            for (int col = 0; col < count(); ++col)
            {
                QModelIndex index = cmodel->headerIndex(orientation(),row,col);

                //заполнения массива нижних границ ряда ячеек
                QSize hint    = cellSizeFromContents(index);
                QVariant span = cmodel->headerData(index, orientation(), CHeaderModel::LevelSpanRole);
                int cellspan  = span.canConvert<uint>()? qBound(1,span.value<int>(),levelCount-index.row()) : 1;
                size = (orientation() == Qt::Horizontal)? qMax(hint.height()/cellspan,size) : qMax(hint.width()/cellspan,size);
            }
            bottom+=size;
            levelBottom.push_back(bottom);

        }
    }

    QHeaderView::initializeSections();
}
// Метод определения размеров ячейки сложного заголовка
// Входной параметр - модельный индекс ячейки сложного заголовка
QSize CHeaderView::cellSizeFromContents(const QModelIndex &index) const
{



    CHeaderModel *cmodel = dynamic_cast<CHeaderModel *>(model());

    if (!index.isValid() || !cmodel)
        return QSize();

    ensurePolished();

    // use SizeHintRole
    QVariant variant = cmodel->headerData(index, orientation(), Qt::SizeHintRole);
    if (variant.isValid())
        return qvariant_cast<QSize>(variant);

    // otherwise use the contents
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    opt.section = index.column();
    QVariant var = cmodel->headerData(index, orientation(),
                                            Qt::FontRole);
    QFont fnt;
    if (var.isValid() && var.canConvert<QFont>())
        fnt = qvariant_cast<QFont>(var);
    else
        fnt = font();
    fnt.setBold(true);
    opt.fontMetrics = QFontMetrics(fnt);
    opt.text = cmodel->headerData(index, orientation(),
                                    Qt::DisplayRole).toString();
    variant = cmodel->headerData(index, orientation(), Qt::DecorationRole);
    opt.icon = qvariant_cast<QIcon>(variant);
    if (opt.icon.isNull())
        opt.icon = qvariant_cast<QPixmap>(variant);

    QSize size = style()->sizeFromContents(QStyle::CT_HeaderSection, &opt, QSize(), this);
    int SortIndicatorSize = 0;
    if (isSortIndicatorShown()) {
        int margin = style()->pixelMetric(QStyle::PM_HeaderMargin, &opt, this);
        if (orientation() == Qt::Horizontal)
            SortIndicatorSize = size.height() + margin;
        else
            SortIndicatorSize = size.width() + margin;
    }
    // Учитывается размер индикатора сортировки
    if (cmodel->headerData(index, orientation(), CHeaderModel::RotationRole).toBool())
        size.transpose();

    if (orientation() == Qt::Horizontal)
        size.rwidth() += SortIndicatorSize;
    else
        size.rheight() += SortIndicatorSize;

     return size;
}

// Переопределенный метод определения размеров секции сложного заголовка
// Входной параметр - индекс секции (строка) сложного заголовка
QSize CHeaderView::sectionSizeFromContents(int section) const
{
    CHeaderModel *cmodel = dynamic_cast<CHeaderModel *>(model());
    if (!cmodel)
        return QHeaderView::sectionSizeFromContents(section);

    QSize sectionSizeHint = QSize(0,0);

    if (isSectionHidden(section))
      return sectionSizeHint;

    for (int row = 0; row < levelCount; ++row)
    {

        QModelIndex index = cmodel->headerIndex(orientation(),row,section);
        QSize cellSize = cellSizeFromContents(index);
        QVariant span = cmodel->headerData(index, orientation(), CHeaderModel::SectionSpanRole);
        if (span.canConvert<uint>())
        {
           if (orientation()==Qt::Horizontal)
               cellSize.rwidth() /= qBound(1,span.value<int>(),count()-index.column());
           else
               cellSize.rheight()/= qBound(1,span.value<int>(),count()-index.column());
        }

        span = cmodel->headerData(index, orientation(), CHeaderModel::LevelSpanRole);
        if (span.canConvert<uint>())
        {
            if (orientation()==Qt::Horizontal)
                cellSize.rheight()/= qBound(1,span.value<int>(),levelCount-index.row());
            else
                cellSize.rwidth() /= qBound(1,span.value<int>(),levelCount-index.row());
        }

        if (orientation()==Qt::Horizontal)
        {
           // Для горизонтального заголовка складываем высоты и ищем максимальную ширину
            sectionSizeHint.rheight() += cellSize.height();
            sectionSizeHint.setWidth(qMax(sectionSizeHint.width(), cellSize.width()));
         }
         else
         {
            // Для вертикального заголовка наоборот
            sectionSizeHint.rwidth() += cellSize.width();
            sectionSizeHint.setHeight(qMax(sectionSizeHint.height(), cellSize.height()));
          }
    }
    return sectionSizeHint;
}
// Метод возвращает модельный индекс ячейки по координатам курсора
QModelIndex CHeaderView::IndexAt(const QPoint &pos) const
{
   return IndexAt(pos.x(), pos.y());
}
// Метод возвращает модельный индекс ячейки по координатам курсора
QModelIndex CHeaderView::IndexAt(int ax, int ay) const
{
    int pos = orientation()==Qt::Horizontal? ay : ax,
        col = orientation()==Qt::Horizontal? logicalIndexAt(ax): logicalIndexAt(ay);

    if(col >=0)
    {
        for (int row = 0; row < levelCount; ++row)
        {
            if (pos < levelBottom.at(row))
            {
                CHeaderModel *cmodel = dynamic_cast<CHeaderModel *>(model());

                return !cmodel ? QModelIndex() : cmodel->headerIndex(orientation(),row, col);
            }
        }
    }
  return QModelIndex();
}
// Метод осуществляющий рисование столбца из нескольких ячеек
// (выполнено для совместимости с реализацией метода предка)
void CHeaderView::paintSection(QPainter *painter, const QRect &rect, int col) const
{
    CHeaderModel *cmodel = dynamic_cast<CHeaderModel *>(model());

    if (!cmodel)
        return QHeaderView::paintSection(painter,rect,col);

        for (int row = 0; row < levelCount; ++row)
        {
            QModelIndex index = cmodel->headerIndex(orientation(),row,col);

            if (!index.isValid()) continue;
            painter->save();

            // get the state of the section
            QStyleOptionHeader opt;
            initStyleOption(&opt);

            QVariant font = cmodel->headerData(index, orientation(), Qt::FontRole);
            if (font.isValid() && font.canConvert<QFont>()) {
                QFont sectionFont = qvariant_cast<QFont>(font);
                painter->setFont(sectionFont);
            }

            // setup the style options structure
            QVariant textAlignment = cmodel->headerData(index, orientation(),
                                                          Qt::TextAlignmentRole);

            int lastSection = index.column(),
                lastLevel   = index.row();

            QVariant span = cmodel->headerData(index, orientation(), CHeaderModel::SectionSpanRole);
            if (span.canConvert<uint>())
                lastSection = index.column()+qBound(1,span.value<int>(),count()-index.column())-1;

            span = cmodel->headerData(index, orientation(), CHeaderModel::LevelSpanRole);
            if (span.canConvert<uint>())
                 lastLevel  = index.row()+qBound(1,span.value<int>(),levelCount-index.row())-1;

            int left   = sectionViewportPosition(index.column()),
                top    = index.row()==0? 0: levelBottom.at(index.row()-1),
                width  = sectionViewportPosition(lastSection)+sectionSize(lastSection)-left,
                height = levelBottom.at(lastLevel)-top;

            opt.rect = orientation() == Qt::Horizontal? QRect(left, top, width, height):
                                                        QRect(top, left, height, width);
            opt.section = col;

            opt.textAlignment = Qt::Alignment(textAlignment.isValid()
                                              ? Qt::Alignment(textAlignment.toInt())
                                              : Qt::AlignCenter);

            opt.iconAlignment = Qt::AlignVCenter;
            opt.text = cmodel->headerData(index, orientation(),
                                            Qt::DisplayRole).toString();

            QVariant variant = cmodel->headerData(index, orientation(),
                                            Qt::DecorationRole);
            opt.icon = qvariant_cast<QIcon>(variant);
            if (opt.icon.isNull())
                opt.icon = qvariant_cast<QPixmap>(variant);
            QVariant foregroundBrush = cmodel->headerData(index, orientation(),
                                                            Qt::ForegroundRole);
            if (foregroundBrush.canConvert<QBrush>())
                opt.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(foregroundBrush));

            QPointF oldBO = painter->brushOrigin();
            QVariant backgroundBrush = cmodel->headerData(index, orientation(),
                                                            Qt::BackgroundRole);
            if (backgroundBrush.canConvert<QBrush>()) {
                opt.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(backgroundBrush));
                opt.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(backgroundBrush));
                painter->setBrushOrigin(opt.rect.topLeft());
            }

            // the section position
            opt.position = QStyleOptionHeader::Middle;
            opt.orientation = orientation();


            if (isEnabled())
                opt.state |= QStyle::State_Enabled;
            if (window()->isActiveWindow())
                opt.state |= QStyle::State_Active;

            opt.selectedPosition = QStyleOptionHeader::NotAdjacent;

            // the selected position
            if(selectionModel())
            {
               if ((orientation() == Qt::Horizontal && selectionModel()->isColumnSelected(index.column(), rootIndex())
                                                    && selectionModel()->isColumnSelected(lastSection, rootIndex())) ||
                   (orientation() == Qt::Vertical   && selectionModel()->isRowSelected(index.column(), rootIndex())
                                                    && selectionModel()->isRowSelected(lastSection, rootIndex())))
                  opt.state |= QStyle::State_Sunken | QStyle::State_On;
            }

            style()->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);

            if (cmodel->headerData(index, orientation(), CHeaderModel::RotationRole).toBool())
            {
                painter->translate(opt.rect.left(), opt.rect.top() + opt.rect.height());
                painter->rotate(-90);
                opt.rect.setRect(0, 0, opt.rect.height(), opt.rect.width());
            }

            opt.text = opt.fontMetrics.elidedText(opt.text, Qt::ElideRight , opt.rect.width() - 4);


            // draw the section
            style()->drawControl(QStyle::CE_HeaderLabel, &opt, painter, this);

            painter->setBrushOrigin(oldBO);
            painter->restore();

        }
}
// Обработчик клика по ячейке (переопределен для корректного выделения строк/столбцов под объединенной ячейкой)
void CHeaderView::mousePressEvent(QMouseEvent *e)
{
    QHeaderView::mousePressEvent(e);

    CHeaderModel *cmodel = dynamic_cast<CHeaderModel *>(model());
    if (cmodel != NULL)
    {
        QModelIndex index = IndexAt(e->pos());
        if (index.isValid())
        {
            if ( e->button() == Qt::LeftButton && selectionModel())
            {
                int firstSection = index.column();
                int lastSection  = firstSection;
                QVariant span = cmodel->headerData(index, orientation(), CHeaderModel::SectionSpanRole);
                if (span.canConvert<uint>())
                    lastSection = index.column()+qBound(1,span.value<int>(),count()-index.column())-1;

                QModelIndex index1 = orientation()==Qt::Horizontal? cmodel->index(0,firstSection,rootIndex()):
                                                                    cmodel->index(firstSection,0,rootIndex()),
                        index2 = orientation()==Qt::Horizontal? cmodel->index(model()->rowCount()-1,lastSection,rootIndex()):
                                                                cmodel->index(lastSection,model()->columnCount()-1,rootIndex());
                selectionModel()->select(QItemSelection(index1, index2), QItemSelectionModel::ClearAndSelect);
            }
        }
    }
}

// Метод обработки событий viewport. Переопределен для совместимости с
// методом IndexAt(), возвращающим модельный индекс ячейки
bool CHeaderView::viewportEvent(QEvent *e)
{
    CHeaderModel *cmodel = dynamic_cast<CHeaderModel*>(model());

    if (!cmodel)
        return QHeaderView::viewportEvent(e);

        switch (e->type())
        {
    #ifndef QT_NO_TOOLTIP
        case QEvent::ToolTip: {
            QHelpEvent *he = static_cast<QHelpEvent*>(e);
            QModelIndex index = IndexAt(he->pos());
            if (index.isValid()) {
                QVariant variant = cmodel->headerData(index, orientation(), Qt::ToolTipRole);
                if (variant.isValid()) {
                    QToolTip::showText(he->globalPos(), variant.toString(), this);
                    return true;
                }
            }
            break; }
    #endif
    #ifndef QT_NO_WHATSTHIS
        case QEvent::QueryWhatsThis: {
            QHelpEvent *he = static_cast<QHelpEvent*>(e);
            QModelIndex index = IndexAt(he->pos());
            if (index.isValid()
                && cmodel->headerData(index, orientation(), Qt::WhatsThisRole).isValid())
                return true;
            break; }
        case QEvent::WhatsThis: {
            QHelpEvent *he = static_cast<QHelpEvent*>(e);
            QModelIndex index = IndexAt(he->pos());
            if (index.isValid()) {
                 QVariant whatsthis = cmodel->headerData(index, orientation(),
                                                          Qt::WhatsThisRole);
                 if (whatsthis.isValid()) {
                     QWhatsThis::showText(he->globalPos(), whatsthis.toString(), this);
                     return true;
                 }
            }
            break; }
    #endif // QT_NO_WHATSTHIS
    #ifndef QT_NO_STATUSTIP
        case QEvent::StatusTip: {
            QHelpEvent *he = static_cast<QHelpEvent*>(e);
            QModelIndex index = IndexAt(he->pos());
            if (index.isValid()) {
                QString statustip = cmodel->headerData(index, orientation(),
                                                        Qt::StatusTipRole).toString();
                if (!statustip.isEmpty())
                    setStatusTip(statustip);
            }
            return true; }
    #endif // QT_NO_STATUSTIP
        default:
            return QHeaderView::viewportEvent(e);
        }
        return QHeaderView::viewportEvent(e);
}



