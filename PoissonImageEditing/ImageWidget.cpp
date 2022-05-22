#include "ImageWidget.h"
#include "ChildWindow.h"

using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow* relatewindow)
{
	image_ = new QImage();
	image_backup_ = new QImage();

	draw_status_ = kNone;
	is_choosing_ = false;
	is_pasting_ = false;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);

	source_window_ = NULL;
	interpolator = NULL;
}

ImageWidget::~ImageWidget(void)
{
	delete interpolator;
	interpolator = NULL;
}

int ImageWidget::ImageWidth()
{
	return image_->width();
}

int ImageWidget::ImageHeight()
{
	return image_->height();
}

void ImageWidget::set_draw_status_to_choose()
{
	draw_status_ = kChoose;
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
	delete interpolator;
	interpolator = new PoissonInterpolator(source_window_->imagewidget_->image(), image_);
}

QImage* ImageWidget::image()
{
	return image_;
}

void ImageWidget::set_source_window(ChildWindow* childwindow)
{
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect(0, 0, image_->width(), image_->height());
	painter.drawImage(rect, *image_);

	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);
	painter.drawRect(point_start_.x(), point_start_.y(),
		point_end_.x() - point_start_.x(), point_end_.y() - point_start_.y());

	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent* mouseevent)
{
	if (Qt::LeftButton == mouseevent->button())
	{
		switch (draw_status_)
		{
		case kChoose:
		{

			int xpos = mouseevent->pos().x();
			int ypos = mouseevent->pos().y();
			if (xpos >= 0 && xpos < image_->width() && ypos >= 0 && ypos < image_->height()) {
				is_choosing_ = true;
				point_start_ = point_end_ = mouseevent->pos();
			}

			break;
		}
		case kPaste:
		{
			is_pasting_ = true;

			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			int xsourcepos = source_window_->imagewidget_->point_start_.rx();
			int ysourcepos = source_window_->imagewidget_->point_start_.ry();

			int w = source_window_->imagewidget_->point_end_.rx()
				    - source_window_->imagewidget_->point_start_.rx() - 1;
			int h = source_window_->imagewidget_->point_end_.ry()
				    - source_window_->imagewidget_->point_start_.ry() - 1;

			if ((xpos + w + 1 < image_->width()) && (ypos + h + 1 < image_->height()))
			{
				
				*(image_) = *(image_backup_);
				QPoint targetpoint = mouseevent->pos();
				interpolator->presolve(targetpoint, source_window_->imagewidget_->point_start_,
									   source_window_->imagewidget_->point_end_);

				std::vector<std::vector<QRgb>> interpolatedvec(w, std::vector<QRgb>(h));
				execinterpolation(targetpoint, interpolatedvec);

				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						image_->setPixel(xpos + i + 1, ypos + j + 1, interpolatedvec[i][j]);
					}
				}
			}
		}
		default:
			break;
		}
		update();

		
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			int xpos = mouseevent->pos().x();
			int ypos = mouseevent->pos().y();

			if (xpos >= 0 && xpos < image_->width() && ypos >= 0 && ypos < image_->height()) {
				point_end_ = mouseevent->pos();

			}
			
		}
		break;

	case kPaste:
		if (is_pasting_)
		{
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			int xsourcepos = source_window_->imagewidget_->point_start_.rx();
			int ysourcepos = source_window_->imagewidget_->point_start_.ry();

			int w = source_window_->imagewidget_->point_end_.rx()
				- source_window_->imagewidget_->point_start_.rx() - 1;
			int h = source_window_->imagewidget_->point_end_.ry()
				- source_window_->imagewidget_->point_start_.ry() - 1;

			if ((xpos > 0) && (ypos > 0) && (xpos + w + 1 < image_->width()) && (ypos + h + 1 < image_->height()))
			{
				*(image_) = *(image_backup_);
				QPoint targetpoint = mouseevent->pos();
				interpolator->presolve(targetpoint, source_window_->imagewidget_->point_start_,
									   source_window_->imagewidget_->point_end_);

				std::vector<std::vector<QRgb>> interpolatedvec(w, std::vector<QRgb>(h));
				execinterpolation(targetpoint, interpolatedvec);

				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						image_->setPixel(xpos + i + 1, ypos + j + 1, interpolatedvec[i][j]);
					}
				}
			}
		}

	default:
		break;
	}

	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			int xpos = mouseevent->pos().x();
			int ypos = mouseevent->pos().y();

			if (xpos >= 0 && xpos < image_->width() && ypos >= 0 && ypos < image_->height()) {
				point_end_ = mouseevent->pos();
				if (xpos < point_start_.x()) {
					int tmp = point_start_.x();
					point_start_.rx() = xpos;
					point_end_.rx() = tmp;
				}
				if (ypos < point_start_.y()) {
					int tmp = point_start_.y();
					point_start_.ry() = ypos;
					point_end_.ry() = tmp;
				}

			}

			is_choosing_ = false;
			draw_status_ = kNone;
		}

	case kPaste:
		if (is_pasting_)
		{
			is_pasting_ = false;
			draw_status_ = kNone;
		}

	default:
		break;
	}

	update();
}

void ImageWidget::Open(QString filename)
{
	// Load file
	if (!filename.isEmpty())
	{
		image_->load(filename);
		*(image_backup_) = *(image_);
	}

	cout << "image size: " << image_->width() << ' ' << image_->height() << endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	image_->save(filename);
}

void ImageWidget::Restore()
{
	*(image_) = *(image_backup_);
	point_start_ = point_end_ = QPoint(0, 0);
	delete interpolator;
	interpolator = NULL;
	update();
}


void ImageWidget::execinterpolation(QPoint& targetpoint, std::vector<std::vector<QRgb>>& interpolatedvec) {

	interpolator->getsolvevec(interpolatedvec, targetpoint, 
							  source_window_->imagewidget_->point_start_,
							  source_window_->imagewidget_->point_end_);

	return;

}