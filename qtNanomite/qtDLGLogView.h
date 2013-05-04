#ifndef QTDLGLOGVIEW_H
#define QTDLGLOGVIEW_H

#include <QDockWidget>
#include "ui_qtDLGLogView.h"

class qtDLGLogView : public QDockWidget, public Ui_qtDLGLogView
{
	Q_OBJECT

public:
	qtDLGLogView(QWidget *parent = 0);
	~qtDLGLogView();

private:
};

#endif // QTDLGLOGVIEW_H
