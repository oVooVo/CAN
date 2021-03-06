#ifndef CHORDPATTERNATTACHMENTVIEW_H
#define CHORDPATTERNATTACHMENTVIEW_H

#include "attachmentview.h"


namespace Ui {
class ChordPatternAttachmentView;
}

class QToolBar;
class ChordPatternEdit;
class ChordPatternAttachmentView : public AttachmentView
{
    Q_OBJECT
    DECL_CREATABLE(ChordPatternAttachmentView)

public:
    explicit ChordPatternAttachmentView(QWidget *parent = 0);
    ~ChordPatternAttachmentView();
    void updateViewIcon();

protected:
    void polish();

private:
    Ui::ChordPatternAttachmentView *ui;
    QToolBar* m_toolBar;

private slots:
    void updateText();
    void textEdited();

    void on_buttonUp_clicked();
    void on_buttonDown_clicked();
    void on_buttonView_clicked();
};

#endif // CHORDPATTERNATTACHMENTVIEW_H
