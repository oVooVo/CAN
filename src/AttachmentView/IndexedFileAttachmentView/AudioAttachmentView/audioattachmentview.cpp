#include "audioattachmentview.h"
#include "ui_audioattachmentview.h"
#include "Attachments/AudioAttachment/sectionsmodel.h"

const QString AudioAttachmentView::RECORD_LEFT_POSITION_ICON_PATH  = ":/icons/icons/turnleft.png";
const QString AudioAttachmentView::RECORD_RIGHT_POSITION_ICON_PATH = ":/icons/icons/turnright.png";
AudioAttachmentView* AudioAttachmentView::m_playingAudioAttachmentView = nullptr;

DEFN_CREATABLE( AudioAttachmentView, AttachmentView );

AudioAttachmentView::AudioAttachmentView(QWidget* parent) :
    IndexedFileAttachmentView(parent),
    ui(new Ui::AudioAttachmentView)
{
    m_audioWidget = new QWidget(this);
    ui->setupUi( m_audioWidget );
    setWidget( m_audioWidget );

    connect( ui->doubleSpinBoxTempo,        SIGNAL(valueChanged(double)),   this, SLOT(setPitchTempo()));
    connect( ui->doubleSpinBoxPitch,        SIGNAL(valueChanged(double)),   this, SLOT(setPitchTempo()));
    connect( ui->pushButtonPlayPause,       SIGNAL(clicked(bool)),          this, SLOT(on_pushButtonPlayPause_toggled(bool))        );
    connect( ui->pushButtonStop,            SIGNAL(clicked()),              this, SLOT(on_pushButtonStop_clicked())                 );
    connect( ui->slider,                    SIGNAL(valueChanged(double)),   this, SLOT(seek(double)) );
    connect( ui->pushButtonClearSection,    SIGNAL(clicked()),              this, SLOT(abortSection()) );
    connect( ui->pushButtonRecordSection,   SIGNAL(clicked()),              this, SLOT(recordSection()) );

    ui->pushButtonRecordSection->setIcon( QIcon(RECORD_LEFT_POSITION_ICON_PATH) );
    ui->pushButtonClearSection->setIcon( QIcon(":/icons/icons/cross56.png") );

    ui->sectionView->setContextMenuPolicy( Qt::ActionsContextMenu );
    ui->sectionView->horizontalHeader()->hide();
    ui->sectionView->verticalHeader()->hide();

    QAction* deleteSectionAction = new QAction( ui->sectionView );
    deleteSectionAction->setText(tr("Delete Section"));
    deleteSectionAction->setIcon(QIcon(":/icons/icons/rubbish7.png"));
    deleteSectionAction->setShortcut( QKeySequence("Del") );
    deleteSectionAction->setShortcutContext(Qt::WidgetShortcut);
    ui->sectionView->addAction( deleteSectionAction );
    connect( deleteSectionAction, SIGNAL(triggered()), this, SLOT(deleteCurrentSection()) );

    QAction* restoreSectionAction = new QAction( ui->sectionView );
    restoreSectionAction->setText(tr("Restore Section"));
    restoreSectionAction->setIcon(QIcon(":/icons/icons/download73.png"));
    restoreSectionAction->setShortcutContext(Qt::WidgetShortcut);
    ui->sectionView->addAction( restoreSectionAction );
    connect( restoreSectionAction, SIGNAL(triggered()), this, SLOT(restoreCurrentSection()) );

    connect( ui->sectionView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(restoreCurrentSection()) );
    connect( ui->pushButtonRestoreSection, SIGNAL(clicked()), this, SLOT(restoreCurrentSection()) );
    connect( ui->pushButtonDeleteSection, SIGNAL(clicked()), this, SLOT(deleteCurrentSection()) );
}

AudioAttachmentView::~AudioAttachmentView()
{
    delete ui;
}

void AudioAttachmentView::polish()
{
#ifdef HAVE_SOUNDTOUCH
    IndexedFileAttachmentView::polish();

    AudioAttachment* a = attachment<AudioAttachment>();

    connect( a, &AudioAttachment::hashChanged, [this, a]()
    {
        ui->slider->setMaximum( a->player().duration() );
        ui->slider->stop();
    });

    connect( a, SIGNAL(currentSectionChanged(const Section*)), ui->slider, SLOT(setSection(const Section*)));

    open();

    ui->slider->setMaximum( a->player().duration() );
    ui->doubleSpinBoxPitch->setValue( player().pitch() );
    ui->doubleSpinBoxTempo->setValue( player().tempo() );

    ui->sectionView->setModel( a->sectionsModel() );

#endif
}

void AudioAttachmentView::on_pushButtonStop_clicked()
{
#ifdef HAVE_SOUNDTOUCH
    player().stop();
    ui->pushButtonPlayPause->setChecked( false );
    ui->slider->stop();
#endif
}

void AudioAttachmentView::on_pushButtonPlayPause_toggled(bool checked)
{
#ifdef HAVE_SOUNDTOUCH
    if (checked)
    {
        setPlayingAudioAttachmentView();
        player().play();
        ui->slider->play();
    }
    else
    {
        player().pause();
        ui->slider->pause();
    }
#else
    Q_UNUSED( checked );
#endif
}

void AudioAttachmentView::seek(double pos)
{
#ifdef HAVE_SOUNDTOUCH
    player().seek( ui->doubleSpinBoxTempo->value() * pos );
#else
    Q_UNUSED( pos );
#endif
}

void AudioAttachmentView::setPitchTempo()
{
#ifdef HAVE_SOUNDTOUCH
    player().seek( ui->doubleSpinBoxPitch->value(), ui->doubleSpinBoxTempo->value(), ui->slider->value() );
    ui->slider->setTempo( ui->doubleSpinBoxTempo->value() );
#endif
}


void AudioAttachmentView::recordSection(bool abort)
{
#ifdef HAVE_SOUNDTOUCH
    enum State { Idle, LeftRecorded };
    static State state;
    static double leftPos = -1;

    if (abort)
    {
        state = Idle;
        ui->pushButtonRecordSection->setIcon( QIcon(RECORD_LEFT_POSITION_ICON_PATH) );
        ui->slider->clearIndicators();
        attachment<AudioAttachment>()->setSection( nullptr );
    }
    else
    {
        double pos = ui->slider->value();
        if (state == Idle)
        {
            ui->slider->clearIndicators();
            leftPos = pos;
            ui->slider->setLeftIndicator( leftPos );
            attachment<AudioAttachment>()->setSection( nullptr );

            state = LeftRecorded;
            ui->pushButtonRecordSection->setIcon( QIcon(RECORD_RIGHT_POSITION_ICON_PATH) );
        }
        else
        {
            ui->slider->clearIndicators();

            Section section(tr("Unnamed"), leftPos, pos );
            attachment<AudioAttachment>()->appendSection( section );

            state = Idle;
            ui->pushButtonRecordSection->setIcon( QIcon(RECORD_LEFT_POSITION_ICON_PATH) );

            leftPos = -1;
        }
    }
#else
    Q_UNUSED( abort );
#endif
}

void AudioAttachmentView::restoreCurrentSection()
{
    const Section* section = nullptr;
    QModelIndexList indexes = ui->sectionView->selectionModel()->selectedRows();
    if (!indexes.isEmpty() && indexes.first().isValid())
    {
        section = attachment<AudioAttachment>()->sectionsModel()->section( indexes.first().row() );
    }
    attachment<AudioAttachment>()->setSection( section );
}

void AudioAttachmentView::deleteCurrentSection()
{
    QModelIndexList indexes = ui->sectionView->selectionModel()->selectedRows();
    if (!indexes.isEmpty() && indexes.first().isValid())
    {
        attachment<AudioAttachment>()->sectionsModel()->removeSection( indexes.first().row() );
    }
}

void AudioAttachmentView::open()
{
    IndexedFileAttachmentView::open();
}

void AudioAttachmentView::pause()
{
    ui->pushButtonPlayPause->setChecked(false);
    on_pushButtonPlayPause_toggled(false);
}

void AudioAttachmentView::play()
{
    ui->pushButtonPlayPause->setChecked(true);
    on_pushButtonPlayPause_toggled(true);
}

void AudioAttachmentView::chooseFile()
{
    on_pushButtonStop_clicked();
    ui->pushButtonStop->click();
    IndexedFileAttachmentView::chooseFile();
}

void AudioAttachmentView::deactivate()
{
    pause();
}

void AudioAttachmentView::setPlayingAudioAttachmentView()
{
    if (m_playingAudioAttachmentView)
    {
        m_playingAudioAttachmentView->pause();
    }
    m_playingAudioAttachmentView = this;
}

