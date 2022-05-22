#include "PoissonInterpolator.h"


PoissonInterpolator::PoissonInterpolator(QImage* Source, QImage* Target) {
	source = Source;
	target = Target;
	coloridx = -1;
	currentfunc = nullptr;
	funcvec.clear();
	funcvec.push_back(&qRed);
	funcvec.push_back(&qGreen);
	funcvec.push_back(&qBlue);
	widsize = 0;
	heisize = 0;
	matsize = 0;
	arr = nullptr;
	presolver = nullptr;
	redvec = nullptr;
	greenvec = nullptr;
	bluevec = nullptr;
}

PoissonInterpolator::~PoissonInterpolator(){
	funcvec.clear();
	source = nullptr;
	target = nullptr;
	currentfunc = nullptr;
	delete arr;
	arr = nullptr;
	delete presolver;
	presolver = nullptr;

	delete redvec;
	redvec = nullptr;
	delete greenvec;
	greenvec = nullptr;
	delete bluevec;
	bluevec = nullptr;
}

void PoissonInterpolator::changestatus(colorstatus currentcolor) {
	coloridx = (int)currentcolor;
	currentfunc = funcvec[coloridx];
	return;
}

void PoissonInterpolator::presolve(QPoint& targetpoint, QPoint& sourcepoint1, QPoint& sourcepoint2) {
	int x0 = targetpoint.x();
	int y0 = targetpoint.y();
	int x1 = sourcepoint1.x();
	int y1 = sourcepoint1.y();
	int x2 = sourcepoint2.x();
	int y2 = sourcepoint2.y();
	if (x0 + x2 - x1 >= target->width()) {
		printf("width out of bound!\n");
		exit(0);
	}
	if (y0 + y2 - y1 >= target->height()) {
		printf("height out of bound!\n");
		exit(0);
	}

	heisize = y2 - y1 - 1;
	widsize = x2 - x1 - 1;
	if (widsize == 0 || heisize == 0) {
		printf("presolve data is incorrect!\n");
		exit(0);
	}

	matsize = widsize * heisize;
	Eigen::SparseMatrix<float> sparsemat(matsize, matsize);
	std::vector<Eigen::Triplet<float>> tripvec(0);

	for (auto i = 0; i < widsize; i++) {
		for (auto j = 0; j < heisize; j++) {

			int idx = idxtosize(i, j);
			tripvec.emplace_back(idx, idx, 4);
			if (i == 0) {
				int idx2 = idxtosize(i + 1, j);
				tripvec.emplace_back(idx, idx2, -1);
			}
			else if (i == widsize - 1) {
				int idx0 = idxtosize(i - 1, j);
				tripvec.emplace_back(idx, idx0, -1);
			}
			else {
				int idx2 = idxtosize(i + 1, j);
				int idx0 = idxtosize(i - 1, j);
				tripvec.emplace_back(idx, idx2, -1);
				tripvec.emplace_back(idx, idx0, -1);
			}

			if (j == 0) {
				int idx3 = idxtosize(i, j + 1);
				tripvec.emplace_back(idx, idx3, -1);
			}
			else if (j == heisize - 1) {
				int idx1 = idxtosize(i, j - 1);
				tripvec.emplace_back(idx, idx1, -1);
			}
			else {
				int idx3 = idxtosize(i, j + 1);
				int idx1 = idxtosize(i, j - 1);
				tripvec.emplace_back(idx, idx1, -1);
				tripvec.emplace_back(idx, idx3, -1);
			}

		}
	}

	sparsemat.setFromTriplets(tripvec.begin(), tripvec.end());
	sparsemat.makeCompressed();

	delete presolver;
	presolver = new Eigen::SimplicialLLT<Eigen::SparseMatrix<float>>();
	presolver->compute(sparsemat);

	return;

}

void PoissonInterpolator::setarr(QPoint& targetpoint, QPoint& sourcepoint1, QPoint& sourcepoint2) {

	if (matsize == 0) {
		printf("matsize undefined!\n");
		exit(0);
	}

	delete arr;
	arr = new Eigen::VectorXf(matsize);

	std::vector<std::vector<float>> realx(widsize + 1, std::vector<float>(heisize + 1));
	std::vector<std::vector<float>> realy(widsize + 1, std::vector<float>(heisize + 1));
	for (auto i = 0; i <= widsize; i++) {
		for (auto j = 0; j <= heisize; j++) {
			int i0 = sourcepoint1.x() + i;
			int j0 = sourcepoint1.y() + j;
			int i1 = targetpoint.x() + i;
			int j1 = targetpoint.y() + j;
			float x1 = (float)(currentfunc(source->pixel(i0 + 1, j0)) - currentfunc(source->pixel(i0, j0)));
			float y1 = (float)(currentfunc(source->pixel(i0, j0 + 1)) - currentfunc(source->pixel(i0, j0)));
			float x2 = (float)(currentfunc(target->pixel(i1 + 1, j1)) - currentfunc(target->pixel(i1, j1)));
			float y2 = (float)(currentfunc(target->pixel(i1, j1 + 1)) - currentfunc(target->pixel(i1, j1)));

			float norm1 = x1 * x1 + y1 * y1;
			float norm2 = x2 * x2 + y2 * y2;
			if (norm1 >= norm2) {
				realx[i][j] = x1;
				realy[i][j] = y1;
			}
			else {
				realx[i][j] = x2;
				realy[i][j] = y2;
			}
		}
	}

	for (auto i = 0; i < widsize; i++) {
		for (auto j = 0; j < heisize; j++) {

			int i1 = targetpoint.x() + 1 + i;
			int j1 = targetpoint.y() + 1 + j;
			int idx = idxtosize(i, j);
			float val = 0;
			val = -(realx[i + 1][j + 1] - realx[i][j + 1] + realy[i + 1][j + 1] - realy[i + 1][j]);

			if (i == 0) {
				val += (float)currentfunc(target->pixel(i1 - 1, j1));
			}
			else if (i == widsize - 1) {
				val += (float)currentfunc(target->pixel(i1 + 1, j1));
			}

			if (j == 0) {
				val += (float)currentfunc(target->pixel(i1, j1 - 1));
			}
			else if (j == heisize - 1) {
				val += (float)currentfunc(target->pixel(i1, j1 + 1));
			}

			(*arr)(idx) = val;

		}
	}

	return;

}

int PoissonInterpolator::sizetoidxy(int size) {
	if (matsize == 0) {
		printf("0 error!\n");
		exit(0);
	}

	return size % heisize;
}

int PoissonInterpolator::sizetoidxx(int size) {
	if (matsize == 0) {
		printf("0 error!\n");
		exit(0);
	}
	int x = (size - sizetoidxy(size)) / heisize;
	if (x < 0 || x >= widsize) {
		printf("size out of range!\n");
		exit(0);
	}
	return x;
}

int PoissonInterpolator::idxtosize(int x, int y) {
	if (matsize == 0) {
		printf("0 error!\n");
		exit(0);
	}
	if (x < 0 || x >= widsize) {
		printf("x out of range!\n");
		exit(0);
	}
	if (y < 0 || y >= heisize) {
		printf("y out of range!\n");
		exit(0);
	}

	return x * heisize + y;

}

void PoissonInterpolator::getsolvevec(std::vector<std::vector<QRgb>>& interpolatedvec, QPoint& targetpoint, 
									  QPoint& sourcepoint1, QPoint& sourcepoint2) {
	
	if (interpolatedvec.size() != widsize || interpolatedvec[0].size() != heisize) {
		printf("vec size errors!\n");
		exit(0);
	}
	
	delete redvec;
	redvec = new Eigen::VectorXf();
	delete greenvec;
	greenvec = new Eigen::VectorXf();
	delete bluevec;
	bluevec = new Eigen::VectorXf();

	changestatus(Red);
	setarr(targetpoint, sourcepoint1, sourcepoint2);
	(*redvec) = presolver->solve((*arr));
	changestatus(Green);
	setarr(targetpoint, sourcepoint1, sourcepoint2);
	(*greenvec) = presolver->solve((*arr));
	changestatus(Blue);
	setarr(targetpoint, sourcepoint1, sourcepoint2);
	(*bluevec) = presolver->solve((*arr));
	auto abv = [&](float a) { 
		float ans = a;
		if(ans<0){
			ans = 0;
		}
		if (ans > 255) {
			ans = 255;
		}
		return ans; 
	};

	for (auto i = 0; i < widsize; i++) {
		for (auto j = 0; j < heisize; j++) {
			int idx = idxtosize(i, j);
			QRgb color = qRgb(abv((*redvec)(idx)),abv((*greenvec)(idx)), abv((*bluevec)(idx)));
			interpolatedvec[i][j] = color;
		}
	}

	return;
}