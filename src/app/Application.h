#pragma once
#include <QCoreApplication>
class Application : public QCoreApplication { public: Application(int &argc, char **argv); int run(); };
