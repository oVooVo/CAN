//#include "comboeditor.h"
//#include <QHBoxLayout>
//#include "global.h"
//#include <QLineEdit>
//#include <QCompleter>
//#include "Database/SongDatabase/songdatabasesortproxy.h"


//DEFN_CREATABLE(ComboEditor, CellEditor);

//ComboEditor::ComboEditor(QWidget *parent) :
//    CellEditor(parent),
//    m_combo( new QComboBox(this) )
//{
//    setSolitaryWidget(m_combo);
//}

//void ComboEditor::polish()
//{

//    m_combo->setEditable(true);

//    // although case sensitive is marked as default, completer is case insenstive without the following.
//    m_combo->completer()->setCaseSensitivity( Qt::CaseSensitive );


//    QStringList items;
//    for (int i = 0; i < model()->rowCount(); ++i)
//    {
//        QModelIndex curIndex = model()->index( i, index().column() );
//        QString item = model()->data( curIndex, Qt::DisplayRole ).toString();
//        if (!item.isEmpty() && !items.contains(item))
//        {
//            items.append(item);
//        }
//    }
//    m_combo->addItems(items);

//    m_combo->setInsertPolicy(QComboBox::InsertAtTop);
//    m_combo->setCurrentText(currentData().toString());
//    m_combo->lineEdit()->selectAll();
//}

//QVariant ComboEditor::editedData() const
//{
//    return m_combo->currentText();
//}


