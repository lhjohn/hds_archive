#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <aws/core/Aws.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/s3/S3Client.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <aws/s3/model/PutObjectRequest.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_openFile_clicked();
  void on_upload_clicked();

private:
  Ui::MainWindow *ui;
  bool PutObjectAsync(const Aws::S3::S3Client& s3Client,
                      const Aws::String& bucketName,
                      const Aws::String& objectName,
                      const Aws::String& region);


};
#endif // MAINWINDOW_H
