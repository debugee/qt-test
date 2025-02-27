#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QIcon icon(":/test");
    ui->pushButton->setIcon(icon);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QIcon icon(":/res/test1.png");
    ui->pushButton->setIcon(icon);
    QMessageBox::information(this, tr("title"), tr("text"));
}