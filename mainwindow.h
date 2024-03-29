#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QThread>
#include <opencv2/opencv.hpp>
#include <rclcpp/rclcpp.hpp>
#include <QFileDialog>
#include <QtCore/QTextCodec>
#include <libssh/libssh.h>
#include <QElapsedTimer>

#define vtkRenderingCore_AUTOINIT 3(vtkRenderingOpenGL2, vtkInteractionStyle,vtkRenderingFreeType)
#include <vtkExtractSelection.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkCellPicker.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkScalarBarWidget.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkCubeAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkAxesActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCommand.h>
#include <vtkBorderWidget.h>
#include <vtkCamera.h>
#include <vtkBorderWidget.h>
#include <vtkCommand.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkVersion.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkAssembly.h>
#include <QVTKOpenGLNativeWidget.h>
#include <pcl/common/common_headers.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/console/parse.h>
#include <pcl/range_image/range_image.h>
#include <pcl/visualization/common/float_image_utils.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/common/common.h>
#include <pcl/io/png_io.h>
#include <vtkColorSeries.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkPropPicker.h>
#include <vtkActor2DCollection.h>

#include "my_params.h"
#include "mypclfunction.h"
#include "showimgpcddlg.h"
#include "getcurtime.h"
#include "cambuilddlg.h"
#include "laser_paramsetingdlg.h"
#include "label_image_viewer.h"

#include <QScatterSeries>
#include <QValueAxis>
QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ImgWindowShowThread;  // 图像窗口显示线程，前置声明

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    My_params *m_mcs;
    cv::Mat pImage;//要计算的图像

    cambuilddlg * cambuild;

    std::vector<cv::Point3f> cv_line; //要计算的点云
    MyPclFunction pclclass;//点云算法

    void img_windowshow(bool b_show, QLabel *lab_show);      //显示图像
    void showpoint(std::string filename);
    pcl::PointCloud<pcl::PointXYZ>::Ptr pclclould;
    volatile  bool b_int_show_record_finish;
    int indexImgShowLabel;      // 图像显示的窗口索引

    // VTK显示点云
    vtkSmartPointer<vtkNamedColors> colors;
    vtkSmartPointer<vtkEventQtSlotConnect> slotConnector;
    vtkSmartPointer<vtkPoints> points;
    vtkSmartPointer<vtkCellArray> cells;
    vtkSmartPointer<vtkPolyData> polydata;
    vtkSmartPointer<vtkFloatArray> scalars;
    vtkSmartPointer<vtkLookupTable> lut;
    vtkSmartPointer<vtkPolyDataMapper> mapper;
    vtkSmartPointer<vtkActor> actor;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkScalarBarActor> scalarBar;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkAxesActor> axes_actor;
    vtkSmartPointer<InteractionStyle::MouseInteractorStylePP> style;
    vtkSmartPointer<vtkCubeAxesActor> cubeAxesActor;
    vtkSmartPointer<vtkPropPicker> propPicker;
    vtkSmartPointer<vtkRenderWindowInteractor> iren;
    vtkSmartPointer<vtkOrientationMarkerWidget> axes_actorWidget;
    vtkSmartPointer<vtkScalarBarWidget> scalarBarWidget;
//    vtkSmartPointer<vtkTextActor> textActorFPS;
//    vtkPropPicker*  Picker;          // Pointer to the picker

    bool b_imgshow_thread;      //线程运行标记
    bool stop_b_imgshow_thread;  //是否成功断开线程

    bool u8_save_data;     //保存图像
    QString save_imgdata_cvimage(cv::Mat cv_image);  //保存opencv类型图
    QString save_pcldata_pclclould(pcl::PointCloud<pcl::PointXYZRGB>::Ptr pclclould);

    void start_deepimg();         //开始采集深度图
    void stop_deepimg();          //停止采集深度

    void vtk_init();

    volatile bool b_int_show_cvimage_inlab_finish;          //int_show_cvimage_inlab信号曹空闲
    volatile bool b_init_show_pclclould_list_finish;          //init_show_pclclould_list信号曹空闲

    volatile bool finish_line;       // 单条轮廓是否采集完成
    volatile bool camera_reset_once;    // 仅首次重置相机
    volatile bool camera_reset_always;    // 每次刷新都重置相机
    volatile bool finish_cloud;  // 整个点云是否采集完成
    volatile bool cloud2deepimg;  // 整个点云是否可以转为深度图
    volatile bool updateVTKShow;    // 是否刷新轮廓显示窗口
    volatile bool recordVideo;    // 是否开始录制视频

private slots:
    void int_show_cvimage_inlab(cv::Mat cv_image);// 显示图像
    void init_show_pclclould_list(pcl::PointCloud<pcl::PointXYZRGB>::Ptr pclclould);      //在QVTKWidgetlib中显示点云
//    void int_show_record(QString msg);
    void slot_timer_tragetor_clould();      //轨迹进入点云的定时器中断函数

    void doDisMeasure(bool value);      // 两点距离测量
    void doDockerRestart();             // 重启docker镜像
    void recordAsVideo();               // 将图像保存为视频

private:
    Ui::MainWindow *ui;
    showImgPcdDlg * showImgPcd;

    ImgWindowShowThread *imgshow_thread;

    laser_paramsetingdlg *paramset;     // 参数设置
//    LabelImageViewer * imgShowLabel;  // 自定义图像显示的label

    void UpdateUi();  //刷新控件显示和使能
    void InitSetEdit(); //初始化控件数字

    QTimer *timer_tragetor_clould;      //轨迹进入点云的定时器

    void open_camer_modbus();
    void close_camer_modbus();

    void showupdata_tabWidget();

    QString ipAddress;
    QString GetCurTime_M();

    // vtk显示点云
    vtkSmartPointer<vtkEventQtSlotConnect> Connections;

    QScatterSeries* series;         // 散点图chart
    QValueAxis *axisX;
    QValueAxis *axisY;
    void initChart();

    cv::VideoWriter videoWriter;    // 视频写入
    QElapsedTimer timerElapsed;     // 计时器计算FPS
    QTimer *fpsShowTimer;
    qint64 fpsShow;
};

// 图像显示线程
class ImgWindowShowThread : public QThread
{
    Q_OBJECT
public:
    ImgWindowShowThread(MainWindow *statci_p);
    void Stop();
    void Lock();
    void unLock();
protected:
    void run();
private:
    MainWindow *_p;
    bool lock;
signals:
    // 自定义信号
//    void Send_show_record(QString msg);
    void Send_show_cvimage_inlab(cv::Mat cv_image);       //发送图像显示信号
    void Send_show_pclclould_list(pcl::PointCloud<pcl::PointXYZRGB>::Ptr pclclould);      //发送点云显示信号
};

#endif // MAINWINDOW_H
