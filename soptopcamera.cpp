#include "soptopcamera.h"

// 图像订阅节点
Camshow::Camshow(SoptopCamera *statci_p): Node("qt_imgshow")
{
  _p = statci_p;

  subscription_ = this->create_subscription<tutorial_interfaces::msg::IfAlgorhmitimage>(
        "camera_tis_node/image", rclcpp::SensorDataQoS(), std::bind(&Camshow::topic_callback, this, _1));

//  subscriresult_ = this->create_subscription<tutorial_interfaces::msg::IfAlgorhmitmsg>(
//        "/laser_imagepos_node/result", rclcpp::SensorDataQoS(), std::bind(&Camshow::result_callback, this, _1));

}

Camshow::~Camshow()
{

}

void Camshow::topic_callback(const tutorial_interfaces::msg::IfAlgorhmitimage msg)  const
{
  if(_p->b_connect==true)
  {
    cv_bridge::CvImagePtr cv_ptr;
    cv_ptr = cv_bridge::toCvCopy(msg.image, msg.image.encoding);
    *(_p->cv_image) = cv_ptr->image.clone();
    _p->b_updataimage_finish=true;
    _p->callbacknumber++;
  }
  else
  {
//  system("ros2 param set gpio_raspberry_node laser False");  //激光关闭
//  system("ros2 param set /camera_tis_node power False");     //相机关闭
    rclcpp::shutdown();
    _p->stop_b_connect=true;
  }
}

/*
void Camshow::result_callback(tutorial_interfaces::msg::IfAlgorhmitmsg msg)  const
{
  if(_p->b_connect==true)
  {
    cv_bridge::CvImagePtr cv_ptr;
    cv_ptr = cv_bridge::toCvCopy(msg.imageout, msg.imageout.encoding);
    *(_p->cv_image)=cv_ptr->image.clone();
    _p->b_updataimage_finish=true;
  }
  else
  {
//  system("ros2 param set gpio_raspberry_node laser False");  //激光关闭
//  system("ros2 param set /camera_tis_node power False");     //相机关闭
    rclcpp::shutdown();
    _p->stop_b_connect=true;
  }
}
*/

// 点云订阅节点
Cloudshow::Cloudshow(SoptopCamera *statci_p): Node("qt_cloudshow")
{
  _p=statci_p;

  subscricloud_ = this->create_subscription<tutorial_interfaces::msg::IfAlgorhmitcloud>(
        "line_center_reconstruction_node/cloud_task100_199", rclcpp::SensorDataQoS(), std::bind(&Cloudshow::cloud_callback, this, _1));

}

Cloudshow::~Cloudshow()
{

}

void Cloudshow::cloud_callback(const tutorial_interfaces::msg::IfAlgorhmitcloud msg)  const
{
  if(_p->b_connect==true)
  {
    if(msg.lasertrackoutcloud.size()>0)
    {
      std::vector<cv::Point3f> cv_ptr;
      cv_ptr.resize(msg.lasertrackoutcloud.size());
      for(int n=0;n<(int)msg.lasertrackoutcloud.size();n++)
      {
        cv_ptr[n].x=msg.lasertrackoutcloud[n].x;
        cv_ptr[n].y=msg.lasertrackoutcloud[n].y;
        cv_ptr[n].z=msg.lasertrackoutcloud[n].u;
      }
      (*(_p->cv_line)).linepoint=cv_ptr;
      (*(_p->cv_line)).linehead=msg.header;
      _p->b_cv_lineEn=true;
    }
    else
    {
      _p->b_cv_lineEn=false;
    }
    _p->b_updatacloud_finish=true;
    _p->callbacknumber++;
  }
  else
  {

    rclcpp::shutdown();
    _p->stop_b_connect=true;
  }
}

// 相机构造函数
SoptopCamera::SoptopCamera()
{
  i32_exposure_min=SOPTOPCAM_EXPOSURE_MIN;
  i32_exposure_max=SOPTOPCAM_EXPOSURE_MAX;
  i32_exposure_use=SOPTOPCAM_EXPOSURE_USE;

  read_para();

  cv_image=new cv::Mat;
  cv_line=new Ros2lineinfo;
  b_connect=false;
  b_updataimage_finish=false;
  b_updatacloud_finish=false;
  node_mode=0;
  callbacknumber=0;
  oldcallbacknumber=0;
  callback_error=0;
  b_stopthred=true;
  StartCamera_thread = new StartCameraThread(this);
}

SoptopCamera::~SoptopCamera()
{
  StartCamera_thread->quit();
  StartCamera_thread->wait();
  delete StartCamera_thread;
  StartCamera_thread=NULL;
  delete cv_image;
  delete cv_line;
}

void SoptopCamera::timerEvent(QTimerEvent *event)
{
    if(event->timerId()==timerid1)
    {
        if(b_connect==true||stop_b_connect==false)
        {
            if(callbacknumber==oldcallbacknumber)
            {
                callback_error=1;
            }
            else
            {
                callback_error=0;
            }
            oldcallbacknumber=callbacknumber;
        }
    }
}


void SoptopCamera::read_para()
{
    uint8_t *buff=new uint8_t[SOPTOPCAM_SAVEBUFF];
    if(buff==NULL)
    {
      init_para();
      return;
    }
    CFileOut fo;
    if(0 > fo.ReadFile((char *)SOPTOPCAM_SYSPATH_MOTO,buff,SOPTOPCAM_SAVEBUFF))
    {
      init_para();
      return;
    }
    int32_t *i32_p;
    i32_p = (int32_t*)buff;
    i32_exposure = *i32_p;
    i32_p++;

    delete[] buff;
}

void SoptopCamera::write_para()
{
    check_para();
    uint8_t *buff=new uint8_t[SOPTOPCAM_SAVEBUFF];
    if(buff==NULL)
      return;

    int32_t *i32_p;

    i32_p = (int32_t*)buff;
    *i32_p = i32_exposure;
    i32_p ++;

    CFileOut fo;
    fo.WriteFile((char *)SOPTOPCAM_SYSPATH_MOTO,buff,SOPTOPCAM_SAVEBUFF);

    if(buff!=NULL)
      delete []buff;
}


void SoptopCamera::check_para()
{
    if(i32_exposure<i32_exposure_min||i32_exposure>i32_exposure_max)
    {
      i32_exposure=i32_exposure_use;
    }
}

void SoptopCamera::init_para()
{
    i32_exposure=i32_exposure_use;
}

void SoptopCamera::InitConnect(QLabel *lab_show)
{
  if(b_connect==false)
  {
    callbacknumber=0;
    oldcallbacknumber=0;
    timerid1 = startTimer(1000);
    m_lab_show=lab_show;
    b_connect=true;
    StartCamera_thread->start();
  }
}

// qjq
void SoptopCamera::InitConnect1()
{
  if(b_connect==false)
  {
    callbacknumber=0;
    oldcallbacknumber=0;
    timerid1 = startTimer(1000);
//    delete m_lab_show;
    b_connect=true;
    StartCamera_thread->start();
  }
}

void SoptopCamera::DisConnect()
{
  if(b_connect==true)
  {
    stop_b_connect=false;
    b_connect=false;
    while (stop_b_connect==false||b_stopthred==false)
    {
      QThread::sleep(0);    // 线程休眠，主动放弃时间片
      if(callback_error==1) // 相机卡住了，强制退出ROS
      {
          rclcpp::shutdown();
          stop_b_connect=true;
          break;
      }
    }
    killTimer(timerid1);
  }
}

void SoptopCamera::updata_parameter()
{
  if(b_connect==true)
  {
    QString array="ros2 param set /camera_tis_node exposure_time ";
    QString data=QString::number(i32_exposure);
    array=array+data;
    system(array.toUtf8());
  }
}

void SoptopCamera::int_show_image_inlab()
{
  QImage::Format format = QImage::Format_RGB888;
  switch (cv_image->type())
  {
  case CV_8UC1:
    format = QImage::Format_Indexed8;
    break;
  case CV_8UC3:
    format = QImage::Format_RGB888;
    break;
  case CV_8UC4:
    format = QImage::Format_ARGB32;
    break;
  }
  QImage img = QImage((const uchar*)cv_image->data, cv_image->cols, cv_image->rows,
                      cv_image->cols * cv_image->channels(), format);
  img = img.scaled(m_lab_show->width(),m_lab_show->height(),Qt::IgnoreAspectRatio, Qt::SmoothTransformation);//图片自适应lab大小
  m_lab_show->setPixmap(QPixmap::fromImage(img));
}

StartCameraThread::StartCameraThread(SoptopCamera *statci_p)
{
  _p=statci_p;
}

void StartCameraThread::run()
{
  _p->b_stopthred=false;
  if(_p->b_connect==true)
  {
    if(_p->node_mode==0)
    {
      rclcpp::init(_p->argc,_p->argv);
      rclcpp::spin(std::make_shared<Camshow>(_p));
    }
    else if(_p->node_mode==1)
    {
      rclcpp::init(_p->argc,_p->argv);
      rclcpp::spin(std::make_shared<Cloudshow>(_p));
    }
    // qjq
    else if(_p->node_mode==3){
    rclcpp::init(_p->argc,_p->argv);
    rclcpp::spin(std::make_shared<Cambuild>(_p));
    }
  }
  _p->b_stopthred=true;
}


void SoptopCamera::ros_config_set(std::string msg)
 {
    std_msgs::msg::String::UniquePtr config_msg(new std_msgs::msg::String());

    config_msg->data=msg;

    _pub_config->publish(std::move(config_msg));
 }

void SoptopCamera::ros_set_homography_matrix(Params ros_Params)
{
     _param_homography_matrix->set_parameters({rclcpp::Parameter("homography_matrix", ros_Params.homography_matrix)});
}


Cambuild::Cambuild(SoptopCamera *statci_p):Node("my_cambuild"){

  _p=statci_p;
  _p->b_int_show_image_inlab=false;

 _p->_param_camera = std::make_shared<rclcpp::AsyncParametersClient>(this, "camera_tis_node");
 _p->_param_camera_get = std::make_shared<rclcpp::AsyncParametersClient>(this, "camera_tis_node");
 _p->_param_gpio = std::make_shared<rclcpp::AsyncParametersClient>(this, "gpio_raspberry_node");
 _p->_param_homography_matrix =  std::make_shared<rclcpp::AsyncParametersClient>(this, "line_center_reconstruction_node");
 _p->_param_homography_matrix_get = std::make_shared<rclcpp::AsyncParametersClient>(this, "line_center_reconstruction_node");

 _p->_pub_config=this->create_publisher<std_msgs::msg::String>("config_tis_node/config", 10);

//#ifdef DEBUG_MYINTERFACES
//  subscription_ = this->create_subscription<tutorial_interfaces::msg::IfAlgorhmitmsg>(
//        "/laser_imagepos_node/result", rclcpp::SensorDataQoS(), std::bind(&Camshow::topic_callback, this, _1));
//#else
subscription1_ = this->create_subscription<tutorial_interfaces::msg::IfAlgorhmitimage>(
      "/rotate_image_node/image_rotated", rclcpp::SensorDataQoS(), std::bind(&Cambuild::cambuild_callback, this, _1));
//#endif

 _p->_param_camera_get->wait_for_service();
 _p->_param_camera_get->get_parameters(
                 {"exposure_time"},
                 std::bind(&Cambuild::callbackGlobalParam, this, std::placeholders::_1));

 _p->_param_homography_matrix_get->wait_for_service();
 _p->_param_homography_matrix_get->get_parameters(
                 {"homography_matrix"},
                 std::bind(&Cambuild::callbackMatrixParam, this, std::placeholders::_1));

}

Cambuild::~Cambuild()
{
       _p->_param_camera.reset();
       _p->_param_camera_get.reset();
       _p->_param_gpio.reset();
       _p->_param_homography_matrix.reset();
       _p->_param_homography_matrix_get.reset();
       _p->_pub_config.reset();
}

void Cambuild::cambuild_callback(const tutorial_interfaces::msg::IfAlgorhmitimage msg) const
{ if(_p->b_connect==true)
    {
      if(_p->b_int_show_image_inlab==false)
      {
          cv_bridge::CvImagePtr cv_ptr;
          cv_ptr = cv_bridge::toCvCopy(msg.image, "mono8");
          if(!cv_ptr->image.empty())
          {
              *(_p->cv_image)=cv_ptr->image.clone();
              _p->b_updataimage_finish=true;
              _p->callbacknumber++;
//              if(_p->luzhi==true)
//              {
//                  _p->writer << cv_ptr->image;
//              }
          }
      }
    }
    else
    {
      rclcpp::shutdown();
      _p->stop_b_connect=true;
    }
}

void Cambuild::callbackGlobalParam(std::shared_future<std::vector<rclcpp::Parameter>> future)
{
    auto result = future.get();
    if(result.size()>=1)
    {
      _p->i32_exposure = result.at(0).as_int();
    }
}

void Cambuild::callbackMatrixParam(std::shared_future<std::vector<rclcpp::Parameter>> future)
{
    auto result = future.get();
    if(result.size()>=1)
    {
      _p->ros_Params.homography_matrix = result.at(0).as_double_array();
    }
}
