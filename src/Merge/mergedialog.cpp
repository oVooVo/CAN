#include "mergedialog.h"
#include "ui_mergedialog.h"
#include "Database/SongDatabase/songdatabase.h"
#include "Database/EventDatabase/eventdatabase.h"
#include "mergeitem.h"
#include "application.h"
#include <QMessageBox>
#include "mainwindow.h"

const QStringList MergeDialog::BUTTON_TEXT = QStringList({ MergeDialog::tr(""),
                                                           MergeDialog::tr("&Songs"),
                                                           MergeDialog::tr("&Events"),
                                                           MergeDialog::tr("&Ok") });

MergeDialog::MergeDialog(Merge *merge, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MergeDialog)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    setMerger(merge); // this might modify current index!
    updateButtonText();
}

MergeDialog::~MergeDialog()
{
    delete ui;
}

void MergeDialog::setMerger(Merge *merger)
{
    m_merger = merger;

    ui->songWidget->setDatabaseMerger( merger->songDatabaseMerger() );
    ui->eventWidget->setDatabaseMerger( merger->eventDatabaseMerger() );

    if (!merger->songDatabaseMerger()->hasActiveItems())
    {
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void MergeDialog::on_buttonNext_clicked()
{
    int i = ui->stackedWidget->currentIndex();
    int n = ui->stackedWidget->count();
    if (i == n-1) // we're on last page
    {
        accept();
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(i + 1);
    }

    updateButtonText();
}

void MergeDialog::on_buttonPrevious_clicked()
{
    int i = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(i - 1);
    updateButtonText();
}

void MergeDialog::on_buttonCancel_clicked()
{
    reject();
}

int MergeDialog::buttonWidth()
{
    int width = 0;
    for (const QString& text : MergeDialog::BUTTON_TEXT)
    {
        width = qMax(fontMetrics().boundingRect(text).width(), width);
    }
    return width;
}

void MergeDialog::updateButtonText()
{
    int i = ui->stackedWidget->currentIndex();
    int buttonWidth = MergeDialog::buttonWidth() + 50;
    ui->buttonPrevious->setText(BUTTON_TEXT[i]);
    ui->buttonPrevious->setFixedWidth(buttonWidth);
    ui->buttonPrevious->setEnabled(i > 0);
    ui->buttonNext->setText(BUTTON_TEXT[i + 2]);
    ui->buttonNext->setFixedWidth(buttonWidth);
    ui->buttonCancel->setFixedWidth(buttonWidth);
}

bool MergeDialog::performMerge(Project *master, Project *slave, QWidget *parent)
{
    // create merger
    Merge merge(master, slave);

    if (!merge.eventDatabaseMerger()->hasActiveItems() && !merge.songDatabaseMerger()->hasActiveItems())
    {
        QMessageBox::information(app().mainWindow(), qApp->applicationName(), tr("Nothing to merge."));
        return false;
    }
    else
    {
        // create dialog
        MergeDialog dialog(&merge, parent);

        // perform the merger
        if ( dialog.exec() == QDialog::Accepted )
        {
            QList<const void*> undeletableItems;
            merge.performMerge(undeletableItems);
            if (undeletableItems.isEmpty())
            {
                QMessageBox::information( parent,
                                          app().applicationName(),
                                          QWidget::tr("Merge successfull"),
                                          QMessageBox::Ok );
                return true;
            }
            else
            {
                QStringList warningString;
                warningString << QWidget::tr("Some Songs could not be removed since they are used.");
                warningString << QWidget::tr("Please try to remove them manually.");
                for (const void* item : undeletableItems)
                {
                    warningString << "\n" + merge.labelItem(item);
                }
                QMessageBox::warning( parent,
                                      app().applicationName(),
                                      warningString.join("\n"),
                                      QMessageBox::Ok );
                return true;
            }
        }
        else
        {
            return false;
        }
    }
}

bool MergeDialog::performMerge(Project *master, const QString &slaveFilename, QWidget* parent)
{
    // try to open the slave project
    bool success;
    Project* slave = new Project();
    OpenError error = slave->openProject(slaveFilename);
    if (error != NoError)
    {
        app().handleProjectOpenError(error, slaveFilename);
        success = false;
    }
    else
    {
        success = performMerge(master, slave, parent);
    }

    delete slave;
    slave = nullptr;

    return success;
}
