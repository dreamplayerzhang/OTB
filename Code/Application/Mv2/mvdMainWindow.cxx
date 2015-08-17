/*=========================================================================

  Program:   Monteverdi2
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mvdMainWindow.h"
#include "ui_mvdMainWindow.h"


/*****************************************************************************/
/* INCLUDE SECTION                                                           */

//
// Qt includes (sorted by alphabetic order)
//// Must be included before system/custom includes.
#include <QtGui>

//
// System includes (sorted by alphabetic order)

//
// ITK includes (sorted by alphabetic order)

//
// OTB includes (sorted by alphabetic order)

//
// Monteverdi includes (sorted by alphabetic order)
#if defined( OTB_USE_QT4 ) && USE_OTB_APPS
# include "ApplicationsWrapper/mvdApplicationLauncher.h"
# include "ApplicationsWrapper/mvdApplicationsToolBoxController.h"
# include "ApplicationsWrapper/mvdOTBApplicationsModel.h"
# include "ApplicationsWrapper/mvdWrapperQtWidgetView.h"
#endif
//
// #include "Core/mvdDatabaseModel.h"
// #include "Core/mvdDatasetModel.h"
#include "Core/mvdQuicklookModel.h"
#include "Core/mvdVectorImageModel.h"
//
#include "Gui/mvdApplicationsToolBox.h"
#include "Gui/mvdColorSetupController.h"
#include "Gui/mvdColorSetupWidget.h"
#include "Gui/mvdColorDynamicsController.h"
#include "Gui/mvdColorDynamicsWidget.h"
// #include "Gui/mvdDatabaseBrowserController.h"
#if ENABLE_TREE_WIDGET_TEST
// #include "Gui/mvdDatabaseBrowserWidgetTest.h"
#else // ENABLE_TREE_WIDGET_TEST
// #include "Gui/mvdDatabaseBrowserWidget.h"
#endif // ENABLE_TREE_WIDGET_TEST
// #include "Gui/mvdDatasetPropertiesController.h"
// #include "Gui/mvdDatasetPropertiesWidget.h"
#include "Gui/mvdFilenameDragAndDropEventFilter.h"
#include "Gui/mvdHistogramController.h"
#include "Gui/mvdHistogramWidget.h"
#include "Gui/mvdKeymapDialog.h"
#include "Gui/mvdImageViewManipulator.h"
#include "Gui/mvdImageViewRenderer.h"
#include "Gui/mvdImageViewWidget.h"
#include "Gui/mvdLayerStackController.h"
#include "Gui/mvdLayerStackWidget.h"
#if USE_PIXEL_DESCRIPTION
#  include "Gui/mvdPixelDescriptionWidget.h"
#endif // USE_PIXEL_DESCRIPTION
#include "Gui/mvdQuicklookViewManipulator.h"
#include "Gui/mvdQuicklookViewRenderer.h"
#include "Gui/mvdShaderWidget.h"
#include "Gui/mvdStatusBarWidget.h"
//
#include "mvdApplication.h"
#include "mvdPreferencesDialog.h"

namespace mvd
{

/*
  TRANSLATOR mvd::MainWindow

  Necessary for lupdate to be aware of C++ namespaces.

  Context comment for translator.
*/


/*****************************************************************************/
/* CONSTANTS                                                                 */

#define REFERENCE_LAYER_COMBOBOX_NAME "referenceLayerComboBox"

/*****************************************************************************/
/* STATIC IMPLEMENTATION SECTION                                             */


/*****************************************************************************/
/* CLASS IMPLEMENTATION SECTION                                              */

/*****************************************************************************/
MainWindow
::MainWindow( QWidget* parent, Qt::WindowFlags flags ) :
  I18nMainWindow( parent, flags ),
  m_UI( new mvd::Ui::MainWindow() ),
  m_ColorDynamicsDock( NULL ),
  m_ColorSetupDock( NULL ),
  // m_DatabaseBrowserDock( NULL ),
  // m_DatasetPropertiesDock(NULL),
  m_LayerStackDock( NULL ),
#if USE_PIXEL_DESCRIPTION
  m_PixelDescriptionDock(NULL),
#endif // USE_PIXEL_DESCRIPTION
  m_HistogramDock( NULL ),
#if defined( OTB_USE_QT4 ) && USE_OTB_APPS
  m_OtbApplicationsBrowserDock(NULL),
#endif
  m_ImageView( NULL ),
  m_QuicklookViewDock( NULL ),
#if USE_TABBED_VIEW
  m_CentralTabWidget( NULL ),
#endif // USE_TABBED_VIEW
  m_StatusBarWidget( NULL ),
  m_ShaderWidget( NULL ),
  m_FilenameDragAndDropEventFilter( NULL )
{
  m_UI->setupUi( this );

  //
  // Event filters.
  m_FilenameDragAndDropEventFilter = new FilenameDragAndDropEventFilter( this );

  QObject::connect(
    m_FilenameDragAndDropEventFilter,
    SIGNAL( FilenameDropped( const QString & ) ),
    // to:
    this,
    SLOT( ImportImage( const QString & ) )
  );
}

/*****************************************************************************/
MainWindow
::~MainWindow()
{
  delete m_UI;
  m_UI = NULL;
}

/*****************************************************************************/
bool
MainWindow
::CheckGLCapabilities() const
{
  assert( m_ImageView!=NULL );
  assert( m_ImageView->GetRenderer()!=NULL );

  if( m_ImageView==NULL ||
      m_ImageView->GetRenderer()==NULL )
    return false;

  return m_ImageView->GetRenderer()->CheckGLCapabilities();
}

/*****************************************************************************/
void
MainWindow
::virtual_SetupUI()
{
  setObjectName( "mvd::MainWindow" );
  setWindowTitle( PROJECT_NAME );

  InitializeCentralWidget();

  InitializeDockWidgets();

  InitializeStatusBarWidgets();

  InitializeRenderToolBar();
  InitializeShaderToolBar();

  assert( m_UI!=NULL );
}

/*****************************************************************************/
void
MainWindow
::virtual_ConnectUI()
{
  ConnectViewMenu();

  //
  // CHAIN CONTROLLERS.
  // Forward model update signals of color-setup controller...
  QObject::connect(
    m_ColorSetupDock->findChild< AbstractModelController* >(),
    SIGNAL( RgbChannelIndexChanged( RgbwChannel, int ) ),
    // to: ...color-dynamics controller model update signal.
    m_ColorDynamicsDock->findChild< AbstractModelController* >(),
    SLOT( OnRgbChannelIndexChanged( RgbwChannel, int ) )
  );
  QObject::connect(
    m_ColorSetupDock->findChild< AbstractModelController* >(),
    SIGNAL( GrayChannelIndexChanged( int ) ),
    // to: ...color-dynamics controller model update signal.
    m_ColorDynamicsDock->findChild< AbstractModelController* >(),
    SLOT( OnGrayChannelIndexChanged( int ) )
  );
  QObject::connect(
    m_ColorSetupDock->findChild< AbstractModelController* >(),
    SIGNAL( GrayscaleActivated( bool ) ),
    // to: ...color-dynamics controller model update signal.
    m_ColorDynamicsDock->findChild< AbstractModelController* >(),
    SLOT( OnGrayscaleActivated( bool ) )
  );

  QObject::connect(
    m_ColorSetupDock->findChild< AbstractModelController* >(),
    SIGNAL( RgbChannelIndexChanged( RgbwChannel, int ) ),
    // to: ...color-dynamics controller model update signal.
    m_HistogramDock->findChild< AbstractModelController* >(),
    SLOT( OnRgbChannelIndexChanged( RgbwChannel, int ) )
  );
  QObject::connect(
    m_ColorSetupDock->findChild< AbstractModelController* >(),
    SIGNAL( GrayChannelIndexChanged( int ) ),
    // to: ...color-dynamics controller model update signal.
    m_HistogramDock->findChild< AbstractModelController* >(),
    SLOT( OnGrayChannelIndexChanged( int ) )
  );
  QObject::connect(
    m_ColorSetupDock->findChild< AbstractModelController* >(),
    SIGNAL( GrayscaleActivated( bool ) ),
    // to: ...color-dynamics controller model update signal.
    m_HistogramDock->findChild< AbstractModelController* >(),
    SLOT( OnGrayscaleActivated( bool ) )
  );

  QObject::connect(
    m_ColorDynamicsDock->findChild< AbstractModelController* >(),
    SIGNAL( LowIntensityChanged( RgbwChannel, double, bool ) ),
    // to: ...histogram controller model update signal.
    m_HistogramDock->findChild< AbstractModelController* >(),
    SLOT( OnLowIntensityChanged( RgbwChannel, double, bool ) )
  );
  QObject::connect(
    m_ColorDynamicsDock->findChild< AbstractModelController* >(),
    SIGNAL( HighIntensityChanged( RgbwChannel, double, bool ) ),
    // to: ...histogram controller model update signal.
    m_HistogramDock->findChild< AbstractModelController* >(),
    SLOT( OnHighIntensityChanged( RgbwChannel, double, bool ) )
  );
  QObject::connect(
    m_ColorDynamicsDock->findChild< AbstractModelController* >(),
    SIGNAL( HistogramRefreshed() ),
    // to: ...histogram controller model update signal.
    m_HistogramDock->findChild< AbstractModelController* >(),
    SLOT( OnHistogramRefreshed() )
  );

  //
  // OTB application support.
#if defined( OTB_USE_QT4 ) && USE_OTB_APPS
  //
  // Done here cause needed to be done once and only once.
  SetControllerModel(
    m_OtbApplicationsBrowserDock,
    Application::Instance()->GetOTBApplicationsModel()
  );

  //
  // need to get the ApplicationToolBox widget to setup connections.
  // a double click on the tree widget should trigger a signal connected
  // to this MainWindow slot. this slot will be in charge of getting the
  // widget of the application selected, and show it in the
  // MainWindow centralView.

  // # Step 1 : get the ApplicationToolBoxWidget
  ApplicationsToolBox* appWidget =
    qobject_cast< ApplicationsToolBox * >(
      m_OtbApplicationsBrowserDock->findChild< ApplicationsToolBox* >()
    );
  assert( appWidget!=NULL );

  // # Step 2 : setup connections
  QObject::connect(
    appWidget,
    SIGNAL( ApplicationToLaunchSelected(const QString &, const QString &) ),
    this,
    SLOT( OnApplicationToLaunchSelected(const QString &, const QString &) )
  );

  // # Step 3 : connect close slots
#endif

  //
  // close tabs handling
#if USE_TABBED_VIEW
  QObject::connect(
    m_CentralTabWidget,
    SIGNAL( tabCloseRequested( int ) ),
    this,
    SLOT( OnTabCloseRequested( int ) )
  );
#endif // USE_TABBED_VIEW

  //
  // Other connections.

  ConnectImageViews();

  ConnectStatusBar();
}

/*****************************************************************************/
void
MainWindow
::ConnectImageViews()
{
  assert( m_ImageView!=NULL );

  QObject::connect(
    this,
    SIGNAL( UserZoomIn() ),
    // to:
    m_ImageView,
    SLOT( ZoomIn() )
  );

  QObject::connect(
    this,
    SIGNAL( UserZoomOut() ),
    // to:
    m_ImageView,
    SLOT( ZoomOut() )
  );

  QObject::connect(
    this,
    SIGNAL( UserZoomExtent() ),
    m_ImageView,
    SLOT( ZoomToExtent() )
  );

  QObject::connect(
    this,
    SIGNAL( UserZoomFull() ),
    m_ImageView,
    SLOT( ZoomToFullResolution() )
  );

  QObject::connect(
    this,
    SIGNAL( UserZoomLayer() ),
    m_ImageView,
    SLOT( ZoomToLayerExtent() )
  );

  //
  // Connect image-views for ROI-changed events.

  const AbstractImageViewManipulator* imageViewManipulator =
    m_ImageView->GetManipulator();

  assert( imageViewManipulator!=NULL );


  ImageViewWidget* quicklookView = GetQuicklookView();
  assert( quicklookView!=NULL );

  const AbstractImageViewManipulator* quicklookManipulator =
    quicklookView->GetManipulator();

  assert( quicklookManipulator!=NULL );


  QObject::connect(
    m_ImageView,
    SIGNAL( RoiChanged(
        const PointType &, const SizeType &, const SpacingType &, const PointType & )
    ),
    // to:
    quicklookManipulator,
    SLOT( OnRoiChanged(
        const PointType &, const SizeType &, const SpacingType &, const PointType & )
    )
  );

  QObject::connect(
    quicklookView,
    SIGNAL( CenterRoiRequested( const PointType& ) ),
    // to:
    imageViewManipulator,
    SLOT( CenterOn( const PointType& ) )
  );

  // Not needed anymore becaure already done in ImageViewWidget.
  // QObject::connect(
  //   quicklookManipulator,
  //   SIGNAL( RefreshViewRequested() ),
  //   // to:
  //   m_ImageView,
  //   SLOT( updateGL() )
  // );

  //
  // Connect controllers to image-views.
  //

  {
  assert( m_LayerStackDock!=NULL );

  AbstractModelController * controller = GetController( m_LayerStackDock );
  assert( controller!=NULL );

  QObject::connect(
    controller,
    SIGNAL( ApplyAllRequested() ),
    // to:
    m_ImageView,
    SLOT( OnApplyAllRequested() )
  );

  QObject::connect(
    controller,
    SIGNAL( ApplyAllRequested() ),
    // to:
    quicklookView,
    SLOT( OnApplyAllRequested() )
  );
  }
}

/*****************************************************************************/
void
MainWindow
::ConnectViewMenu()
{
  m_UI->menu_View->addAction( m_UI->m_ToolBar->toggleViewAction() );
  m_UI->menu_View->addAction( m_UI->m_RenderToolBar->toggleViewAction() );
  m_UI->menu_View->addAction( m_UI->m_ShaderToolBar->toggleViewAction() );

  m_UI->menu_View->addSeparator();

  m_UI->menu_View->addAction( m_ColorSetupDock->toggleViewAction() );
  m_UI->menu_View->addAction( m_ColorDynamicsDock->toggleViewAction() );
  m_UI->menu_View->addAction( m_HistogramDock->toggleViewAction() );
  m_UI->menu_View->addAction( m_LayerStackDock->toggleViewAction() );
  m_UI->menu_View->addAction( m_QuicklookViewDock->toggleViewAction() );

  m_UI->menu_View->addSeparator();

  m_UI->menu_View->addSeparator();

#if USE_PIXEL_DESCRIPTION
  m_UI->menu_View->addAction( m_PixelDescriptionDock->toggleViewAction() );
#endif // USE_PIXEL_DESCRIPTION

#if defined( OTB_USE_QT4 ) && USE_OTB_APPS
  m_UI->menu_View->addAction(
    m_OtbApplicationsBrowserDock->toggleViewAction() );
#endif
}

/*****************************************************************************/
void
MainWindow
::ConnectStatusBar()
{
  assert( m_StatusBarWidget!=NULL );
  assert( m_ImageView!=NULL );

  QObject::connect(
    m_ImageView,
    SIGNAL( ScaleChanged( double, double ) ),
    // to:
    m_StatusBarWidget,
    SLOT( SetScale( double, double ) )
  );

  QObject::connect(
    m_StatusBarWidget,
    SIGNAL( ScaleChanged( double ) ),
    // to:
    m_ImageView->GetManipulator(),
    SLOT( ZoomTo( double ) )
  );

  QObject::connect(
    m_StatusBarWidget,
    SIGNAL( PixelIndexChanged( const IndexType& ) ),
    // to:
    m_ImageView,
    SLOT( CenterOnSelected( const IndexType& ) )
  );

  QObject::connect(
    m_ImageView,
    SIGNAL( PixelInfoChanged( const QPoint &, const PointType &, const PixelInfo::Vector & ) ),
    // to:
    this,
    SLOT( OnPixelInfoChanged( const QPoint &, const PointType &, const PixelInfo::Vector & ) )
  );
}

/*****************************************************************************/
#if USE_PIXEL_DESCRIPTION

void
MainWindow
::ConnectPixelDescriptionWidget( AbstractLayerModel * model )
{
  if( model==NULL ||
      !model->inherits( VectorImageModel::staticMetaObject.className() ) )
    return;

  // Access vector-image model.
  VectorImageModel * vectorImageModel =
    qobject_cast< VectorImageModel * >( model );

  assert( vectorImageModel==qobject_cast< VectorImageModel * >( model ) );
  assert( vectorImageModel!=NULL );

  //
  // send the physical point of the clicked point in screen
  // vectorImageModel is in charge of pixel information computation
  QObject::connect(
    m_ImageView,
    SIGNAL(
      PhysicalCursorPositionChanged(
	const QPoint &,
	const PointType &,
	const PointType &,
	const DefaultImageType::PixelType &
      )
    ),
    // to:
    vectorImageModel,
    SLOT(
      OnPhysicalCursorPositionChanged(
	const QPoint &,
	const PointType &,
	const PointType &,
	const DefaultImageType::PixelType&
      )
    )
  );

  QObject::connect(
    GetQuicklookView(),
    SIGNAL(
      PhysicalCursorPositionChanged(
	const QPoint &,
	const PointType &,
	const PointType &,
	const DefaultImageType::PixelType&
      )
    ),
    // to:
    vectorImageModel,
    SLOT(
      OnPhysicalCursorPositionChanged(
	const QPoint &,
	const PointType &,
	const PointType &,
	const DefaultImageType::PixelType&
      )
    )
  );

  // get the PixelDescription widget
  // TODO: Make a widget controller!
  PixelDescriptionWidget * pixelDescriptionWidget = 
    qobject_cast< PixelDescriptionWidget * >(
      m_PixelDescriptionDock->findChild< PixelDescriptionWidget * >()
    );

  assert( pixelDescriptionWidget!=NULL );

  QObject::connect(
    vectorImageModel,
    SIGNAL( CurrentPhysicalUpdated( const QStringList & ) ),
    // to:
    pixelDescriptionWidget,
    SLOT( OnCurrentPhysicalUpdated( const QStringList & ) )
  );

  QObject::connect(
    vectorImageModel,
    SIGNAL( CurrentGeographicUpdated( const QStringList & ) ),
    // to:
    pixelDescriptionWidget,
    SLOT( OnCurrentGeographicUpdated( const QStringList & ) )
  );

  QObject::connect(
    vectorImageModel,
    SIGNAL( CurrentPixelValueUpdated(const VectorImageType::PixelType &, 
                                     const QStringList & ) ),
    // to:
    pixelDescriptionWidget,
    SLOT( OnCurrentPixelValueUpdated(const VectorImageType::PixelType &, 
                                     const QStringList & ) )
  );
}

#endif // USE_PIXEL_DESCRIPTION

/*****************************************************************************/
#if USE_PIXEL_DESCRIPTION

void
MainWindow
::DisconnectPixelDescriptionWidget( const AbstractLayerModel * model )
{
  if( model==NULL ||
      !model->inherits( VectorImageModel::staticMetaObject.className() ) )
    return;

  // Access vector-image model.
  const VectorImageModel * vectorImageModel =
    qobject_cast< const VectorImageModel * >( model );

  assert( vectorImageModel==qobject_cast< const VectorImageModel * >( model ) );
  assert( vectorImageModel!=NULL );

  //
  // send the physical point of the clicked point in screen
  // vectorImageModel is in charge of pixel information computation
  QObject::disconnect(
    m_ImageView,
    SIGNAL(
      PhysicalCursorPositionChanged(
	const QPoint &,
	const PointType &,
	const PointType &,
	const DefaultImageType::PixelType &
      )
    ),
    // to:
    vectorImageModel,
    SLOT(
      OnPhysicalCursorPositionChanged(
	const QPoint &,
	const PointType &,
	const PointType &,
	const DefaultImageType::PixelType &
      )
    )
  );

  QObject::disconnect(
    GetQuicklookView(),
    SIGNAL(
      PhysicalCursorPositionChanged(
	const QPoint &,
	const PointType &,
	const PointType &,
	const DefaultImageType::PixelType &
      )
    ),
    // to:
    vectorImageModel,
    SLOT(
      OnPhysicalCursorPositionChanged(
	const QPoint &,
	const PointType &,
	const PointType &,
	const DefaultImageType::PixelType&
      )
    )
  );

  // get the PixelDescription widget
  // TODO: Make a widget controller!
  PixelDescriptionWidget * pixelDescriptionWidget = 
    qobject_cast< PixelDescriptionWidget * >(
      m_PixelDescriptionDock->findChild< PixelDescriptionWidget * >()
    );

  assert( pixelDescriptionWidget!=NULL );

  QObject::disconnect(
    vectorImageModel,
    SIGNAL( CurrentPhysicalUpdated( const QStringList & ) ),
    // to:
    pixelDescriptionWidget,
    SLOT( OnCurrentPhysicalUpdated( const QStringList & ) )
  );

  QObject::disconnect(
    vectorImageModel,
    SIGNAL( CurrentGeographicUpdated( const QStringList & ) ),
    // to:
    pixelDescriptionWidget,
    SLOT( OnCurrentGeographicUpdated( const QStringList & ) )
  );

  QObject::connect(
    vectorImageModel,
    SIGNAL(
      CurrentPixelValueUpdated(
        const VectorImageType::PixelType &,
        const QStringList &
      )
    ),
    // to:
    pixelDescriptionWidget,
    SLOT(
      OnCurrentPixelValueUpdated(
        const VectorImageType::PixelType &,
        const QStringList &
      )
    )
  );
}

#endif // USE_PIXEL_DESCRIPTION

/*****************************************************************************/
void
MainWindow
::InitializeDockWidgets()
{
#define ENABLE_QTOOLBAR_TEST 0
#if ENABLE_QTOOLBAR_TEST
  // XP: QToolBar test.
  QDockWidget* dock =
    AddWidgetToDock(
      m_UI->m_ToolBar,
      "TOOL_BAR",
      tr( "Tool bar" ),
      Qt::TopDockWidgetArea
    );

  m_UI->menu_View->addAction( dock->toggleViewAction() );
#endif

  //
  // Left pane.

#if defined( OTB_USE_QT4 ) && USE_OTB_APPS
  // OTB-applications browser.
  assert( m_OtbApplicationsBrowserDock==NULL );
  m_OtbApplicationsBrowserDock =
    AddDockWidget
    < ApplicationsToolBox, ApplicationsToolBoxController, QDockWidget >
    ( "APPLICATIONS_BROWSER",
      tr( "OTB Applications browser" ),
      Qt::LeftDockWidgetArea );

  // tabifyDockWidget( m_DatasetPropertiesDock, m_OtbApplicationsBrowserDock );
#endif

  //
  // Right pane.

  // Quicklook-view dock-widget
  assert( m_QuicklookViewDock==NULL );
  assert( m_ImageView!=NULL );
  m_QuicklookViewDock = AddWidgetToDock(
    CreateQuicklookViewWidget( m_ImageView ),
    "QUICKLOOK_VIEW",
    tr( "Quicklook view" ),
    Qt::RightDockWidgetArea
  );

  // Histogram-view.
  assert( m_HistogramDock==NULL );
  m_HistogramDock =
    AddDockWidget
    < HistogramWidget, HistogramController, QDockWidget >
    ( "HISTOGRAM",
      tr( "Histogram" ),
      Qt::RightDockWidgetArea,
      false
    );

#if USE_PIXEL_DESCRIPTION

  // Pixel Description (no controller needed here / direct update of
  // the pixel description through signals from VectorImageModel)
  assert( m_PixelDescriptionDock==NULL );
  m_PixelDescriptionDock =
    AddDockWidget
    < PixelDescriptionWidget, QDockWidget >
    ( "CURRENT_PIXEL_DESCRIPTION",
      tr( "Pixel Description" ),
      Qt::RightDockWidgetArea
    );

  tabifyDockWidget( m_PixelDescriptionDock, m_HistogramDock );

#endif // USE_PIXEL_DESCRIPTION

  // Color-setup.
  assert( m_ColorSetupDock==NULL );
  m_ColorSetupDock =
    AddDockWidget
    < ColorSetupWidget, ColorSetupController, QDockWidget >
    ( "COLOR_SETUP",
      tr( "Color setup" ),
      Qt::RightDockWidgetArea
    );

  // Color-dynamics.
  assert( m_ColorDynamicsDock==NULL );
  m_ColorDynamicsDock =
    AddDockWidget
    < ColorDynamicsWidget, ColorDynamicsController, QDockWidget >
    ( "COLOR_DYNAMICS",
      tr( "Color dynamics" ),
      Qt::RightDockWidgetArea
    );

  // Tabify dock-widgets.
  tabifyDockWidget( m_ColorSetupDock, m_ColorDynamicsDock );


  //
  // Bottom pane
  //

  // Layer-stack editor.
  assert( m_LayerStackDock==NULL );
  m_LayerStackDock = AddDockWidget<
    LayerStackWidget, LayerStackController, QDockWidget >(
      "LAYER_STACK",
      tr( "Layer stack" ),
      Qt::BottomDockWidgetArea
    );

}

/*****************************************************************************/
void
MainWindow
::InitializeCentralWidget()
{
#if USE_TABBED_VIEW
  // Create central tab-widget for multi-view support.
  assert( m_CentralTabWidget==NULL );
  m_CentralTabWidget = new QTabWidget( this );

  // Customize it.
  m_CentralTabWidget->setTabsClosable( true );

  // Set-it up as central widget.
  setCentralWidget( m_CentralTabWidget );

  //
  // access to the quicklook tabBar to remove the close button
  QTabBar* tabBar = m_CentralTabWidget->findChild< QTabBar* >();
#endif // USE_TABBED_VIEW

  // Initialize image-view.
  assert( m_ImageView==NULL );
  m_ImageView = CreateImageViewWidget();

#if USE_TABBED_VIEW
  // Add first tab: image-view.
  int index = m_CentralTabWidget->addTab(
    m_ImageView,
    tr( "Image view" )
  );

  tabBar->setTabButton( index, QTabBar::RightSide, 0);
  tabBar->setTabButton( index, QTabBar::LeftSide, 0);

#else // USE_TABBED_VIEW
  setCentralWidget( m_ImageView );

#endif // USE_TABBED_VIEW
}

/*****************************************************************************/
void
MainWindow
::InitializeRenderToolBar()
{
  m_UI->m_RenderToolBar->addSeparator();

  {
  m_UI->m_RenderToolBar->addWidget(
    new QLabel( tr( "Proj" ) )
  );

  QComboBox * comboBox =
    new QComboBox( m_UI->m_RenderToolBar );

  comboBox->setToolTip( tr( "Select projection used as reference for the view." ) );

  assert( comboBox!=NULL );

  comboBox->setObjectName( "referenceLayerComboBox" );
  comboBox->setMinimumSize(
    QSize(
#ifdef _DEBUG
      116,
#else
      128,
#endif
      0
    )
  );

  m_UI->m_RenderToolBar->addWidget( comboBox );
  }
}

/*****************************************************************************/
void
MainWindow
::InitializeShaderToolBar()
{
  assert( m_ShaderWidget==NULL );

  {
  m_UI->m_ShaderToolBar->addWidget(
    new QLabel( tr( "Layer FX" ) )
  );

  m_ShaderWidget = new ShaderWidget( m_UI->m_ShaderToolBar );

  m_UI->m_ShaderToolBar->addWidget( m_ShaderWidget );
  }
}

/*****************************************************************************/
void
MainWindow
::InitializeStatusBarWidgets()
{
  m_StatusBarWidget = new StatusBarWidget(statusBar());
  statusBar()->addPermanentWidget(m_StatusBarWidget, 1);
}

/*****************************************************************************/
ImageViewWidget*
MainWindow
::CreateImageViewWidget( QGLWidget* sharedGlWidget )
{
  ImageViewRenderer* renderer =
    new ImageViewRenderer( this );

  ImageViewManipulator* manipulator =
    new ImageViewManipulator(
#if USE_VIEW_SETTINGS_SIDE_EFFECT
      renderer->GetViewSettings(),
#endif // USE_VIEW_SETTINGS_SIDE_EFFECT
      this
    );

  ImageViewWidget* imageView = new ImageViewWidget(
    manipulator, // (will be reparented.)
    renderer, // (will be reparented.)
    this,
    sharedGlWidget
  );

  imageView->setMinimumWidth( 256 );

  return imageView;
}

/*****************************************************************************/
ImageViewWidget*
MainWindow
::CreateQuicklookViewWidget( QGLWidget* sharedGlWidget )
{
  QuicklookViewRenderer* renderer =
    new QuicklookViewRenderer( this );

  QuicklookViewManipulator* manipulator =
    new QuicklookViewManipulator(
#if USE_VIEW_SETTINGS_SIDE_EFFECT
      renderer->GetViewSettings(),
#endif // USE_VIEW_SETTINGS_SIDE_EFFECT
      this
    );

  ImageViewWidget* quicklookView = new ImageViewWidget(
    manipulator, // (will be reparented.)
    renderer, // (will be reparented.)
    this,
    sharedGlWidget
  );

  quicklookView->setMinimumSize(  64,  64 );
  quicklookView->setMaximumSize( 512, 512 );
  quicklookView->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );

  return quicklookView;
}

/*****************************************************************************/
void
MainWindow
::ImportImage( const QString& filename )
{
  // qDebug() << this << "::ImportImage(" << filename << "," << forceCreate << ")";

  //
  // Get stacked-layer.
  assert( Application::Instance() );
  assert(
    Application::Instance()->GetModel()==
    Application::Instance()->GetModel< StackedLayerModel >()
  );

  StackedLayerModel* stackedLayerModel =
    Application::Instance()->GetModel< StackedLayerModel >();

  assert( stackedLayerModel!=NULL );

  //
  // Count images regarding their spatial-reference type.
  size_t gcs = 0; // Geo/Carto/Sensor
  size_t unk = 0; // Unknown

  stackedLayerModel->CountSRT( unk, gcs, gcs, gcs );

  //
  //
  SpatialReferenceType srt = GetSpatialReferenceType( ToStdString( filename ) );

  QMessageBox::StandardButton button = QMessageBox::NoButton;

  if( unk==0 && gcs>0 && srt==SRT_UNKNOWN )
    {
    button =
      QMessageBox::warning(
	this,
	tr( "Monteverdi2 - Warning!" ),
	tr( "No projection information (geographical or cartographical coordinate-system or sensor model) has been found for image. While already loaded images all have some, they are displayed in a geo-referenced view.\n\nLoading '%1' will cause the display to switch to a non geo-referenced view (where images are displayed relatively regarding their origin and spacing)." )
	.arg( filename ),
	QMessageBox::Ok | QMessageBox::Cancel
      );
    }

  if( button==QMessageBox::Cancel )
    return;

  /*
  switch( button )
    {
    case QMessageBox::Cancel:
      return;
      break;

    case QMessageBox::Ok:
      // Disable reference-layer combo-box.
      // Disable layer-stack projection-button.
      break;

    default:
      assert( false && "Unhandled QMessageBox::StandardButton value!" );
      break;
    }
  */

  //
  // Import image file.
  VectorImageModel * imageModel = ImportImage( filename, -1, -1 );

  if( imageModel==NULL )
    return;

  //
  // Bypass rendering of image-views.
  assert( m_ImageView!=NULL );
  bool bypassImageView = m_ImageView->SetBypassRenderingEnabled( true );

  ImageViewWidget * quicklookView = GetQuicklookView();
  assert( quicklookView!=NULL );

  bool bypassQuicklookView = quicklookView->SetBypassRenderingEnabled( true );

  //
  // Store image-mode in layer-stack.
  stackedLayerModel->Add( imageModel );

  imageModel->setParent( stackedLayerModel );

  stackedLayerModel->SetCurrent( imageModel );

  bool hasReference = stackedLayerModel->HasReference();

  if( !hasReference && srt!=SRT_UNKNOWN )
    stackedLayerModel->SetReference( imageModel  );

  else if( hasReference && srt==SRT_UNKNOWN )
    stackedLayerModel->SetReference( StackedLayerModel::NIL_INDEX );

  //
  // Activate rendering of image-views.
  m_ImageView->SetBypassRenderingEnabled( bypassImageView );
  quicklookView->SetBypassRenderingEnabled( bypassQuicklookView );

  //
  // Set zoom-level which forces image-views refresh.
  if( !hasReference )
    UserZoomExtent();
}

/*****************************************************************************/
void
MainWindow
::closeEvent( QCloseEvent* event )
{
  I18nMainWindow::closeEvent( event );
}

/*****************************************************************************/
void
MainWindow
::virtual_InitializeUI()
{
  assert( Application::Instance() );

  Application::Instance()->SetModel( new StackedLayerModel() ); 
}

/*****************************************************************************/
void
MainWindow
::ConnectReferenceLayerComboBox( StackedLayerModel * model )
{
  assert( model!=NULL );

  QComboBox * comboBox =
    m_UI->m_RenderToolBar->findChild< QComboBox * >(
      REFERENCE_LAYER_COMBOBOX_NAME
    );

  assert( comboBox!=NULL );

  QObject::connect(
    comboBox,
    SIGNAL( currentIndexChanged( int ) ),
    // to:
    this,
    SLOT( OnReferenceLayerCurrentIndexChanged( int ) )
  );

  QObject::connect(
    model,
    SIGNAL( ContentChanged() ),
    // to:
    this,
    SLOT( RefreshReferenceLayerComboBox() )
  );

  QObject::connect(
    model,
    SIGNAL( ContentReset() ),
    // to:
    this,
    SLOT( RefreshReferenceLayerComboBox() )
  );

  QObject::connect(
    model,
    SIGNAL( OrderChanged() ),
    // to:
    this,
    SLOT( RefreshReferenceLayerComboBox() )
  );

  QObject::connect(
    model,
    SIGNAL( ReferenceChanged( size_t ) ),
    // to:
    this,
    SLOT( OnReferenceLayerChanged( size_t ) )
  );
}

/*****************************************************************************/
void
MainWindow
::SetupReferenceLayerComboBox( StackedLayerModel * model )
{
  QComboBox * comboBox =
    m_UI->m_RenderToolBar->findChild< QComboBox * >(
      REFERENCE_LAYER_COMBOBOX_NAME
    );

  assert( comboBox!=NULL );


  bool signalsBlocked = comboBox->blockSignals( true );
  {
  comboBox->clear();

  comboBox->addItem( "None" );
  }
  comboBox->blockSignals( signalsBlocked );


  if( model==NULL )
    return;


  for( StackedLayerModel::SizeType i=0;
       i<model->GetCount();
       ++i )
    {
    AbstractLayerModel * layer = model->At( i );
    assert( layer!=NULL );

    if( layer->inherits( VectorImageModel::staticMetaObject.className() ) )
      {
      const VectorImageModel * vectorImageModel =
        qobject_cast< const VectorImageModel * >( layer );

      assert( vectorImageModel!=NULL );

      comboBox->addItem(
        QString( "%1 (%2)" )
        .arg( layer->GetAuthorityCode( true ).c_str()  )
        .arg( QFileInfo( vectorImageModel->GetFilename() ).fileName() )
      );
      }
    else
      qDebug() << "Unhandled AbstractLayerModel subclass.";
    }

  {
  size_t gcs = 0; // Geo/Carto/Sensor
  size_t unk = 0; // Unknown

  model->CountSRT( unk, gcs, gcs, gcs );

  comboBox->setEnabled( model->GetCount()>0 && unk==0 );
  }

  comboBox->setCurrentIndex(
    model->GetReferenceIndex()>=model->GetCount()
    ? 0 // comboBox->count() - 1
    : model->GetReferenceIndex() + 1
  );
}

/*****************************************************************************/
void
MainWindow
::DisconnectReferenceLayerComboBox( StackedLayerModel * model )
{
  QComboBox * comboBox =
    m_UI->m_RenderToolBar->findChild< QComboBox * >(
      REFERENCE_LAYER_COMBOBOX_NAME
    );

  assert( comboBox!=NULL );

  QObject::disconnect(
    comboBox,
    SIGNAL( currentIndexChanged( int ) ),
    // to:
    this,
    SLOT( OnReferenceLayerCurrentIndexChanged( int ) )
  );

  if( model!=NULL )
    {
    QObject::disconnect(
      model,
      SIGNAL( ContentChanged() ),
      // to:
      this,
      SLOT( RefreshReferenceLayerComboBox() )
    );

    QObject::disconnect(
      model,
      SIGNAL( ContentReset() ),
      // to:
      this,
      SLOT( RefreshReferenceLayerComboBox() )
    );

    QObject::disconnect(
      model,
      SIGNAL( OrderChanged() ),
      // to:
      this,
      SLOT( RefreshReferenceLayerComboBox() )
    );

    QObject::disconnect(
      model,
      SIGNAL( ReferenceChanged( size_t ) ),
      // to:
      this,
      SLOT( OnReferenceLayerChanged( size_t ) )
    );
    }

  comboBox->clear();
  comboBox->setEnabled( false );
}

/*****************************************************************************/
/* SLOTS                                                                     */
/*****************************************************************************/
void
MainWindow
::on_action_Keymap_triggered()
{
  KeymapDialog keymapDialog( this );

  keymapDialog.exec();
}

/*****************************************************************************/
void
MainWindow
::on_action_OpenImage_triggered()
{
  //
  // Select filename.
  ImportImages(
    I18nMainWindow::GetOpenFileNames( this, tr( "Open file..." ) )
  );
}

/*****************************************************************************/
void
MainWindow
::on_action_Preferences_triggered()
{
  PreferencesDialog prefDialog( this );

  prefDialog.exec();
}

/*****************************************************************************/
void
MainWindow
::on_action_ZoomIn_triggered()
{
  emit UserZoomIn();
}

/*****************************************************************************/
void
MainWindow
::on_action_ZoomOut_triggered()
{
  emit UserZoomOut();
}

/*****************************************************************************/
void
MainWindow
::on_action_ZoomExtent_triggered()
{
  emit UserZoomExtent();
}

/*****************************************************************************/
void
MainWindow
::on_action_ZoomFull_triggered()
{
  emit UserZoomFull();
}

/*****************************************************************************/
void
MainWindow
::on_action_ZoomLayer_triggered()
{
  emit UserZoomLayer();
}

/*****************************************************************************/
void
MainWindow
::OnAboutToChangeModel( const AbstractModel * )
{
  // qDebug() << this << "::OnAboutToChangeModel(" << model << ")";


  // Force to disconnect previously selected layer-model before
  // stacked-layer model is disconnected.
  //
  // If there was no previously set layer-model, this will cause GUI
  // views to be disabled.
  //
  // N.B.: This will cause UI controllers to disable widgets.
  OnAboutToChangeSelectedLayerModel( StackedLayerModel::KeyType() );

  // Assign stacked-layer model controller.
  SetControllerModel( m_LayerStackDock, NULL );


  assert( Application::Instance() );
  assert( Application::Instance()->GetModel()==
          Application::Instance()->GetModel< StackedLayerModel >() );

  StackedLayerModel * stackedLayerModel =
    Application::Instance()->GetModel< StackedLayerModel >();

  DisconnectReferenceLayerComboBox( stackedLayerModel );

  // Exit, if there were no previously set database model.
  if( stackedLayerModel==NULL )
    return;

  // Disonnect stacked-layer model from main-window when selected
  // layer-model is about to change.
  QObject::disconnect(
    stackedLayerModel,
    SIGNAL(
      AboutToChangeSelectedLayerModel( const StackedLayerModel::KeyType & )
    ),
    // from:
    this,
    SLOT(
      OnAboutToChangeSelectedLayerModel( const StackedLayerModel::KeyType & )
    )
  );

  // Disconnect stacked-layer model to main-window when selected
  // layer-model has been changed.
  QObject::disconnect(
    stackedLayerModel,
    SIGNAL( SelectedLayerModelChanged( const StackedLayerModel::KeyType & ) ),
    // from:
    this,
    SLOT( OnSelectedLayerModelChanged( const StackedLayerModel:KeyType & ) )
  );
}

/*****************************************************************************/
void
MainWindow
::OnModelChanged( AbstractModel * model )
{
  // qDebug() << this << "::OnModelChanged(" << model << ")";

  assert( model==qobject_cast< StackedLayerModel * >( model ) );

  StackedLayerModel * stackedLayerModel =
    qobject_cast< StackedLayerModel * >( model );

  SetupReferenceLayerComboBox( stackedLayerModel );
  ConnectReferenceLayerComboBox( stackedLayerModel );

  // Assign stacked-layer model controller.
  SetControllerModel( m_LayerStackDock, model );

  m_ImageView->SetLayerStack( stackedLayerModel );

  assert( GetQuicklookView()!=NULL );
  GetQuicklookView()->SetLayerStack( stackedLayerModel );

  if( stackedLayerModel==NULL )
    return;


  // Connect stacked-layer model to main-window when selected layer-model
  // is about to change.
  QObject::connect(
    stackedLayerModel,
    SIGNAL(
      AboutToChangeSelectedLayerModel( const StackedLayerModel::KeyType & )
    ),
    // to:
    this,
    SLOT(
      OnAboutToChangeSelectedLayerModel( const StackedLayerModel::KeyType & )
    )
  );

  // Connect stacked-layer -model to main-window when selected layer-model
  // has been changed.
  QObject::connect(
    stackedLayerModel,
    SIGNAL( SelectedLayerModelChanged( const StackedLayerModel::KeyType & ) ),
    // to:
    this,
    SLOT( OnSelectedLayerModelChanged( const StackedLayerModel::KeyType & ) )
  );

  // Force to connect selected layer-model after stacked-layer model
  // is connected.
  //
  // N.B.: This will cause UI controllers to disable widgets.
  OnSelectedLayerModelChanged( stackedLayerModel->GetCurrentKey() );
}

/*****************************************************************************/
void
MainWindow
::OnAboutToChangeSelectedLayerModel( const StackedLayerModel::KeyType & )
{
  // qDebug()
  //   << this
  //   << "::OnAboutToChangeSelectedDatasetModel(" << FromStdString( key ) << ")";

  //
  // CONTROLLERS.
  //

  //
  // Unset model from controllers.
  //
  // N.B.: This step must be done *before* disconnecting signals &
  // slots between model(s) and view(s).
  //
  // See also, ::OnSelectedLayerModel() changed.

  // Unset image-model from color-dynamics controller.
  SetControllerModel( m_ColorDynamicsDock, NULL );

  // Unset image-model from color-setup controller.
  SetControllerModel( m_ColorSetupDock, NULL );

  // Unset histogram-model from histogram controller.
  SetControllerModel( m_HistogramDock, NULL );

  // Unset stacked-layer model from stacked-layer controller.
  // SetControllerModel( m_StackedLayerDock, NULL );

  //
  // VIEWS.
  //

  //
  // MODEL(s).
  //
  assert( Application::Instance() );
  assert( Application::Instance()->GetModel()==
          Application::Instance()->GetModel< StackedLayerModel >() );

  const StackedLayerModel * stackedLayerModel =
    Application::Instance()->GetModel< StackedLayerModel >();

  if( !stackedLayerModel )
    return;

  const AbstractLayerModel * layerModel = stackedLayerModel->GetCurrent();

  if( !layerModel )
    return;

  if( layerModel->inherits( VectorImageModel::staticMetaObject.className() ) )
    {
    m_ShaderWidget->SetSettings( NULL );

    // Disconnect previously selected image-model from view.
    QObject::disconnect(
      layerModel,
      SIGNAL( SettingsUpdated() ),
      // from:
      this,
      SLOT( OnSettingsUpdated()  )
    );

    // Disconnect previously selected quicklook-model from view.
    // TODO: Remove quicklook temporary hack by better design.
    QObject::disconnect(
      layerModel,
      SIGNAL( SettingsUpdated() ),
      // from:
      this,
      SLOT( OnSettingsUpdated()  )
    );

    //
    // Disconnect shader-widget from model-updated slot.
    QObject::disconnect(
      m_ShaderWidget,
      SIGNAL( SettingsChanged() ),
      // from:
      layerModel,
      SLOT( OnModelUpdated() )
    );
    }

  else
    {
    assert( false && "Unhandled AbstractLayerModel derived-type." );
    }

#if USE_PIXEL_DESCRIPTION
  DisconnectPixelDescriptionWidget( layerModel );
#endif // USE_PIXEL_DESCRIPTION
}

/*****************************************************************************/
void
MainWindow
::OnSelectedLayerModelChanged( const StackedLayerModel::KeyType & key )
{
  // qDebug()
  //   << this << "::OnLayerModelChanged( " << FromStdString( key ) << ")";

  assert( Application::Instance() );
  assert( Application::Instance()->GetModel()==
          Application::Instance()->GetModel< StackedLayerModel >() );

  StackedLayerModel * stackedLayerModel =
    Application::Instance()->GetModel< StackedLayerModel >();

  assert( stackedLayerModel==Application::Instance()->GetModel() );

  if( stackedLayerModel==NULL )
    return;

  //
  // VIEWS.
  //

  //
  // MODEL(s).
  //

  AbstractLayerModel * layerModel = stackedLayerModel->Get( key );

  if( !layerModel )
    return;

  if( layerModel->inherits( VectorImageModel::staticMetaObject.className() ) )
    {
    //
    // SAT: Using m_TabWidget->index( 0 ) or m_ImageView is equivalent
    // since Qt may use signal & slot names to connect (see MOC .cxx
    // files). Thus, using m_ImageView saves one indirection call.
    QObject::connect(
      layerModel,
      SIGNAL( SettingsUpdated() ),
      // to:
      this,
      SLOT( OnSettingsUpdated()  )
    );

    QObject::connect(
      layerModel,
      SIGNAL( SettingsUpdated() ),
      // to:
      this,
      SLOT( OnSettingsUpdated() )
    );

    //
    // Connect shader-widget to model-updated slot.
    QObject::connect(
      m_ShaderWidget,
      SIGNAL( SettingsChanged() ),
      // to:
      layerModel,
      SLOT( OnModelUpdated() )
    );

    VectorImageModel * imageModel =
      qobject_cast< VectorImageModel * >( layerModel );

    assert( imageModel!=NULL );

    setWindowTitle(
      QString( PROJECT_NAME " - %1" )
      .arg( QFileInfo( imageModel->GetFilename() ).fileName() )
    );

    m_ShaderWidget->SetSettings( &imageModel->GetSettings() );
    }
  else
    {
    assert( false && "Unhandled AbstractLayerModel derived-type." );
    }

#if USE_PIXEL_DESCRIPTION
  ConnectPixelDescriptionWidget( layerModel );
#endif // USE_PIXEL_DESCRIPTION

  //
  // CONTROLLERS.
  //

  //
  // Connect image-model controllers.
  //
  // N.B.: This step *must* be done after signals and slots between
  // model(s) and view(s) have been connected (because when model is
  // assigned to controller, widgets/view are reset and emit
  // refreshing signals).
  //
  // See also: OnAboutToChangeLayerModel().

  // Assign dataset-model to dataset-properties controller.

  // Assign image-model to color-dynamics controller.
  SetControllerModel( m_ColorDynamicsDock, layerModel );

  // Assign image-model to color-setup controller.
  SetControllerModel( m_ColorSetupDock, layerModel );

  // Assign histogram-model to histogram controller.
  SetControllerModel( m_HistogramDock, layerModel );

  //
  // TOOLBAR.
  //
  m_UI->action_ZoomIn->setEnabled( layerModel!=NULL );
  m_UI->action_ZoomOut->setEnabled( layerModel!=NULL );
  m_UI->action_ZoomExtent->setEnabled( layerModel!=NULL );
  m_UI->action_ZoomFull->setEnabled( layerModel!=NULL );
  m_UI->action_ZoomLayer->setEnabled( layerModel!=NULL );
}

/*****************************************************************************/
#if defined( OTB_USE_QT4 ) && USE_OTB_APPS

void
MainWindow
::OnApplicationToLaunchSelected( const QString & appName,
				 const QString & docName )
{
  assert( Application::ConstInstance()!=NULL );
  assert( Application::ConstInstance()->GetOTBApplicationsModel()!=NULL );
  assert(
    Application::ConstInstance()->GetOTBApplicationsModel()->GetLauncher()!=NULL
  );

  Wrapper::QtWidgetView* appWidget =
    Application::ConstInstance()
    ->GetOTBApplicationsModel()
    ->GetLauncher()
    ->NewOtbApplicationWidget( appName );

  assert( appWidget!=NULL );

#if USE_TABBED_VIEW

  //
  // add the application in a tab
  // TODO : check if this application is already opened ???
  int tabIndex = m_CentralTabWidget->addTab(
    appWidget, QIcon( ":/icons/process" ), docName );

  // no checking needed here, if index is not available nothing is
  // done. Focus on the newly added tab
  m_CentralTabWidget->setCurrentIndex( tabIndex );

#else // USE_TABBED_VIEW

  assert( false && "OTB-Application widget newed and not linked to QApplication interface." );

#endif // USE_TABBED_VIEW

  //
  // connections. not using m_CentralTabWidget->currentWidget() leads
  // to a wrong connection!!!!
  QObject::connect(
    appWidget,
    SIGNAL( OTBApplicationOutputImageChanged( const QString&,
					      const QString& ) ),
    // to:
    this,
    SLOT( OnOTBApplicationOutputImageChanged( const QString&,
					      const QString& ) )
    );

  QObject::connect(
    appWidget,
    SIGNAL( ExecutionDone( int ) ),
    // to:
    this,
    SLOT( OnExecutionDone( int ) )
  );

  //
  // on quit widget signal, close its tab
#if USE_TABBED_VIEW

  QObject::connect(
    appWidget,
    SIGNAL( QuitSignal() ),
    // to:
    this,
    SLOT( OnTabCloseRequested() )
    );

#endif // USE_TABBED_VIEW
}

#endif // defined( OTB_USE_QT4 ) && USE_OTB_APPS

/*****************************************************************************/
#if USE_TABBED_VIEW

void
MainWindow
::OnTabCloseRequested()
{
  // get current tab index
  int currentIndex = m_CentralTabWidget->currentIndex();

  // close tab and delete its widget
  OnTabCloseRequested( currentIndex );
}

#endif // USE_TABBED_VIEW

/*****************************************************************************/
#if USE_TABBED_VIEW

void
MainWindow
::OnTabCloseRequested( int index )
{
  assert( index >= 1 );

  QWidget* appWidget = m_CentralTabWidget->widget( index );
  assert( appWidget!=NULL );

#if defined( OTB_USE_QT4 ) && USE_OTB_APPS

  assert( appWidget==qobject_cast< Wrapper::QtWidgetView* >( appWidget ) );
  Wrapper::QtWidgetView* appWidgetView =
    qobject_cast< Wrapper::QtWidgetView* >( appWidget );
  assert( appWidgetView!=NULL );

  if( !appWidgetView->IsClosable() )
    {
    QMessageBox::warning(
      this,
      tr( "Monteverdi2 - Warning!" ),
      tr( "Tab cannot be closed while OTB application is running." )
    );
   
    return;
    }

#endif

  m_CentralTabWidget->removeTab( index );

  delete appWidget;
  appWidget = NULL;
}

#endif // USE_TABBED_VIEW

/*****************************************************************************/
void
MainWindow
::OnOTBApplicationOutputImageChanged( const QString &,
				      const QString & outfname )
{
  //
  // If this slot is called, it means that an application has finished
  // its process and has an output image parameter. The 'outfname' in
  // parameters is the output filename. This slot may launch or not,
  // depending on the app settings, the import of the 'outfname' to the
  // catalog database.

  // import the result image into the database
  ImportImage( outfname );
}

/*****************************************************************************/
void
MainWindow
::OnExecutionDone( int status )
{
  if( status<0 )
    return;
}

/*****************************************************************************/
void
MainWindow
::OnReferenceLayerChanged( size_t index )
{
  // qDebug() << this << "::OnReferenceLayerChanged(" << index << ")";

  //
  // Access widget.
  QComboBox * comboBox =
    m_UI->m_RenderToolBar->findChild< QComboBox * >(
      REFERENCE_LAYER_COMBOBOX_NAME
    );

  assert( comboBox!=NULL );

  //
  // Access model.
  assert( I18nCoreApplication::Instance()!=NULL );
  assert( I18nCoreApplication::Instance()->GetModel()==
          I18nCoreApplication::Instance()->GetModel< StackedLayerModel >() );

  StackedLayerModel * model = 
    I18nCoreApplication::Instance()->GetModel< StackedLayerModel >();

  // assert( model!=NULL );

  //
  // Update widget from model.
  // comboBox->setCurrentIndex(
  //   model->GetReferenceIndex()>=model->GetCount()
  //   ? 0 // comboBox->count() - 1
  //   : model->GetReferenceIndex() + 1
  // );
  comboBox->setCurrentIndex(
    index>=model->GetCount()
    ? 0
    : index + 1
  );
}

/*****************************************************************************/
void
MainWindow
::OnReferenceLayerCurrentIndexChanged( int index )
{
  // qDebug() << this << "::OnReferenceLayerCurrentIndexChanged(" << index << ")";

  //
  // Access widget.
  QComboBox * comboBox =
    m_UI->m_RenderToolBar->findChild< QComboBox * >(
      REFERENCE_LAYER_COMBOBOX_NAME
    );

  assert( comboBox!=NULL );

  //
  // Access model.
  assert( I18nCoreApplication::Instance()!=NULL );
  assert( I18nCoreApplication::Instance()->GetModel()==
          I18nCoreApplication::Instance()->GetModel< StackedLayerModel >() );

  StackedLayerModel * model = 
    I18nCoreApplication::Instance()->GetModel< StackedLayerModel >();

  assert( model!=NULL );


  //
  // Update model.
  model->SetReference(
    index<=0 // index>=comboBox->count() - 1
    ? StackedLayerModel::NIL_INDEX
    : comboBox->currentIndex() - 1
  );
}

/*****************************************************************************/
void
MainWindow
::OnSettingsUpdated()
{
  assert( m_ShaderWidget!=NULL );

  m_ShaderWidget->UpdateSettings();

  //
  {
  assert( m_ColorSetupDock!=NULL );

  AbstractModelController * controller = GetController( m_ColorSetupDock );
  assert( controller!=NULL );

  if( controller->GetModel()!=NULL )
    controller->ResetWidget();
  }
  //
  {
  assert( m_ColorDynamicsDock!=NULL );

  AbstractModelController * controller = GetController( m_ColorDynamicsDock );
  assert( controller!=NULL );

  if( controller->GetModel()!=NULL )
    controller->ResetWidget();
  }
  //

  assert( m_ImageView!=NULL );

  m_ImageView->updateGL();

  //

  ImageViewWidget * quicklookView = GetQuicklookView();
  assert( quicklookView!=NULL );

  quicklookView->updateGL();
}

/****************************************************************************/
void
MainWindow
::RefreshReferenceLayerComboBox()
{
  // qDebug() << this << "::RefreshReferenceLayerComboBox()";

  assert( I18nCoreApplication::Instance()!=NULL );
  assert( I18nCoreApplication::Instance()->GetModel()==
          I18nCoreApplication::Instance()->GetModel< StackedLayerModel >() );

  SetupReferenceLayerComboBox(
    I18nCoreApplication::Instance()->GetModel< StackedLayerModel >()
  );
}

/****************************************************************************/
void
MainWindow
::OnPixelInfoChanged( const QPoint &,
		      const PointType &,
		      const PixelInfo::Vector & pixels )
{
  //
  // Get stacked-layer.
  assert( Application::Instance() );
  assert(
    Application::Instance()->GetModel()==
    Application::Instance()->GetModel< StackedLayerModel >()
  );

  const StackedLayerModel * stackedLayerModel =
    Application::Instance()->GetModel< StackedLayerModel >();

  assert( stackedLayerModel!=NULL );

  if( !stackedLayerModel->HasCurrent() )
    {
    m_StatusBarWidget->SetPixelIndex( IndexType(), false );
    m_StatusBarWidget->SetText( QString() );

    m_StatusBarWidget->setEnabled( false );
    return;
    }

  m_StatusBarWidget->setEnabled( true );

  StackedLayerModel::SizeType current = stackedLayerModel->GetCurrentIndex();
  assert( current!=StackedLayerModel::NIL_INDEX );

  m_StatusBarWidget->SetPixelIndex( pixels[ current ].m_Index, pixels[ current ].m_HasIndex );

  QString text;

  if( pixels[ current ].m_HasPoint )
    {
    assert( stackedLayerModel->GetCurrent()!=NULL );

    PointType wgs84;
    double alt = std::numeric_limits< double >::quiet_NaN();

    stackedLayerModel->GetCurrent()->ToWgs84( pixels[ current ].m_Point, wgs84, alt );

    text =
      tr( "(%1 %2 ; %3 %4 ; %5)" )
      .arg( wgs84[ 0 ]>=0.0 ? "N" : "S" ).arg( fabs( wgs84[ 1 ] ) )
      .arg( wgs84[ 1 ]>=0.0 ? "E" : "W" ).arg( fabs( wgs84[ 0 ] ) )
      .arg( alt );
    }

  if( pixels[ current ].m_HasPixel )
    {
    if( !text.isEmpty() )
      text.append( " " );

    text.append(
      tr( " [ R: %1 ; G: %2 ; B: %3 ]" )
      .arg( pixels[ current ].m_Pixel[ 0 ] )
      .arg( pixels[ current ].m_Pixel[ 1 ] )
      .arg( pixels[ current ].m_Pixel[ 2 ] )
    );
    }

  m_StatusBarWidget->SetText( text );
}

} // end namespace 'mvd'