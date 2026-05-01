/**
 * @file place_dialog.h
 * @brief PlaceDialog - dialog for editing place properties (name, tokens, action).
 * @author xfacka00 (xfacka00@stud.fit.vutbr.cz)
 * @author xlogin02 (xlogin02@stud.fit.vutbr.cz)
 * @date 2026-04
 */

#pragma once

#include <QDialog>

class PlaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PlaceDialog(QWidget *parent = nullptr);
};
