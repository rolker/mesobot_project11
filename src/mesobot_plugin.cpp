#include "mesobot_project11/mesobot_plugin.h"
#include <pluginlib/class_list_macros.h>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
namespace mesobot
{

MesobotPlugin::MesobotPlugin():rqt_gui_cpp::Plugin()
{
  setObjectName("MesobotPlugin");
}

void MesobotPlugin::initPlugin(qt_gui_cpp::PluginContext& context)
{
  widget_ = new QWidget();
  ui_.setupUi(widget_);

  ui_.goalDegreesOfFreedomComboBox->addItem("0 forward");
  ui_.goalDegreesOfFreedomComboBox->addItem("1 starboard (right)");
  ui_.goalDegreesOfFreedomComboBox->addItem("2 depth (down)");
  ui_.goalDegreesOfFreedomComboBox->addItem("3 roll");
  ui_.goalDegreesOfFreedomComboBox->addItem("4 pitch");
  ui_.goalDegreesOfFreedomComboBox->addItem("5 heading (yaw)");

  connect(ui_.commandTopicLineEdit, &QLineEdit::editingFinished, this, &MesobotPlugin::on_commandTopicLineEdit_editingFinished);
  connect(ui_.feedbackTopicLineEdit, &QLineEdit::editingFinished, this, &MesobotPlugin::on_feedbackTopicLineEdit_editingFinished);
  connect(ui_.poseTopicLineEdit, &QLineEdit::editingFinished, this, &MesobotPlugin::on_poseTopicLineEdit_editingFinished);
  connect(ui_.namespaceLineEdit, &QLineEdit::editingFinished, this, &MesobotPlugin::on_namespaceLineEdit_editingFinished);
  
  connect(ui_.sendCommandPushButton, &QPushButton::pressed, this, &MesobotPlugin::on_sendCommandPushButton_pressed);
  
  connect(ui_.queueDrivePushButton, &QPushButton::pressed, this, &MesobotPlugin::on_queueDrivePushButton_pressed);
  connect(ui_.queueGoalPushButton, &QPushButton::pressed, this, &MesobotPlugin::on_queueGoalPushButton_pressed);
  connect(ui_.queueJoyPushButton, &QPushButton::pressed, this, &MesobotPlugin::on_queueJoyPushButton_pressed);
  connect(ui_.queueWaitPushButton, &QPushButton::pressed, this, &MesobotPlugin::on_queueWaitPushButton_pressed);
  connect(ui_.queueInsertPushButton, &QPushButton::pressed, this, &MesobotPlugin::on_queueInsertPushButton_pressed);
  connect(ui_.queueRemotePushButton, &QPushButton::pressed, this, &MesobotPlugin::on_queueRemotePushButton_pressed);
  
  connect(ui_.goalSetPointLineEdit, &QLineEdit::editingFinished, this, &MesobotPlugin::on_goalSetPointLineEdit_editingFinished);
  connect(ui_.goalDegreesSetPointLineEdit, &QLineEdit::editingFinished, this, &MesobotPlugin::on_goalDegreesSetPointLineEdit_editingFinished);
  connect(ui_.driveHeadingLineEdit, &QLineEdit::editingFinished, this, &MesobotPlugin::on_driveHeadingLineEdit_editingFinished);
  connect(ui_.driveHeadingDegreesLineEdit, &QLineEdit::editingFinished, this, &MesobotPlugin::on_driveHeadingDegreesLineEdit_editingFinished);
  

  widget_->setWindowTitle(widget_->windowTitle() + " (" + QString::number(context.serialNumber()) + ")");
  
  context.addWidget(widget_);
}

void MesobotPlugin::shutdownPlugin()
{
  raw_subscriber_.shutdown();
  send_raw_subscriber_.shutdown();
  pose_subscriber_.shutdown();
  raw_publisher_.shutdown();
  remote_command_subscriber_.shutdown();
  wait_time_subscriber_.shutdown();
}

void MesobotPlugin::saveSettings(qt_gui_cpp::Settings& plugin_settings, qt_gui_cpp::Settings& instance_settings) const
{
  QString mesobotCommandTopic = ui_.commandTopicLineEdit->text();
  instance_settings.setValue("mesobotCommandTopic", mesobotCommandTopic);
  QString mesobotFeedbackTopic = ui_.feedbackTopicLineEdit->text();
  instance_settings.setValue("mesobotFeedbackTopic", mesobotFeedbackTopic);
  QString mesobotPoseTopic = ui_.poseTopicLineEdit->text();
  instance_settings.setValue("mesobotPoseTopic", mesobotPoseTopic);
  QString mesobotNamespace = ui_.namespaceLineEdit->text();
  instance_settings.setValue("mesobotNamespace", mesobotNamespace);

  QString smsPreamble = ui_.smsPreambleLineEdit->text();
  instance_settings.setValue("mesobotSMSPreamble", smsPreamble);
  QString smsPostamble = ui_.smsPostambleLineEdit->text();
  instance_settings.setValue("mesobotSMSPostamble", smsPostamble);

}

void MesobotPlugin::restoreSettings(const qt_gui_cpp::Settings& plugin_settings, const qt_gui_cpp::Settings& instance_settings)
{
  QString mesobotCommandTopic = instance_settings.value("mesobotCommandTopic", "").toString();
  ui_.commandTopicLineEdit->setText(mesobotCommandTopic);
  QString mesobotFeedbackTopic = instance_settings.value("mesobotFeedbackTopic", "").toString();
  ui_.feedbackTopicLineEdit->setText(mesobotFeedbackTopic);
  QString mesobotPoseTopic = instance_settings.value("mesobotPoseTopic", "").toString();
  ui_.poseTopicLineEdit->setText(mesobotPoseTopic);
  QString mesobotNamespace = instance_settings.value("mesobotNamespace", "").toString();
  ui_.namespaceLineEdit->setText(mesobotNamespace);

  QString smsPreamble = instance_settings.value("mesobotSMSPreamble", "").toString();
  ui_.smsPreambleLineEdit->setText(smsPreamble);
  QString smsPostamble = instance_settings.value("mesobotSMSPostamble", "").toString();
  ui_.smsPostambleLineEdit->setText(smsPostamble);

  on_commandTopicLineEdit_editingFinished();
  on_feedbackTopicLineEdit_editingFinished();
  on_poseTopicLineEdit_editingFinished();
  on_namespaceLineEdit_editingFinished();
}

void MesobotPlugin::on_commandTopicLineEdit_editingFinished()
{
  auto topic = ui_.commandTopicLineEdit->text().toStdString();
  if(topic != command_topic_)
  {
    raw_publisher_.shutdown();
    send_raw_subscriber_.shutdown();
    if(!topic.empty())
    {
      raw_publisher_ = getNodeHandle().advertise<std_msgs::String>(topic, 10);
      send_raw_subscriber_ = getNodeHandle().subscribe(topic, 10, &MesobotPlugin::sendByOthersCallback, this);
    }
  }
  command_topic_ = topic;
}

void MesobotPlugin::on_feedbackTopicLineEdit_editingFinished()
{
  auto topic = ui_.feedbackTopicLineEdit->text().toStdString();
  if(topic != feedback_topic_)
  {
    raw_subscriber_.shutdown();
    if(!topic.empty())
      raw_subscriber_ = getNodeHandle().subscribe(topic, 10, &MesobotPlugin::feedbackCallback, this);
    feedback_topic_ = topic;
  }  
}

void MesobotPlugin::on_poseTopicLineEdit_editingFinished()
{
  auto topic = ui_.poseTopicLineEdit->text().toStdString();
  if(topic != pose_topic_)
  {
    pose_subscriber_.shutdown();
    if(!topic.empty())
      pose_subscriber_ = getNodeHandle().subscribe(topic, 10, &MesobotPlugin::poseCallback, this);
    pose_topic_ = topic;
  }  
}

void MesobotPlugin::on_namespaceLineEdit_editingFinished()
{
  auto ns = ui_.namespaceLineEdit->text().toStdString();
  if(ns != namespace_)
  {
    std::vector<std::string> files;
    if(ros::param::get(ns+"/control/files", files))
    {
      ui_.insertFileComboBox->clear();
      for(auto f: files)
        ui_.insertFileComboBox->addItem(f.c_str());
    }

    auto remote_topic = ns;
    if(!remote_topic.empty() && remote_topic[0] != '/')
      remote_topic = "/"+remote_topic;
    remote_topic += "/remote_command";
    remote_command_subscriber_.shutdown();
    remote_command_subscriber_ = getNodeHandle().subscribe(remote_topic, 1, &MesobotPlugin::remoteCommandCallback, this);

    auto wait_time_topic = ns;
    if(!wait_time_topic.empty() && wait_time_topic[0] != '/')
      wait_time_topic = "/"+wait_time_topic;
    wait_time_topic += "/wait_seconds";
    wait_time_subscriber_.shutdown();
    wait_time_subscriber_ = getNodeHandle().subscribe(wait_time_topic, 1, &MesobotPlugin::waitTimeCallback, this);


    namespace_ = ns;
  }
}

void MesobotPlugin::feedbackCallback(const std_msgs::String::ConstPtr& message)
{
  std::lock_guard<std::mutex> lock(feedback_to_add_lock_);
  std::string html = "<span style=\"color:blue;\">" + QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss").toStdString();
  html += " </span>" + message->data;
  feedback_to_add_.push_back(html);
  QMetaObject::invokeMethod(this, &MesobotPlugin::appendFeedback);
}

void MesobotPlugin::sendByOthersCallback(const std_msgs::String::ConstPtr& message)
{
  std::lock_guard<std::mutex> lock(feedback_to_add_lock_);
  std::string html = "<span style=\"color:maroon;\">" + QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss").toStdString();
  auto data = message->data;
  auto less_than = data.find('<');
  while (less_than != std::string::npos)
  {
    data = data.substr(0,less_than)+data.substr(less_than+1);
    less_than = data.find('<');
  }
  html += " </span>" + data;
  feedback_to_add_.push_back(html);
  QMetaObject::invokeMethod(this, &MesobotPlugin::appendFeedback);
}


void MesobotPlugin::appendFeedback()
{
  std::lock_guard<std::mutex> lock(feedback_to_add_lock_);
  for(auto feedback: feedback_to_add_)
    ui_.feedbackPlainTextEdit->appendHtml(feedback.c_str());
  feedback_to_add_.clear();
}

void MesobotPlugin::on_sendCommandPushButton_pressed()
{
  std_msgs::String message;
  message.data = ui_.commandLineEdit->text().toStdString();
  raw_publisher_.publish(message);
  std::string html = "<span style=\"color:red;\">" + QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss").toStdString();
  html += " </span>" + message.data;
  {
    std::lock_guard<std::mutex> lock(feedback_to_add_lock_);
    feedback_to_add_.push_back(html);
  }
  QMetaObject::invokeMethod(this, &MesobotPlugin::appendFeedback);
}

void MesobotPlugin::on_queueDrivePushButton_pressed()
{
  QString command = ui_.smsPreambleLineEdit->text();
  command += "DRIVE ";
  command += ui_.driveTimeLineEdit->text();
  command += " ";
  command += ui_.driveDutyCycleLineEdit->text();
  if (!ui_.driveHeadingLineEdit->text().isEmpty())
  {
    command += " ";
    command += ui_.driveHeadingLineEdit->text();
    if(!ui_.driveDepthLineEdit->text().isEmpty())
    {
      command += " ";
      command += ui_.driveDepthLineEdit->text();
    }
  }
  command += ui_.smsPostambleLineEdit->text();
  ui_.commandLineEdit->setText(command);
}

void MesobotPlugin::on_queueGoalPushButton_pressed()
{
  QString command = ui_.smsPreambleLineEdit->text();
  command += "GOAL1 ";
  QString dof = ui_.goalDegreesOfFreedomComboBox->currentText();
  if(dof.size() > 1)
    dof = dof.toStdString().substr(0,1).c_str();
  command += dof;
  command += " ";
  command += ui_.goalSetPointLineEdit->text();
  command += ui_.smsPostambleLineEdit->text();
  ui_.commandLineEdit->setText(command);

}

void MesobotPlugin::on_queueJoyPushButton_pressed()
{
  QString command = ui_.smsPreambleLineEdit->text();
  command += "JOY1 ";
  QString dof = ui_.goalDegreesOfFreedomComboBox->currentText();
  if(dof.size() > 1)
    dof = dof.toStdString().substr(0,1).c_str();
  command += dof;
  command += " ";
  command += ui_.goalSetPointLineEdit->text();
  command += ui_.smsPostambleLineEdit->text();
  ui_.commandLineEdit->setText(command);
}

void MesobotPlugin::on_queueWaitPushButton_pressed()
{
  QString command = ui_.smsPreambleLineEdit->text();
  QString postamble = ui_.smsPostambleLineEdit->text();
  while(!postamble.isEmpty() && (postamble[0] == ';' || postamble[0] == ' ') )
    postamble = postamble.toStdString().substr(1).c_str();
  command += postamble;
  ui_.commandLineEdit->setText(command);
}

void MesobotPlugin::on_queueInsertPushButton_pressed()
{
  QString command = ui_.smsPreambleLineEdit->text();
  command += "INSERT ";
  command += ui_.insertFileComboBox->currentText();
  command += ui_.smsPostambleLineEdit->text();
  ui_.commandLineEdit->setText(command);
}


void MesobotPlugin::on_queueRemotePushButton_pressed()
{
  QString command = ui_.smsPreambleLineEdit->text();
  command += ui_.remoteCommandResultLineEdit->text();
  command += ui_.smsPostambleLineEdit->text();
  ui_.commandLineEdit->setText(command);
}

void MesobotPlugin::on_goalSetPointLineEdit_editingFinished()
{
  bool ok;
  double radians = ui_.goalSetPointLineEdit->text().toDouble(&ok);
  if(ok)
  {
    auto degrees = 180.0*radians/M_PI;
    ui_.goalDegreesSetPointLineEdit->setText(QString::number(degrees));
  }
}

void MesobotPlugin::on_goalDegreesSetPointLineEdit_editingFinished()
{
  bool ok;
  double degrees = ui_.goalDegreesSetPointLineEdit->text().toDouble(&ok);
  if(ok)
  {
    auto radians = M_PI*degrees/180.0;
    ui_.goalSetPointLineEdit->setText(QString::number(radians));
  }

}

void MesobotPlugin::on_driveHeadingLineEdit_editingFinished()
{
  bool ok;
  double radians = ui_.driveHeadingLineEdit->text().toDouble(&ok);
  if(ok)
  {
    auto degrees = 180.0*radians/M_PI;
    ui_.driveHeadingDegreesLineEdit->setText(QString::number(degrees));
  }
  else
    ui_.driveHeadingDegreesLineEdit->clear();
}

void MesobotPlugin::on_driveHeadingDegreesLineEdit_editingFinished()
{
  bool ok;
  double degrees = ui_.driveHeadingDegreesLineEdit->text().toDouble(&ok);
  if(ok)
  {
    auto radians = M_PI*degrees/180.0;
    ui_.driveHeadingLineEdit->setText(QString::number(radians));
  }
  else
    ui_.driveHeadingLineEdit->clear();

}


void MesobotPlugin::poseCallback(const geographic_msgs::GeoPoseStamped::ConstPtr & message)
{
  {
    std::lock_guard<std::mutex> lock(depth_lock_);
    depth_ = -message->pose.position.altitude;
  }
  QMetaObject::invokeMethod(this, &MesobotPlugin::updateDepth);
}

void MesobotPlugin::updateDepth()
{
  std::lock_guard<std::mutex> lock(depth_lock_);
  ui_.depthLcdNumber->display(int(depth_));
}


void MesobotPlugin::remoteCommandCallback(const std_msgs::String::ConstPtr & message)
{
  QString command = message->data.c_str();
  QMetaObject::invokeMethod(this, "updateRemoteCommand", Qt::QueuedConnection, Q_ARG(QString, command));
}

void MesobotPlugin::updateRemoteCommand(QString command)
{
  ui_.remoteCommandLineEdit->setText(command);
  QString resulting_command = "Invalid command";
  auto parts = command.split(QRegularExpression("\\s+"));
  if(!parts.empty())
  {
    if(parts.front() == "mesobot")
    {
      if(parts.size() == 3)
      {
        if(parts[1] == "depth")
        {
          bool ok;
          float value = parts[2].toFloat(&ok);
          if(ok)
          {
            resulting_command = "GOAL1 2 "+parts[2];
          }
        }
        if(parts[1] == "sample")
        {
          bool ok;
          int sampler = parts[2].toInt(&ok);
          if(ok)
          {
            auto sampler_string = QString::number(sampler);
            if(sampler_string.size() == 1)
              sampler_string = "0"+sampler_string;
            QString mxfile = "bytes-sample-"+sampler_string+"-20m.mx";
            if(ui_.insertFileComboBox->findText(mxfile) == -1)
              resulting_command = "Invlid sampler";
            else              
              resulting_command = "INSERT "+mxfile;
          }
          else
            resulting_command = "Invlid sampler";
        }
        if(parts[1] == "lights")
        {
          if(parts[2] == "on")
            resulting_command = "INSERT bytes-lights-white.mx";
          if(parts[2] == "off")
            resulting_command = "INSERT bytes-lights-off.mx";
        }
      }
    }
  }

  ui_.remoteCommandResultLineEdit->setText(resulting_command);
}


void MesobotPlugin::waitTimeCallback(const std_msgs::Int32::ConstPtr & message)
{
  last_wait_time_report_ = ros::Time::now();
  wait_seconds_ = message->data;
  QMetaObject::invokeMethod(this, &MesobotPlugin::updateWaitTime, Qt::QueuedConnection);
}

void MesobotPlugin::updateWaitTime()
{
  int wait = wait_seconds_ - (ros::Time::now()-last_wait_time_report_).toSec();
  ui_.waitTimeLcdNumber->display(wait);
  QTimer::singleShot(1000, this,  &MesobotPlugin::updateWaitTime);
}

} // namespace mesobot

PLUGINLIB_EXPORT_CLASS(mesobot::MesobotPlugin, rqt_gui_cpp::Plugin)
