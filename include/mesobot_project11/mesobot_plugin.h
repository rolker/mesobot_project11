#ifndef MESOBOT_PROJECT11_MESOBOT_PLUGIN_H
#define MESOBOT_PROJECT11_MESOBOT_PLUGIN_H

#include <rqt_gui_cpp/plugin.h>
#include <QWidget>

#include "ui_mesobot_plugin.h"
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <geographic_msgs/GeoPoseStamped.h>
#include <std_msgs/Int32.h>

namespace mesobot {

class MesobotPlugin: public rqt_gui_cpp::Plugin
{
    Q_OBJECT
public:
  MesobotPlugin();

  virtual void initPlugin(qt_gui_cpp::PluginContext& context);
  virtual void shutdownPlugin();
  virtual void saveSettings(qt_gui_cpp::Settings& plugin_settings, qt_gui_cpp::Settings& instance_settings) const;
  virtual void restoreSettings(const qt_gui_cpp::Settings& plugin_settings, const qt_gui_cpp::Settings& instance_settings);


private slots:
  void on_commandTopicLineEdit_editingFinished();
  void on_feedbackTopicLineEdit_editingFinished();
  void on_poseTopicLineEdit_editingFinished();
  void on_namespaceLineEdit_editingFinished();

  void on_sendCommandPushButton_pressed();

  void on_queueDrivePushButton_pressed();
  void on_queueJoyPushButton_pressed();
  void on_queueGoalPushButton_pressed();
  void on_queueWaitPushButton_pressed();
  void on_queueInsertPushButton_pressed();
  void on_queueRemotePushButton_pressed();

  void on_goalSetPointLineEdit_editingFinished();
  void on_goalDegreesSetPointLineEdit_editingFinished();
  void on_driveHeadingLineEdit_editingFinished();
  void on_driveHeadingDegreesLineEdit_editingFinished();

  void updateRemoteCommand(QString command);
  void updateWaitTime();

private:
  void feedbackCallback(const std_msgs::String::ConstPtr & message);
  void sendByOthersCallback(const std_msgs::String::ConstPtr & message);
  void poseCallback(const geographic_msgs::GeoPoseStamped::ConstPtr & message);
  void remoteCommandCallback(const std_msgs::String::ConstPtr & message);
  void waitTimeCallback(const std_msgs::Int32::ConstPtr & message);
  void appendFeedback();
  void updateDepth();

  Ui::MesobotPlugin ui_;
  QWidget* widget_=nullptr;

  double depth_;
  std::mutex depth_lock_;

  int wait_seconds_;
  ros::Time last_wait_time_report_;

  std::string command_topic_;
  std::string feedback_topic_;
  std::string pose_topic_;
  ros::Publisher raw_publisher_;
  ros::Subscriber raw_subscriber_;
  ros::Subscriber send_raw_subscriber_;
  ros::Subscriber pose_subscriber_;
  ros::Subscriber remote_command_subscriber_;
  ros::Subscriber wait_time_subscriber_;

  std::string namespace_;

  std::vector<std::string> feedback_to_add_;
  std::mutex feedback_to_add_lock_;
};

} // namespace mesobot

#endif
