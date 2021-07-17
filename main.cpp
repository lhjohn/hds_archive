#include "mainwindow.h"
#include <QApplication>

#include <aws/core/Aws.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/s3/S3Client.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <aws/s3/model/PutObjectRequest.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
