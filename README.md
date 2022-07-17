# CHeaderView

Класс представляет собой графический компонент "Иерархический заголовок".

## Описание 

Для отображения класс использует собственный тип модели CHeaderModel, но поддерживается и обратная совместимость с QAbstractTableModel.
Переопределение класса модели потребовалось для введения раздельного доступа к строкам заголовка:

`QVariant headerData(const QModelIndex &index, Qt::Orientation orientation, int role = Qt::DisplayRole) const;`

этот метод не рекомендуется переопределять поскольку в нем реализована дополнительных ролей задающих поворот текста в ячейках, а также объединение ячеек:

```
enum 
{
   // Поворот текста в ячейке заголовка
   RotationRole = Qt::UserRole, 
   // Число объединенных столбцов для данной родительской (верхней левой) ячейки
   SectionSpanRole,
   // Число объединенных строк для данной родительской ячейки
   LevelSpanRole                
};
```
 
 Для формирования атрибута заголовка предназначен метод headerDataInternal переопределяемый в наследниках CHeaderModel:

`virtual QVariant headerDataInternal(const QModelIndex &index, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const = 0;`
                                
Для задания объединений ячеек в заголовке используйте метод headerSpan:

`
void headerSpan(Qt::Orientation orientation, int row, int column, int rowSpanCount, int columnSpanCount)
`

Параметры row и column задают левую верхнюю ячейку объединения, а rowSpanCount и columnSpanCount ширину и высоту объединенной области.



