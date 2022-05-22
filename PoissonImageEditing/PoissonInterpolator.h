#pragma once
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include <Eigen/SparseCholesky>


typedef int(*funcptr)(QRgb);

enum colorstatus {

	Red = 0,
	Green = 1,
	Blue = 2,

};


class PoissonInterpolator
{
public:
	PoissonInterpolator() = delete;
	PoissonInterpolator(QImage* Source, QImage* Target);
	~PoissonInterpolator();

public:

	void presolve(QPoint& targetpoint, QPoint& sourcepoint1, QPoint& sourcepoint2);
	void setarr(QPoint& targetpoint, QPoint& sourcepoint1, QPoint& sourcepoint2);
	void changestatus(colorstatus Currentcolor);

	void getsolvevec(std::vector<std::vector<QRgb>>& interpolatedvec, QPoint& targetpoint, 
					 QPoint& sourcepoint1, QPoint& sourcepoint2);

private:

	QImage* source;
	QImage* target;

	int coloridx;
	std::vector<funcptr> funcvec;
	funcptr currentfunc;

	int widsize;
	int heisize;
	int matsize;

	Eigen::VectorXf* arr;
	Eigen::SimplicialLLT<Eigen::SparseMatrix<float>>* presolver;

private:

	int sizetoidxx(int size);
	int sizetoidxy(int size);
	int idxtosize(int x, int y);

	Eigen::VectorXf* redvec;
	Eigen::VectorXf* greenvec;
	Eigen::VectorXf* bluevec;

};

