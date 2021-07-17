#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

// #include <aws/core/Aws.h>
// #include <aws/core/utils/logging/LogLevel.h>
// #include <aws/s3/S3Client.h>
// #include <iostream>
// #include <iostream>
// #include <fstream>
// #include <sys/stat.h>
// #include <aws/s3/model/PutObjectRequest.h>

#define UNUSED(x) (void)(x)

// A mutex is a synchronization primitive that can be used to protect shared
// data from being simultaneously accessed by multiple threads.
std::mutex upload_mutex;

// A condition_variable is a synchronization primitive that can be used to
// block a thread, or multiple threads at the same time, until another
// thread both modifies a shared variable (the condition) and
// notifies the condition_variable.
std::condition_variable upload_variable;

using namespace Aws;

void PutObjectAsyncFinished(const Aws::S3::S3Client* s3Client,
    const Aws::S3::Model::PutObjectRequest& request,
    const Aws::S3::Model::PutObjectOutcome& outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext>& context)
{
    if (outcome.IsSuccess()) {
        std::cout << "Success: PutObjectAsyncFinished: Finished uploading '"
            << context->GetUUID() << "'." << std::endl;
    }
    else {
        std::cout << "Error: PutObjectAsyncFinished: " <<
            outcome.GetError().GetMessage() << std::endl;
    }

    UNUSED(s3Client);
    UNUSED(request);
    UNUSED(context);
    // Unblock the thread that is waiting for this function to complete.
    upload_variable.notify_one();
}

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}
QString file_name;

void MainWindow::on_openFile_clicked()
{
    QString filter = "All Files (*.*) ;; Zip Files (*.zip)";
    file_name = QFileDialog::getOpenFileName(this, "Open a file mister", QDir::homePath(), filter);
    ui->lineEdit->setText(file_name);
    QFile file(file_name);
}

bool MainWindow::PutObjectAsync(const Aws::S3::S3Client& s3Client,
    const Aws::String& bucketName,
    const Aws::String& objectName,
    const Aws::String& region)
{
    // Verify that the file exists.
    struct stat buffer;

    if (stat(objectName.c_str(), &buffer) == -1)
    {
        std::cout << "Error: PutObjectAsync: File '" <<
            objectName << "' does not exist." << std::endl;

        return false;
    }

    // Create and configure the asynchronous put object request.
    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucketName);
    request.SetKey(objectName);

    char *key = "SHA256";
    QString value = ui->metaSha256->text();
    // Aws::S
    request.AddMetadata(key, value.toUtf8().constData());

    const std::shared_ptr<Aws::IOStream> input_data =
        Aws::MakeShared<Aws::FStream>("SampleAllocationTag",
            objectName.c_str(),
            std::ios_base::in | std::ios_base::binary);

    request.SetBody(input_data);

    // Create and configure the context for the asynchronous put object request.
    std::shared_ptr<Aws::Client::AsyncCallerContext> context =
        Aws::MakeShared<Aws::Client::AsyncCallerContext>("PutObjectAllocationTag");
    context->SetUUID(objectName);

    // Make the asynchronous put object call. Queue the request into a
    // thread executor and call the PutObjectAsyncFinished function when the
    // operation has finished.
    s3Client.PutObjectAsync(request, PutObjectAsyncFinished, context);
    UNUSED(region);
    return true;
}

void MainWindow::on_upload_clicked()
{
  SDKOptions options;
      options.loggingOptions.logLevel = Utils::Logging::LogLevel::Debug;

      //The AWS SDK for C++ must be initialized by calling Aws::InitAPI.
      InitAPI(options);
      {

        //TODO: Change bucket_name to the name of a bucket in your account.
        const Aws::String bucket_name = "result-store-demo";
        //TODO: Create a file called "my-file.txt" in the local folder where your executables are built to.
        Aws::String object_name = file_name.toUtf8().constData();
        // on windows use: std::string current_locale_text = qs.toLocal8Bit().constData();
        //TODO: Set to the AWS Region in which the bucket was created.
        const Aws::String region = "eu-west-1";

         // S3::S3Client client;
        // Create and configure the Amazon S3 client.
        // This client must be declared here, as this client must exist
        // until the put object operation finishes.
          Aws::Client::ClientConfiguration config;

          if (!region.empty())
          {
              config.region = region;
          }

          Aws::S3::S3Client client(config);

          auto outcome = client.ListBuckets();
          if (outcome.IsSuccess()) {
              std::cout << "Found " << outcome.GetResult().GetBuckets().size() << " buckets\n";
              for (auto&& b : outcome.GetResult().GetBuckets()) {
                  std::cout << b.GetName() << std::endl;
              }
          }
          else {
              std::cout << "Failed with error: " << outcome.GetError() << std::endl;
          }

          // A unique_lock is a general-purpose mutex ownership wrapper allowing
          // deferred locking, time-constrained attempts at locking, recursive
          // locking, transfer of lock ownership, and use with
          // condition variables.
          std::unique_lock<std::mutex> lock(upload_mutex);

          if (PutObjectAsync(client, bucket_name, object_name, region)) {

              std::cout << "main: Waiting for file upload attempt..." <<
                  std::endl << std::endl;

              // While the put object operation attempt is in progress,
              // you can perform other tasks.
              // This example simply blocks until the put object operation
              // attempt finishes.
              upload_variable.wait(lock);

              std::cout << std::endl << "main: File upload attempt completed."
                  << std::endl;
          }
          else
          {
              // return 1;
          }

          // if (!PutObject(bucket_name, object_name, region)) {

          // std::cout << "FAIL" << std::endl;
          // }
      }

      //Before the application terminates, the SDK must be shut down.
      ShutdownAPI(options);
      std::cout << "SUCCS " << std::endl;
}

